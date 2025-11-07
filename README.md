# 🧠 Persistent Memory Leak Detector

The **Persistent Memory Leak Detector** tracks and logs unfreed memory in C programs by intercepting `malloc()` and `free()` using **LD_PRELOAD**.  
It stores allocation details in an **SQLite database** for persistent analysis.  
A **Python tool** summarizes and compares leaks across runs to detect long-term memory issues.

---

## 🎯 Objectives
- Detect dynamically allocated memory that is not freed.  
- Store all allocation and deallocation information persistently.  
- Generate detailed leak summaries and reports.  
- Compare memory leak data across multiple runs.  

---

## ⚙️ How It Works
1. **Hooking:** `malloc()` and `free()` are intercepted using `LD_PRELOAD`.  
2. **Logging:** Every allocation and deallocation is recorded in `memory_leaks.db`.  
3. **Persistence:** Data is stored permanently for future analysis.  
4. **Analysis:** The Python script summarizes leaks, compares runs, and exports reports.  

---

## 🧩 Project Structure

├── memory_hook.c       # Intercepts malloc() and free() calls

├── analyze_leaks.py    # Python script for analysis and summaries

├── live_graph.py       # Displays live visualization of memory usage

├── memory_server.c     # Handles database operations and persistent logging

└── memory_leaks.db     # SQLite database (auto-created)


---

## 🧰 Requirements
- **Operating System:** Ubuntu 22.04+ / Linux  
- **Compiler:** GCC 13+  
- **Database:** SQLite 3+  
- **Python Version:** 3.10+  

---

## 🚀 Usage

### 1️⃣ Compile the Hook Library

gcc -shared -fPIC -o memory_hook.so memory_hook.c -ldl -lsqlite3


### 2️⃣ Compile the Application


gcc memory_server.c -o memory_server


### 3️⃣ Run the Application with Memory Tracking


LD_PRELOAD=./memory_hook.so ./memory_server


### 4️⃣ Analyze the Leak Data

```bash
python3 analyze_leaks.py summary
```

Optional:

```bash
python3 analyze_leaks.py compare       # Compare two runs
```

---

## 🖥️ Output

### 1️⃣ Terminal Execution

When the project runs with:

```bash
LD_PRELOAD=./memory_hook.so ./memory_server
```

It shows initialization and memory tracking logs:

```
Memory Leak Detector Initialized
Database: memory_leaks.db

[LOG] malloc() intercepted → Address: 0x562af2d3e2a0, Size: 256 bytes
[LOG] malloc() intercepted → Address: 0x562af2d3e3b0, Size: 128 bytes
[LOG] free() intercepted   → Address: 0x562af2d3e3b0
```

---

### 2️⃣ Leak Report (Auto-Generated on Program Exit)

When the application ends, a summary report is generated:

```
=== MEMORY LEAK REPORT ===
Total Leaked Allocations: 5
Total Leaked Bytes: 1720 bytes
===========================

--- Detailed Leak Information ---
Address: 0x562af2d3e2a0 | Size: 256 bytes | Time: 2025-10-14 22:30:15
Address: 0x562af2d3f4b0 | Size: 64 bytes  | Time: 2025-10-14 22:30:16
Address: 0x562af2d3f8c0 | Size: 512 bytes | Time: 2025-10-14 22:30:17
--------------------------------
```

---

### 3️⃣ Python Leak Analysis

Command:

```bash
python3 analyze_leaks.py summary
```

Output:

```
==================================================
MEMORY LEAK DETECTOR - ANALYSIS SUMMARY
==================================================
Current Status:
  Total Leaked Allocations: 5
  Total Leaked Memory: 1720 bytes
  Average Leak Size: 344.00 bytes

Leak History (Last 10):
  1. 2025-10-13 23:12:40 - 4 leaks (1600 bytes)
  2. 2025-10-14 22:30:18 - 5 leaks (1720 bytes)

Top Leaks (by size):
  1. Address: 0x562af2d3f8c0, Size: 512 bytes
  2. Address: 0x562af2d3e2a0, Size: 256 bytes
==================================================
```

---

### 4️⃣ Database View (SQLite)

Query:

```sql
SELECT address, size, freed, timestamp FROM allocations;
```

Result:

| Address        | Size | Freed | Timestamp           |
| -------------- | ---- | ----- | ------------------- |
| 0x562af2d3e2a0 | 256  | 0     | 2025-10-14 22:30:15 |
| 0x562af2d3f4b0 | 64   | 0     | 2025-10-14 22:30:16 |
| 0x562af2d3f8c0 | 512  | 0     | 2025-10-14 22:30:17 |

---

## 🧠 Concept Summary

* Uses **LD_PRELOAD** for dynamic function hooking.
* Tracks all memory allocations and frees in real-time.
* Stores logs persistently using **SQLite**.
* Generates post-run analysis with a **Python script**.

---

## 🧾 References

* [Linux Man Pages – LD_PRELOAD & dlsym](https://man7.org/linux/man-pages/)
* [SQLite Official Documentation](https://www.sqlite.org)
* [Valgrind: Memcheck Manual](https://valgrind.org/docs/manual/mc-manual.html)
* Ulrich Drepper, *How To Write Shared Libraries*, Red Hat, 2011

---

