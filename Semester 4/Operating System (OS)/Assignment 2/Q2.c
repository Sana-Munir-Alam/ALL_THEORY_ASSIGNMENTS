#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ── tunables ─────────────────────────────────────────────────────────── */
#define N           1000
#define T_HOT       35.0f
#define T_COLD     -10.0f
#define NUM_THREADS    5

/* ── satellite data types ─────────────────────────────────────────────── */
typedef struct {
    float data[N][N];
    int   satellite_id;
} SatelliteMatrix;

typedef struct {
    float          global_mat[N][N];
    int            overlap_count[N][N];
    pthread_mutex_t row_mutex[N];
} Climate;

typedef struct {
    Climate        *mainmap;
    SatelliteMatrix satellitemap;
} SatelliteWorkerArgs;

Climate main_map;

void initClimate(Climate *c) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            c->global_mat[i][j]    = 0.0f;
            c->overlap_count[i][j] = 0;
        }
        pthread_mutex_init(&c->row_mutex[i], NULL);
    }
}

void generateSatelliteMatrix(SatelliteMatrix *res, int id) {
    res->satellite_id = id;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            res->data[i][j] = ((i + j) % 10 == 0) ? NAN : (float)(rand() % 70 - 20);
}

void interpolateNAN(float mat[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (!isnan(mat[i][j])) continue;
            float sum = 0.0f;
            int   cnt = 0;
            if (i > 0   && !isnan(mat[i-1][j])) { sum += mat[i-1][j]; cnt++; }
            if (i < N-1 && !isnan(mat[i+1][j])) { sum += mat[i+1][j]; cnt++; }
            if (j > 0   && !isnan(mat[i][j-1])) { sum += mat[i][j-1]; cnt++; }
            if (j < N-1 && !isnan(mat[i][j+1])) { sum += mat[i][j+1]; cnt++; }
            mat[i][j] = (cnt > 0) ? sum / cnt : 20.0f;
        }
    }
}

void *satelliteWorker(void *args) {
    SatelliteWorkerArgs *s_args = (SatelliteWorkerArgs *)args;
    SatelliteMatrix     *s_data = &s_args->satellitemap;
    Climate             *c_map  =  s_args->mainmap;

    interpolateNAN(s_data->data);

    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&c_map->row_mutex[i]);
        for (int j = 0; j < N; j++) {
            c_map->global_mat[i][j]    += s_data->data[i][j];
            c_map->overlap_count[i][j] += 1;
        }
        pthread_mutex_unlock(&c_map->row_mutex[i]);
    }
    printf("[satellite] Satellite %d processing complete.\n", s_data->satellite_id);
    return NULL;
}

/* ── tile-based statistics ────────────────────────────────────────────── */
#define TILE_SIZE   100
#define NUM_TILES_X (N / TILE_SIZE)
#define TOTAL_TILES (NUM_TILES_X * NUM_TILES_X)

static int             tile_index = 0;
static pthread_mutex_t tile_lock  = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    float          global_max, global_min, global_sum, global_sum_sq;
    int            anomaly_count;
    pthread_mutex_t lock;
} GlobalStats;

GlobalStats g_stats = {
    .global_max   = -INFINITY,
    .global_min   =  INFINITY,
    .global_sum   = 0.f,
    .global_sum_sq= 0.f,
    .anomaly_count= 0,
    .lock         = PTHREAD_MUTEX_INITIALIZER
};

typedef struct {
    float      (*matrix)[N];
    GlobalStats *stats;
} TileWorkerArgs;

static int getNextTile(void) {
    pthread_mutex_lock(&tile_lock);
    int idx = (tile_index < TOTAL_TILES) ? tile_index++ : -1;
    pthread_mutex_unlock(&tile_lock);
    return idx;
}

