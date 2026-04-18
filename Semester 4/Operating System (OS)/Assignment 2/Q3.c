#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define CRITICAL 0
#define SERIOUS  1
#define NORMAL   2
#define SENIOR   1
#define JUNIOR   0

#define MAX_WAIT           8
#define SERIOUS_THRESHOLD  5
#define NORMAL_RUN_LIMIT   3
#define MAX_PATIENTS      20

typedef struct {
    int    id, type, treated, assigned_doctor;
    time_t arrival_time;
    pthread_cond_t done;
} Patient;

typedef struct {
    int id, is_senior, normal_count;
} Doctor;

/* ── Three simple circular queues ── */
#define QMAX 30
static Patient *queue[3][QMAX];
static int      qhead[3] = {0}, qtail[3] = {0}, qsize[3] = {0};

static void q_push(int lvl, Patient *p) {
    if (qsize[lvl] >= QMAX) {
        fprintf(stderr, "[ERROR] Queue %d overflow! Patient %d dropped.\n", lvl, p->id);
        return;
    }
    queue[lvl][qtail[lvl]] = p;
    qtail[lvl] = (qtail[lvl] + 1) % QMAX;
    qsize[lvl]++;
}

static Patient *q_pop(int lvl) {
    if (qsize[lvl] == 0) return NULL;
    Patient *p = queue[lvl][qhead[lvl]];
    qhead[lvl] = (qhead[lvl] + 1) % QMAX;
    qsize[lvl]--;
    return p;
}

static int total_patients     = MAX_PATIENTS;
static int completed_patients = 0;
static int simulation_done    = 0;   /* watchdog exit flag */

static pthread_mutex_t mtx         = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  any_patient = PTHREAD_COND_INITIALIZER;

static const char *pname(int t) {
    return t == CRITICAL ? "Critical" : t == SERIOUS ? "Serious" : "Normal";
}

static const char *dname(int s) { 
    return s == SENIOR ? "Senior" : "Junior"; 
}

/* ── Rule 3: Promotions on threshold ── */
static void check_threshold(void) {
    if (qsize[SERIOUS] >= SERIOUS_THRESHOLD) {
        Patient *p = q_pop(SERIOUS);
        p->type = CRITICAL;
        q_push(CRITICAL, p);
        printf("[PROMOTE] Patient %d: Serious -> Critical (queue threshold reached)\n", p->id);
    }
}

/*
 * Rule 5: direct promotion to Critical/Serious after MAX_WAIT.
 *   Normal   -> Critical  (not chained via Serious first)
 *   Serious  -> Critical
 * Previously Normal->Serious->Critical required 60s total, violating the rule.
 * Caller MUST hold mtx.
 */
static void check_timeouts(void) {
    time_t now = time(NULL);

    /* Serious → Critical */
    int n = qsize[SERIOUS];
    for (int i = 0; i < n; i++) {
        Patient *p = q_pop(SERIOUS);
        if (now - p->arrival_time >= MAX_WAIT) {
            p->type = CRITICAL;
            q_push(CRITICAL, p);
            printf("[PROMOTE] Patient %d: Serious -> Critical (%d: timeout)\n", p->id, MAX_WAIT);
        } else {
            q_push(SERIOUS, p);
        }
    }

    /* Normal → Critical (direct, not chained) */
    n = qsize[NORMAL];
    for (int i = 0; i < n; i++) {
        Patient *p = q_pop(NORMAL);
        if (now - p->arrival_time >= MAX_WAIT) {
            p->type = CRITICAL;
            q_push(CRITICAL, p);
            printf("[PROMOTE] Patient %d: Normal -> Critical (%d: timeout)\n", p->id, MAX_WAIT);
        } else {
            q_push(NORMAL, p);
        }
    }
}

