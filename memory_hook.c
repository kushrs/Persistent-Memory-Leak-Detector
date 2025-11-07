#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

typedef void *(*malloc_t)(size_t size);
typedef void (*free_t)(void *ptr);

static malloc_t real_malloc = NULL;
static free_t real_free = NULL;
static sqlite3 *hook_db = NULL;
static int initialized = 0;
static pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

static void safe_log(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

// Initialize DB safely, only called once, after first malloc from app
static void init_db_once() {
    if (initialized) return;
    initialized = 1;

    safe_log("[memory_hook] Initializing hooks...\n");

    real_malloc = (malloc_t)dlsym(RTLD_NEXT, "malloc");
    real_free = (free_t)dlsym(RTLD_NEXT, "free");
    if (!real_malloc || !real_free) {
        safe_log("[memory_hook] ERROR: Could not resolve malloc/free.\n");
        _exit(1);
    }

    if (sqlite3_open("memory_leak.db", &hook_db) != SQLITE_OK) {
        safe_log("[memory_hook] ERROR: Could not open DB.\n");
        hook_db = NULL;
        return;
    }

const char *sql =
    "CREATE TABLE IF NOT EXISTS allocations ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "address TEXT NOT NULL,"
    "size INTEGER NOT NULL,"
    "process_id INTEGER NOT NULL,"
    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "freed BOOLEAN DEFAULT 0,"
    "freed_timestamp DATETIME"
    ");";



    sqlite3_exec(hook_db, sql, 0, 0, 0);
    safe_log("[memory_hook] Hooks initialized successfully.\n");
}

// Hooked malloc
static __thread int in_hook = 0; // thread-local flag

void *malloc(size_t size) {
    if (!real_malloc)
        real_malloc = (malloc_t)dlsym(RTLD_NEXT, "malloc");

    if (in_hook) {
        return real_malloc(size);  // don't log recursively
    }

    in_hook = 1; // enter hook
    void *ptr = real_malloc(size);

    if (!initialized)
        init_db_once();

    if (hook_db && ptr) {
        pthread_mutex_lock(&db_lock);

        sqlite3_stmt *stmt;
        // Include timestamp column
        const char *sql = "INSERT INTO allocations(address, size, process_id, timestamp) "
                          "VALUES(?, ?, ?, datetime('now'));";

        if (sqlite3_prepare_v2(hook_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            char addr[32];
            snprintf(addr, sizeof(addr), "%p", ptr);
            sqlite3_bind_text(stmt, 1, addr, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, (int)size);
            sqlite3_bind_int(stmt, 3, getpid());
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        pthread_mutex_unlock(&db_lock);
    }

    in_hook = 0; // exit hook
    return ptr;
}
void free(void *ptr) {
    if (!real_free)
        real_free = (free_t)dlsym(RTLD_NEXT, "free");

    if (in_hook) {
        real_free(ptr); // don't log recursively
        return;
    }

    in_hook = 1;

    if (hook_db && ptr) {
        pthread_mutex_lock(&db_lock);

        sqlite3_stmt *stmt;
        // Update freed status AND store freed timestamp
        const char *sql = "UPDATE allocations SET freed = 1, freed_timestamp = datetime('now') "
                          "WHERE address = ? AND freed = 0;";

        if (sqlite3_prepare_v2(hook_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            char addr[32];
            snprintf(addr, sizeof(addr), "%p", ptr);
            sqlite3_bind_text(stmt, 1, addr, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        pthread_mutex_unlock(&db_lock);
    }

    real_free(ptr);
    in_hook = 0;
}

__attribute__((destructor))
void cleanup_hook() {
    safe_log("[memory_hook] Cleaning up...\n");
    if (hook_db) {
        sqlite3_close(hook_db);
        hook_db = NULL;
    }
}

