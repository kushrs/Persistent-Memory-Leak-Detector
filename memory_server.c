#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define MAX_PTRS 1000

static int running = 1;
static void *ptrs[MAX_PTRS] = {NULL};
static size_t sizes[MAX_PTRS] = {0};
static int active_allocs = 0;

// Graceful shutdown
void handle_sigint(int sig) {
    printf("\n[Server] Received stop signal. Shutting down gracefully...\n");
    running = 0;
}

// Get current time as string
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Random allocation size (between 64B and 2KB)
size_t random_size() {
    return (rand() % 1985) + 64;
}

int main() {
    srand(time(NULL));
    signal(SIGINT, handle_sigint);

    printf("\n============================================\n");
    printf("  Persistent Memory Leak Detector - DEMO\n");
    printf("============================================\n");
    printf("[Server] Press Ctrl+C anytime to stop tracking.\n\n");

    int cycle = 0;

    while (running) {
        cycle++;
        char timebuf[32];
        get_timestamp(timebuf, sizeof(timebuf));
        printf("[Server] --- Cycle %d (%s) ---\n", cycle, timebuf);

        int num_ops = rand() % 5 + 3; // 3–7 operations per cycle
        for (int i = 0; i < num_ops; i++) {
            int action = rand() % 3; // 0 = alloc, 1 = free, 2 = leak

            if (action == 0) {
                size_t sz = random_size();
                void *p = malloc(sz);
                if (p) {
                    ptrs[active_allocs] = p;
                    sizes[active_allocs] = sz;
                    active_allocs++;
                    get_timestamp(timebuf, sizeof(timebuf));
                    printf("[ALLOC] %zu bytes at %p | Time: %s\n", sz, p, timebuf);
                }
            } else if (action == 1 && active_allocs > 0) {
                int idx = rand() % active_allocs;
                void *p = ptrs[idx];
                if (p) {
                    free(p);
                    get_timestamp(timebuf, sizeof(timebuf));
                    printf("[FREE ] %zu bytes at %p | Time: %s\n", sizes[idx], p, timebuf);
                    ptrs[idx] = ptrs[active_allocs - 1];
                    sizes[idx] = sizes[active_allocs - 1];
                    active_allocs--;
                }
            } else {
                get_timestamp(timebuf, sizeof(timebuf));
                printf("[LEAK ] Simulating memory leak (no free this cycle) | Time: %s\n", timebuf);
            }
        }

        printf("[Server] Active allocations: %d\n", active_allocs);
        printf("--------------------------------------------\n");
        sleep(2); // wait before next cycle
    }

    printf("[Server] Process stopped. Data persisted to DB (memory_leak.db).\n");
    printf("[Server] Use 'python3 analyze_leaks.py summary' to view report.\n");
    return 0;
}