/* Watchdog thread: checks timeouts every second continuously */
static void *watchdog_thread(void *arg) {
    (void)arg;
    while (1) {
        sleep(1);
        pthread_mutex_lock(&mtx);
        if (simulation_done) {
            pthread_mutex_unlock(&mtx);
            break;
        }
        check_timeouts();
        /* wake doctors in case a promotion created a new critical patient */
        pthread_cond_broadcast(&any_patient);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

/*
    Select_patient 
    Senior doctors rely on priority ordering here to grab critical first.  This guarantees a free senior doctor will always pick up a critical patient immediately.
    Rule 2: Explicit junior guard — if critical patients are waiting and this doctor is junior, return NULL so the junior keeps waiting rather than skipping critical silently via ordering alone.
 */
static Patient *select_patient(Doctor *doc) {
    /* Rule 2: junior cannot treat critical */
    if (doc->is_senior == JUNIOR && qsize[CRITICAL] > 0) return NULL;

    if (doc->is_senior == SENIOR && qsize[CRITICAL] > 0) return q_pop(CRITICAL);     /* Seniors handle critical first */
    if (doc->normal_count >= NORMAL_RUN_LIMIT && qsize[SERIOUS] > 0)  return q_pop(SERIOUS);      /* Rule 4: after 3 normals → prefer serious */

    /* Normal priority order */
    if (qsize[SERIOUS] > 0) return q_pop(SERIOUS);
    if (qsize[NORMAL]  > 0) return q_pop(NORMAL);
    return NULL;
}

/* ── Patient storage ── */
static Patient all_patients[MAX_PATIENTS];

/* Arrival_time set INSIDE the thread after acquiring the lock so the timer starts at the moment the patient actually enters the queue,*/
static void *patient_thread(void *arg) {
    Patient *p = (Patient *)arg;
    pthread_mutex_lock(&mtx);

    p->arrival_time = time(NULL);
    printf("[ARRIVE ] Patient %d arrived as %s\n", p->id, pname(p->type));

    q_push(p->type, p);
    check_threshold();              /* Rule 3 */
    pthread_cond_broadcast(&any_patient);

    while (!p->treated)
        pthread_cond_wait(&p->done, &mtx);

    printf("[TREATED] Patient %d (%s) by Doctor %d\n", p->id, pname(p->type), p->assigned_doctor);
    pthread_mutex_unlock(&mtx);
    return NULL;
}

/* ── Doctor thread ── */
static void *doctor_thread(void *arg) {
    Doctor *doc = (Doctor *)arg;
    while (1) {
        pthread_mutex_lock(&mtx);
        Patient *p = select_patient(doc);

        while (p == NULL && completed_patients < total_patients) {
            pthread_cond_wait(&any_patient, &mtx);
            p = select_patient(doc);
        }

        if (completed_patients >= total_patients) {
            pthread_mutex_unlock(&mtx);
            break;
        }

        p->assigned_doctor = doc->id;
        printf("[START  ] Doctor %d (%s) -> Patient %d (%s)\n",  doc->id, dname(doc->is_senior), p->id, pname(p->type));
        pthread_mutex_unlock(&mtx);

        sleep(2);   /* simulate treatment which takes 2 seconds */

        pthread_mutex_lock(&mtx);
        doc->normal_count = (p->type == NORMAL) ? doc->normal_count + 1 : 0;
        p->treated = 1;
        completed_patients++;
        pthread_cond_signal(&p->done);
        pthread_cond_broadcast(&any_patient);
        pthread_mutex_unlock(&mtx);
    }

    printf("[EXIT   ] Doctor %d (%s) off duty.\n", doc->id, dname(doc->is_senior));
    return NULL;
}

int main() {
    Doctor doctors[4] = {
        {1, SENIOR, 0}, {2, SENIOR, 0},
        {3, JUNIOR, 0}, {4, JUNIOR, 0}
    };

    int data[MAX_PATIENTS][2] = {
        {1,NORMAL},{2,SERIOUS},{3,NORMAL},{4,NORMAL},
        {5,SERIOUS},{6,SERIOUS},{7,SERIOUS},{8,SERIOUS},
        {9,SERIOUS},   /* 5 serious in queue -> Rule 3 fires */
        {10,CRITICAL},{11,NORMAL},{12,NORMAL},
        {13,NORMAL},   /* 3 consecutive normals -> Rule 4 fires */
        {14,SERIOUS},{15,CRITICAL},{16,NORMAL},
        {17,SERIOUS},{18,NORMAL},{19,NORMAL},{20,SERIOUS}
    };

    for (int i = 0; i < MAX_PATIENTS; i++) {
        all_patients[i].id              = data[i][0];
        all_patients[i].type            = data[i][1];
        all_patients[i].treated         = 0;
        all_patients[i].assigned_doctor = -1;
        all_patients[i].arrival_time    = 0;
        pthread_cond_init(&all_patients[i].done, NULL);
    }

    printf("==============================================\n");
    printf("  Hospital Emergency Department Simulation\n");
    printf("  Doctors: 2 Senior, 2 Junior | Patients: %d\n", total_patients);
    printf("==============================================\n\n");

    pthread_t dt[4], pt[MAX_PATIENTS], wt;

    /* start watchdog first (FIX 4) */
    pthread_create(&wt, NULL, watchdog_thread, NULL);

    for (int i = 0; i < 4; i++)
        pthread_create(&dt[i], NULL, doctor_thread, &doctors[i]);

    for (int i = 0; i < MAX_PATIENTS; i++) {
        pthread_create(&pt[i], NULL, patient_thread, &all_patients[i]);
        usleep(400000);
    }

    for (int i = 0; i < MAX_PATIENTS; i++)
        pthread_join(pt[i], NULL);

    /* signal doctors they can exit */
    pthread_mutex_lock(&mtx);
    pthread_cond_broadcast(&any_patient);
    pthread_mutex_unlock(&mtx);

    for (int i = 0; i < 4; i++)
        pthread_join(dt[i], NULL);

    /* stop watchdog */
    pthread_mutex_lock(&mtx);
    simulation_done = 1;
    pthread_mutex_unlock(&mtx);
    pthread_join(wt, NULL);

    for (int i = 0; i < MAX_PATIENTS; i++)
        pthread_cond_destroy(&all_patients[i].done);

    printf("\n==============================================\n");
    printf("  All %d patients treated successfully.\n", total_patients);
    printf("==============================================\n");

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&any_patient);
    return 0;
}