void *tileWorker(void *arg) {
    TileWorkerArgs *args  = (TileWorkerArgs *)arg;
    float        (*mat)[N]= args->matrix;
    GlobalStats   *stats  = args->stats;

    while (1) {
        int tile = getNextTile();
        if (tile == -1) break;

        int rowstart = (tile / NUM_TILES_X) * TILE_SIZE;
        int colstart = (tile % NUM_TILES_X) * TILE_SIZE;

        float  local_min = INFINITY, local_max = -INFINITY;
        double local_sum = 0.0,      local_sum_sq = 0.0;
        int    cell_count = 0,       local_anomalies = 0;

        for (int i = rowstart; i < rowstart + TILE_SIZE; i++)
            for (int j = colstart; j < colstart + TILE_SIZE; j++) {
                float val = mat[i][j];
                if (val < local_min) local_min = val;
                if (val > local_max) local_max = val;
                local_sum    += val;
                local_sum_sq += (double)val * val;
                cell_count++;
            }

        float local_mean     = (float)(local_sum / cell_count);
        float local_variance = (float)(local_sum_sq / cell_count)
                               - local_mean * local_mean;
        float local_std      = sqrtf(local_variance);

        for (int i = rowstart; i < rowstart + TILE_SIZE; i++)
            for (int j = colstart; j < colstart + TILE_SIZE; j++)
                if (fabsf(mat[i][j] - local_mean) > 2.f * local_std)
                    local_anomalies++;

        pthread_mutex_lock(&stats->lock);
        if (local_min < stats->global_min) stats->global_min = local_min;
        if (local_max > stats->global_max) stats->global_max = local_max;
        stats->global_sum    += (float)local_sum;
        stats->global_sum_sq += (float)local_sum_sq;
        stats->anomaly_count += local_anomalies;
        pthread_mutex_unlock(&stats->lock);
    }
    return NULL;
}

/* ── task-parallel phase globals ──────────────────────────────────────── */
bool is_hotspot[N][N]  = {false};
bool is_coldspot[N][N] = {false};
bool near_both[N][N]   = {false};

int rows_processed_A = 0, rows_processed_B = 0;
pthread_mutex_t progress_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  progress_cond = PTHREAD_COND_INITIALIZER;

int hotspot_count  = 0, coldspot_count = 0;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;

float normalized_mat[N][N];
float risk_score[N][N];

static pthread_barrier_t barrier_abc;   /* A, B, C signal "primary work done" */
static pthread_barrier_t barrier_risk;  /* C → Risk handoff                   */

void mark_near_both(int i, int j, bool is_hot) {
    for (int di = -2; di <= 2; di++) {
        for (int dj = -2; dj <= 2; dj++) {
            if (abs(di) + abs(dj) > 2) continue;
            int ni = i + di, nj = j + dj;
            if (ni < 0 || ni >= N || nj < 0 || nj >= N) continue;
            if (( is_hot && is_coldspot[ni][nj]) || (!is_hot && is_hotspot[ni][nj])) {
                near_both[i][j]   = true;
                near_both[ni][nj] = true;
            }
        }
    }
}

/* ── Task A: hotspot detection ────────────────────────────────────────── */
void *taskA_hotspots(void *arg) {
    (void)arg;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (main_map.global_mat[i][j] > T_HOT) {
                pthread_mutex_lock(&flag_mutex);
                is_hotspot[i][j] = true;
                mark_near_both(i, j, true);
                pthread_mutex_unlock(&flag_mutex);

                pthread_mutex_lock(&count_lock);
                hotspot_count++;
                pthread_mutex_unlock(&count_lock);
            }
        }
        /* signal taskC that row i is ready */
        pthread_mutex_lock(&progress_lock);
        rows_processed_A = i + 1;
        pthread_cond_broadcast(&progress_cond);
        pthread_mutex_unlock(&progress_lock);
    }

    printf("[taskA] Hotspot detection done - %d hotspots found.\n",  hotspot_count);
    pthread_barrier_wait(&barrier_abc);
    return NULL;
}

/* ── Task B: coldspot detection ───────────────────────────────────────── */
void *taskB_coldspots(void *arg) {
    (void)arg;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (main_map.global_mat[i][j] < T_COLD) {
                pthread_mutex_lock(&flag_mutex);
                is_coldspot[i][j] = true;
                mark_near_both(i, j, false);
                pthread_mutex_unlock(&flag_mutex);

                pthread_mutex_lock(&count_lock);
                coldspot_count++;
                pthread_mutex_unlock(&count_lock);
            }
        }
        pthread_mutex_lock(&progress_lock);
        rows_processed_B = i + 1;
        pthread_cond_broadcast(&progress_cond);
        pthread_mutex_unlock(&progress_lock);
    }

    printf("[taskB] Coldspot detection done - %d coldspots found.\n", coldspot_count);

    pthread_barrier_wait(&barrier_abc);
    return NULL;
}

