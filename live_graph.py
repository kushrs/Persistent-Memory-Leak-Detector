import sqlite3
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

DB_FILE = "memory_leak.db"

times = []
leak_sizes = []
leak_blocks = []

def fetch_stats():
    conn = sqlite3.connect(DB_FILE)
    cur = conn.cursor()
    cur.execute("SELECT COUNT(*), IFNULL(SUM(size),0) FROM allocations WHERE freed = 0")
    blocks, bytes_ = cur.fetchone()
    conn.close()
    return blocks, bytes_

def update(frame):
    blocks, bytes_ = fetch_stats()
    
    current_time = time.strftime("%H:%M:%S")
    times.append(current_time)
    leak_sizes.append(bytes_ / (1024 * 1024))   # Convert to MB
    leak_blocks.append(blocks)
    
    plt.cla()

    plt.subplot(2,1,1)
    plt.plot(times, leak_sizes, marker='o')
    plt.title("Live Memory Leak Monitor")
    plt.ylabel("Leaked Memory (MB)")
    plt.xticks(rotation=45, fontsize=8)

    plt.subplot(2,1,2)
    plt.plot(times, leak_blocks, marker='x', color='r')
    plt.ylabel("Leaked Blocks")
    plt.xlabel("Time")
    plt.xticks(rotation=45, fontsize=8)

ani = FuncAnimation(plt.gcf(), update, interval=2000)  # refresh every 2 seconds

plt.tight_layout()
plt.show()

