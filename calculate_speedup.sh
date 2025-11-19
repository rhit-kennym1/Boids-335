#!/bin/bash

# Compute speedup values from benchmark results

if [ ! -f speedup_data.txt ]; then
    echo "speedup_data.txt not found"
    exit 1
fi

OUTFILE="speedup.txt"

{
    echo "Threads | Update (ms) | Speedup | Compute (ms) | Render (ms)"

    # Baseline (1 thread)
    base_line=$(grep "^1 " speedup_data.txt)
    base_update=$(echo "$base_line" | awk '{print $2}')

    if [ -z "$base_update" ]; then
        echo "No baseline (1 thread) entry found"
        exit 1
    fi

    # Rows
    while read t u c r; do
        sp=$(echo "scale=2; $base_update / $u" | bc)
        printf "%7s | %11.3f | %6.2fx | %12.3f | %11.3f\n" "$t" "$u" "$sp" "$c" "$r"
    done < speedup_data.txt

} | tee "$OUTFILE"

echo "Results saved to $OUTFILE"