void *taskC_normalize(void *arg) {
    (void)arg;
    float global_min = g_stats.global_min;
    float global_max = g_stats.global_max;
    float range      = global_max - global_min;
    if (range == 0.f) range = 1.f;

    int processed_rows = 0;

    /* ---- pipelined normalization pass (no near_both yet) ---- */
    while (processed_rows < N) {
        /* acquire lock, snapshot ra/rb, determine ready_rows, then release — never read the shared vars outside the lock */
        pthread_mutex_lock(&progress_lock);
        while (1) {
            int ra    = rows_processed_A;   /* read inside lock */
            int rb    = rows_processed_B;
            int ready = (ra < rb) ? ra : rb;

            if (ready > processed_rows) break;  /* new rows available  */
            if (ra == N && rb == N)     break;  /* both fully done     */
            pthread_cond_wait(&progress_cond, &progress_lock);
        }
        /* snapshot INSIDE the lock (still holding it) */
        int ra = rows_processed_A;
        int rb = rows_processed_B;
        pthread_mutex_unlock(&progress_lock);
        /* ← lock released; ra/rb are local copies, no race */

        /* Determine how many rows are safe to normalise now. If both A and B are fully done, flush everything (ready = N). */
        int ready_rows = (ra < rb) ? ra : rb;
        if (ra == N && rb == N) ready_rows = N; /* flush first */
        if (ready_rows < processed_rows) ready_rows = processed_rows;

        for (int i = processed_rows; i < ready_rows; i++)
            for (int j = 0; j < N; j++) {
                float val  = main_map.global_mat[i][j];
                normalized_mat[i][j] = (val - global_min) / range;
            }
        processed_rows = ready_rows;
    }

    pthread_barrier_wait(&barrier_abc); /* ---- wait for A and B to fully complete ---- */

    /*  A and B are done; near_both[] is now fully stable (no writers).  */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (near_both[i][j])
                normalized_mat[i][j] *= 0.5f;

    printf("[taskC] Normalisation (+ near_both adjustment) done.\n");
    pthread_barrier_wait(&barrier_risk);    /* hand off to riskScoreWorker via barrier_risk */
    return NULL;
}

/* ── Risk Score thread ────────────────────────────────────────────────── */
void *riskScoreWorker(void *arg) {
    (void)arg;
    /* wait until taskC (normalization + near_both) is fully done */
    pthread_barrier_wait(&barrier_risk);
    printf("[riskScore] Risk score thread started.\n");

    static float prox_hot [N][N];
    static float prox_cold[N][N];
    memset(prox_hot,  0, sizeof(prox_hot));
    memset(prox_cold, 0, sizeof(prox_cold));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (is_hotspot[i][j] || is_coldspot[i][j]) {
                for (int di = -2; di <= 2; di++)
                    for (int dj = -2; dj <= 2; dj++) {
                        int dist = abs(di) + abs(dj);
                        if (dist > 2) continue;
                        int ni = i + di, nj = j + dj;
                        if (ni < 0 || ni >= N || nj < 0 || nj >= N) continue;
                        float p = 1.0f / (float)(dist + 1);
                        if (is_hotspot [i][j] && p > prox_hot [ni][nj])
                            prox_hot [ni][nj] = p;
                        if (is_coldspot[i][j] && p > prox_cold[ni][nj])
                            prox_cold[ni][nj] = p;
                    }
            }
        }
    }

    /* min-heap for top-10 */
    typedef struct { int i, j; float score; } RiskEntry;
    RiskEntry heap[10];
    int heap_size = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float score = normalized_mat[i][j] * prox_hot[i][j]  / (prox_cold[i][j] + 1.0f);
            risk_score[i][j] = score;

            if (heap_size < 10) {
                heap[heap_size++] = (RiskEntry){i, j, score};
                int idx = heap_size - 1;
                while (idx > 0) {
                    int parent = (idx - 1) / 2;
                    if (heap[idx].score < heap[parent].score) {
                        RiskEntry tmp = heap[idx];
                        heap[idx] = heap[parent];
                        heap[parent] = tmp;
                        idx = parent;
                    } else break;
                }
            } else if (score > heap[0].score) {
                heap[0] = (RiskEntry){i, j, score};
                int idx = 0;
                while (1) {
                    int l = 2*idx+1, r = 2*idx+2, sm = idx;
                    if (l < heap_size && heap[l].score < heap[sm].score) sm = l;
                    if (r < heap_size && heap[r].score < heap[sm].score) sm = r;
                    if (sm == idx) break;
                    RiskEntry tmp = heap[idx]; heap[idx] = heap[sm]; heap[sm] = tmp;
                    idx = sm;
                }
            }
        }
    }

    /* sort top-10 descending */
    for (int i = 0; i < heap_size - 1; i++) {
        int mx = i;
        for (int k = i+1; k < heap_size; k++)
            if (heap[k].score > heap[mx].score) mx = k;
        if (mx != i) {
            RiskEntry tmp = heap[i]; heap[i] = heap[mx]; heap[mx] = tmp;
        }
    }

    printf("\nTop 10 Risk Cells:\n");
    for (int k = 0; k < heap_size; k++)
        printf("  Rank %2d: (%4d,%4d)  score=%.6f\n", k+1, heap[k].i, heap[k].j, heap[k].score);

    return NULL;
}

