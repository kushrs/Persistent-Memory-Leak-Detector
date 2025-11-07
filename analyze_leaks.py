#!/usr/bin/env python3

import sqlite3
import json
from datetime import datetime
from collections import defaultdict

DB_PATH = "memory_leak.db"

def connect_db():
    try:
        conn = sqlite3.connect(DB_PATH)
        conn.row_factory = sqlite3.Row
        return conn
    except sqlite3.Error as e:
        print(f"Database error: {e}")
        return None

def get_current_leaks():
    """Get all unfreed allocations"""
    conn = connect_db()
    if not conn:
        return []
    
    cursor = conn.cursor()
    cursor.execute("SELECT address, size, timestamp FROM allocations WHERE freed = 0")
    leaks = cursor.fetchall()
    conn.close()
    return leaks

def get_leak_statistics():
    """Generate leak statistics"""
    conn = connect_db()
    if not conn:
        return None
    
    cursor = conn.cursor()
    cursor.execute("SELECT COUNT(*) as count, SUM(size) as total FROM allocations WHERE freed = 0")
    stats = cursor.fetchone()
    conn.close()
    
    return {
        "total_leaks": stats[0] if stats else 0,
        "total_bytes": stats[1] if stats else 0
    }

def get_leak_history():
    """Get historical leak reports"""
    conn = connect_db()
    if not conn:
        return []
    
    cursor = conn.cursor()
    cursor.execute("SELECT report_date, total_leaks, total_leaked_bytes FROM leak_reports ORDER BY report_date DESC LIMIT 10")
    reports = cursor.fetchall()
    conn.close()
    return reports

def compare_executions():
    """Compare leaks across multiple executions"""
    conn = connect_db()
    if not conn:
        return None
    
    cursor = conn.cursor()
    cursor.execute("""
        SELECT report_date, total_leaks, total_leaked_bytes 
        FROM leak_reports 
        ORDER BY report_date DESC
    """)
    reports = cursor.fetchall()
    conn.close()
    
    if len(reports) < 2:
        print("Not enough reports for comparison (need at least 2)")
        return None
    
    latest = reports[0]
    previous = reports[1]
    
    leak_diff = latest[1] - previous[1]
    bytes_diff = latest[2] - previous[2]
    
    print("\n=== EXECUTION COMPARISON ===")
    print(f"Latest Report ({latest[0]}):")
    print(f"  Leaks: {latest[1]}, Bytes: {latest[2]}")
    print(f"\nPrevious Report ({previous[0]}):")
    print(f"  Leaks: {previous[1]}, Bytes: {previous[2]}")
    print(f"\nDifference:")
    print(f"  Leaks: {leak_diff:+d}")
    print(f"  Bytes: {bytes_diff:+d}")
    print("=" * 30)

def export_report(filename="leak_report.json"):
    """Export leak data to JSON"""
    leaks = get_current_leaks()
    stats = get_leak_statistics()
    
    report = {
        "generated": datetime.now().isoformat(),
        "statistics": stats,
        "leaks": [{"address": l[0], "size": l[1], "timestamp": l[2]} for l in leaks]
    }
    
    with open(filename, 'w') as f:
        json.dump(report, f, indent=2)
    
    print(f"Report exported to {filename}")

def print_summary():
    """Print comprehensive summary"""
    stats = get_leak_statistics()
    history = get_leak_history()
    
    print("\n" + "=" * 50)
    print("MEMORY LEAK DETECTOR - ANALYSIS SUMMARY")
    print("=" * 50)
    
    if stats:
        print(f"\nCurrent Status:")
        print(f"  Total Leaked Allocations: {stats['total_leaks']}")
        print(f"  Total Leaked Memory: {stats['total_bytes']} bytes")
        
        if stats['total_leaks'] > 0:
            avg_size = stats['total_bytes'] / stats['total_leaks']
            print(f"  Average Leak Size: {avg_size:.2f} bytes")
    
    print(f"\nLeak History (Last 10):")
    if history:
        for i, report in enumerate(history, 1):
            print(f"  {i}. {report[0]} - {report[1]} leaks ({report[2]} bytes)")
    else:
        print("  No historical data yet")
    
    leaks = get_current_leaks()
    if leaks and len(leaks) > 0:
        print(f"\nTop Leaks (by size):")
        sorted_leaks = sorted(leaks, key=lambda x: x[1], reverse=True)[:5]
        for i, leak in enumerate(sorted_leaks, 1):
            print(f"  {i}. Address: {leak[0]}, Size: {leak[1]} bytes")
    
    print("\n" + "=" * 50)

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1:
        if sys.argv[1] == "summary":
            print_summary()
        elif sys.argv[1] == "compare":
            compare_executions()
        elif sys.argv[1] == "export":
            filename = sys.argv[2] if len(sys.argv) > 2 else "leak_report.json"
            export_report(filename)
        else:
            print("Usage: python3 analyze_leaks.py [summary|compare|export]")
    else:
        print_summary()
