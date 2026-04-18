#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define MAX_FLIGHTS   30
#define NUM_RUNWAYS    3
#define WAIT_THRESHOLD 5      /* seconds before priority boost */
#define TOTAL_FLIGHTS 15

/* ── Flight types & base priorities ── */
#define EMERGENCY     100
#define FUEL_LOW       40
#define LANDING        20
#define TAKEOFF        10
#define CARGO_TAKEOFF   5

typedef struct {
    int  id,  priority, type;
    long arrival_time;
    char label[32];
} Flight;

static Flight heap[MAX_FLIGHTS];
static int    heap_size = 0;

static void heap_swap(int a, int b) {
    Flight tmp = heap[a]; heap[a] = heap[b]; heap[b] = tmp;
}

static void sift_up(int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heap[p].priority >= heap[i].priority) break;
        heap_swap(p, i);
        i = p;
    }
}

static void sift_down(int i) {
    while (1) {
        int best = i, l = 2*i+1, r = 2*i+2;
        if (l < heap_size && heap[l].priority > heap[best].priority) best = l;
        if (r < heap_size && heap[r].priority > heap[best].priority) best = r;
        if (best == i) break;
        heap_swap(i, best);
        i = best;
    }
}

/* Must hold global lock when calling these */
static void pq_push(Flight f) {
    if (heap_size >= MAX_FLIGHTS) {
        fprintf(stderr, "[WARN ] Queue full – flight %d dropped!\n", f.id);
        return;
    }
    heap[heap_size++] = f;
    sift_up(heap_size - 1);
}

/* Boost priority of long-waiting flights, then pop best */
static int pq_pop(Flight *out) {
    if (heap_size == 0) return 0;

    long now = time(NULL);
    /* Dynamic priority adjustment per requirement 5 */
    int heap_changed = 0;
    for (int i = 0; i < heap_size; i++) {
        long waited = now - heap[i].arrival_time;
        if (waited > WAIT_THRESHOLD) {
            int boost = (int)(waited - WAIT_THRESHOLD);
            int new_p = heap[i].priority + boost;
            if (new_p >= EMERGENCY) new_p = EMERGENCY - 1;
            if (new_p != heap[i].priority) {
                heap[i].priority = new_p;
                heap_changed = 1;
            }
        }
    }
    /* Rebuild heap once after all updates to avoid mid-iteration corruption */
    if (heap_changed) {
        for (int i = heap_size / 2 - 1; i >= 0; i--)
            sift_down(i);
    }

    /* Rule 3: if any LANDING flight waited > threshold, prefer it over TAKEOFF */
    int land_idx = -1;
    for (int i = 0; i < heap_size; i++) {
        if ((heap[i].type == LANDING || heap[i].type == FUEL_LOW) &&
            (now - heap[i].arrival_time > WAIT_THRESHOLD)) {
            if (land_idx == -1 ||  heap[i].priority > heap[land_idx].priority)
                land_idx = i;
        }
    }

    /* Rule 3: if any landing flight waited > threshold, it beats any takeoff, regardless of what the heap root happens to be */
    int pick = 0;
    if (land_idx != -1) {
        int top_is_takeoff = (heap[0].type == TAKEOFF ||  heap[0].type == CARGO_TAKEOFF);       /* Only override if the current best is a takeoff type */
        if (top_is_takeoff || heap[land_idx].priority > heap[0].priority) pick = land_idx;      /* Or even if top is a landing but land_idx has higher priority */
    }

    *out = heap[pick];
    heap[pick] = heap[--heap_size];
    sift_down(pick);
    if (pick > 0) sift_up(pick);
    return 1;
}

/* Flight Waiting Time Map Maps flight id → arrival_time for O(1) lookup. */
#define MAX_ID (TOTAL_FLIGHTS + 1)
static long waiting_map[MAX_ID];   /* waiting_map[flight.id] = arrival_time */

static void wtmap_add(int id, long arrival) {
    if (id > 0 && id < MAX_ID) waiting_map[id] = arrival;
}

static long wtmap_get(int id) {
    if (id > 0 && id < MAX_ID) return waiting_map[id];
    return 0;
}

static void wtmap_remove(int id) {
    if (id > 0 && id < MAX_ID) waiting_map[id] = 0;
}