int main(void) {
    srand((unsigned)time(NULL));
    initClimate(&main_map);

    /* ---- Phase 1: satellite merge ---- */
    pthread_t threads[NUM_THREADS];
    SatelliteWorkerArgs *sat_args =
        malloc(sizeof(SatelliteWorkerArgs) * NUM_THREADS);
    if (!sat_args) { perror("malloc"); return 1; }

    for (int i = 0; i < NUM_THREADS; i++) {
        sat_args[i].mainmap = &main_map;
        generateSatelliteMatrix(&sat_args[i].satellitemap, i + 1);
        pthread_create(&threads[i], NULL, satelliteWorker, &sat_args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    /* average overlapping cells */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            main_map.global_mat[i][j] =
                (main_map.overlap_count[i][j] > 0) ? main_map.global_mat[i][j] / main_map.overlap_count[i][j] : 20.f;

    /* destroy per-row mutexes now that merge is complete */
    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&main_map.row_mutex[i]);

    printf("Global Matrix Merged and Averaged.\n");

    /* ---- Phase 2: tile statistics ---- */
    int        num_tile_threads = 8;
    pthread_t  tile_threads[num_tile_threads];
    TileWorkerArgs tile_args = { .matrix = main_map.global_mat, .stats  = &g_stats };
    for (int i = 0; i < num_tile_threads; i++)
        pthread_create(&tile_threads[i], NULL, tileWorker, &tile_args);
    for (int i = 0; i < num_tile_threads; i++)
        pthread_join(tile_threads[i], NULL);

    float global_mean     = g_stats.global_sum / (float)(N * N);
    float global_variance = g_stats.global_sum_sq / (float)(N * N) - global_mean * global_mean;

    printf("Global Statistics:\n");
    printf("  Max:      %.2f\n", g_stats.global_max);
    printf("  Min:      %.2f\n", g_stats.global_min);
    printf("  Mean:     %.2f\n", global_mean);
    printf("  Variance: %.2f\n", global_variance);
    printf("  Anomalies (>2 sigma local): %d\n", g_stats.anomaly_count);

    /* ---- Phase 3: task-parallel (A, B, C, Risk) ---- */
    pthread_barrier_init(&barrier_abc,  NULL, 3);
    pthread_barrier_init(&barrier_risk, NULL, 2);

    pthread_t threadA, threadB, threadC, threadRisk;
    pthread_create(&threadA,    NULL, taskA_hotspots,  NULL);
    pthread_create(&threadB,    NULL, taskB_coldspots, NULL);
    pthread_create(&threadC,    NULL, taskC_normalize, NULL);
    pthread_create(&threadRisk, NULL, riskScoreWorker, NULL);

    pthread_join(threadA,    NULL);
    pthread_join(threadB,    NULL);
    pthread_join(threadC,    NULL);
    pthread_join(threadRisk, NULL);

    int near_both_count = 0;
    for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        if (near_both[i][j]) near_both_count++;

    printf("\nNear-both cells (entire matrix): %d\n", near_both_count);

    pthread_barrier_destroy(&barrier_abc);
    pthread_barrier_destroy(&barrier_risk);

    /* ---- Final output ---- */
    printf("\nHotspots:  %d\n", hotspot_count);
    printf("Coldspots: %d\n",   coldspot_count);

    printf("\nSample 5x5 normalized matrix (top-left):\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            printf("%7.4f ", normalized_mat[i][j]);
        printf("\n");
    }

    printf("\nSample 5x5 weighted-adjustment flags (top-left):\n");
    printf("(ADJ = near_both *0.5 applied; --- = normal)\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            printf("%s ", near_both[i][j] ? " ADJ" : " ---");
        printf("\n");
    }
    printf("\n\n NOTE: Weighted adjustments are rare due to strict spatial constraints,\n so they may not appear in a small 5x5 sample, but the global count confirms correctness.");

    pthread_mutex_destroy(&tile_lock);
    pthread_mutex_destroy(&g_stats.lock);
    pthread_mutex_destroy(&progress_lock);
    pthread_mutex_destroy(&count_lock);
    pthread_mutex_destroy(&flag_mutex);
    free(sat_args);
    return 0;
}
