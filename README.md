# Persistent-Memory-Leak-Detector
The Persistent Memory Leak Detector tracks and logs unfreed memory in C programs by intercepting malloc() and free() using LD_PRELOAD. It stores allocation details in an SQLite database for persistent analysis. A Python tool summarizes and compares leaks across runs to detect long-term memory issues.