/* Runway Status Table  (one mutex per runway)  */
static int runway_busy[NUM_RUNWAYS]  = {0};
static int runway_maint[NUM_RUNWAYS] = {0};
static pthread_mutex_t runway_lock[NUM_RUNWAYS]; /* per-runway locking */

/* Global queue lock + condition */
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  queue_cond = PTHREAD_COND_INITIALIZER;

static int flights_done = 0;

/* Emergency preemption :When an emergency is detected, the monitor sets this flag. Runway controllers check it cooperatively and yield their runway. */
static int emergency_pending = 0;   /* 1 when emergency waiting */

static const char *type_name(int t) {
    switch (t) {
        case EMERGENCY:    return "EMERGENCY";
        case FUEL_LOW:     return "FUEL-LOW";
        case LANDING:      return "LANDING";
        case TAKEOFF:      return "TAKEOFF";
        case CARGO_TAKEOFF:return "CARGO";
        default:           return "UNKNOWN";
    }
}

/* Flight Generator Thread  */
static void *flight_generator(void *arg) {
    (void)arg;
    int types[]   = { LANDING, TAKEOFF, CARGO_TAKEOFF, FUEL_LOW, EMERGENCY };
    int weights[] = { 40,      30,      15,            10,        5 };

    for (int i = 0; i < TOTAL_FLIGHTS; i++) {
        int r = rand() % 100, cum = 0, t = 0;
        for (int k = 0; k < 5; k++) {
            cum += weights[k];
            if (r < cum) { t = k; break; }
        }
        Flight f;
        f.id           = i + 1;
        f.type         = types[t];
        f.priority     = types[t];
        f.arrival_time = time(NULL);
        snprintf(f.label, sizeof(f.label), "%s #%d", type_name(f.type), f.id);

        pthread_mutex_lock(&queue_lock);
        pq_push(f);
        wtmap_add(f.id, f.arrival_time);
        printf("[GEN   ] %-20s priority=%-3d\n", f.label, f.priority);
        pthread_cond_broadcast(&queue_cond);
        pthread_mutex_unlock(&queue_lock);

        usleep(400000); /* ~0.4 s between arrivals */
    }
    return NULL;
}

/* Runway Controller Thread */
static void *runway_controller(void *arg) {
    int rw = *(int *)arg;
    while (1) {
        pthread_mutex_lock(&queue_lock);

        while (1) {
            if (flights_done >= TOTAL_FLIGHTS && heap_size == 0) {
                pthread_mutex_unlock(&queue_lock);
                printf("[RWY %-2d] Controller exiting.\n", rw + 1);
                return NULL;
            }

            pthread_mutex_lock(&runway_lock[rw]);
            int maint = runway_maint[rw];
            int busy  = runway_busy[rw];
            pthread_mutex_unlock(&runway_lock[rw]);

            if (heap_size > 0 && !maint && !busy)  break;

            pthread_cond_wait(&queue_cond, &queue_lock);
        }

        /* ── Pick the best flight ── */
        Flight f;
        if (!pq_pop(&f)) {
            pthread_mutex_unlock(&queue_lock);
            continue;
        }

        /* Mark runway busy (under queue_lock for atomicity) */
        pthread_mutex_lock(&runway_lock[rw]);
        runway_busy[rw] = 1;
        pthread_mutex_unlock(&runway_lock[rw]);

        long arrival = wtmap_get(f.id);
        long waited  = time(NULL) - arrival;
        wtmap_remove(f.id);

        printf("[RWY %-2d] %-20s priority=%-3d waited=%lds -> ASSIGNED\n", rw + 1, f.label, f.priority, waited);
        pthread_mutex_unlock(&queue_lock);

        /* ── Simulate flight operation ── */
        int op_time = (f.type == EMERGENCY) ? 1 : 2;

        for (int s = 0; s < op_time; s++) {
            sleep(1);
            pthread_mutex_lock(&queue_lock);
            int ep = emergency_pending;
            pthread_mutex_unlock(&queue_lock);
            if (ep && f.type != EMERGENCY) {
                printf("[RWY %-2d] %-20s → PREEMPTED (emergency waiting)\n", rw + 1, f.label);
                /* Re-queue the interrupted flight so it is not lost */
                pthread_mutex_lock(&queue_lock);
                pq_push(f);
                wtmap_add(f.id, f.arrival_time);
                pthread_cond_broadcast(&queue_cond);
                pthread_mutex_unlock(&queue_lock);

                pthread_mutex_lock(&runway_lock[rw]);
                runway_busy[rw] = 0;
                pthread_mutex_unlock(&runway_lock[rw]);
                goto next_flight;
            }
        }

        /* ── Flight completed normally ── */
        pthread_mutex_lock(&queue_lock);
        pthread_mutex_lock(&runway_lock[rw]);
        runway_busy[rw] = 0;
        pthread_mutex_unlock(&runway_lock[rw]);

        flights_done++;
        if (f.type == EMERGENCY) {
            int more = 0;
            for (int i = 0; i < heap_size; i++)
                if (heap[i].type == EMERGENCY) { more = 1; break; }
            if (!more) emergency_pending = 0;
        }
        printf("[RWY %-2d] %-20s -> DONE (%d/%d)\n", rw + 1, f.label, flights_done, TOTAL_FLIGHTS);
        pthread_cond_broadcast(&queue_cond);
        pthread_mutex_unlock(&queue_lock);

        next_flight:;   // go to next flight
    }
    return NULL;
}

/* Emergency Monitor Thread*/
static void *emergency_monitor(void *arg) {
    (void)arg;

    while (1) {
        sleep(2);

        pthread_mutex_lock(&queue_lock);

        if (flights_done >= TOTAL_FLIGHTS && heap_size == 0) {
            pthread_mutex_unlock(&queue_lock);
            break;
        }

        int found_emergency = 0;
        for (int i = 0; i < heap_size; i++) {
            if (heap[i].type == EMERGENCY) {
                found_emergency = 1;
                if (heap[i].priority < EMERGENCY) {
                    heap[i].priority = EMERGENCY;
                }
                printf("[EMRG  ] %-20s detected – priority set to %d,preemption flag SET\n", heap[i].label, heap[i].priority);
            }
        }

        /* Rebuild heap once cleanly after all priority changes */
        if (found_emergency) {
            for (int i = heap_size / 2 - 1; i >= 0; i--)
                sift_down(i);
            emergency_pending = 1;
            pthread_cond_broadcast(&queue_cond);
        }

        /* Random runway maintenance toggle (only when runway is idle) */
        int rw = rand() % NUM_RUNWAYS;
        pthread_mutex_lock(&runway_lock[rw]);
        if (!runway_busy[rw]) {
            runway_maint[rw] = !runway_maint[rw];
            printf("[MAINT ] Runway %d maintenance %s\n", rw + 1, runway_maint[rw] ? "ON" : "OFF");
            if (!runway_maint[rw])
                pthread_cond_broadcast(&queue_cond);
        }
        pthread_mutex_unlock(&runway_lock[rw]);
        pthread_mutex_unlock(&queue_lock);
    }
    return NULL;
}

int main(void) {
    srand((unsigned)time(NULL));

    /* Initialise per-runway mutexes */
    for (int i = 0; i < NUM_RUNWAYS; i++)
        pthread_mutex_init(&runway_lock[i], NULL);

    memset(waiting_map, 0, sizeof(waiting_map));

    printf("========================================\n");
    printf("        Airport Control System\n");
    printf("  Runways: %d  |  Flights: %d\n", NUM_RUNWAYS, TOTAL_FLIGHTS);
    printf("========================================\n\n");

    pthread_t gen, monitor;
    pthread_t controllers[NUM_RUNWAYS];
    int rw_ids[NUM_RUNWAYS];

    pthread_create(&gen,     NULL, flight_generator,  NULL);
    pthread_create(&monitor, NULL, emergency_monitor, NULL);

    for (int i = 0; i < NUM_RUNWAYS; i++) {
        rw_ids[i] = i;
        pthread_create(&controllers[i], NULL, runway_controller, &rw_ids[i]);
    }

    pthread_join(gen, NULL);
    for (int i = 0; i < NUM_RUNWAYS; i++)
        pthread_join(controllers[i], NULL);

    /* Signal monitor and join it too */
    pthread_mutex_lock(&queue_lock);
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_lock);
    pthread_join(monitor, NULL);

    /* Cleanup */
    pthread_mutex_destroy(&queue_lock);
    pthread_cond_destroy(&queue_cond);
    for (int i = 0; i < NUM_RUNWAYS; i++)
        pthread_mutex_destroy(&runway_lock[i]);

    printf("\n========================================\n");
    printf("   All %d flights handled.\n", TOTAL_FLIGHTS);
    return 0;
}
