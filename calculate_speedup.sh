#!/bin/bash
# Calculate speedup from benchmark data

if [ ! -f speedup_data.txt ]; then
    echo "Error: speedup_data.txt not found"
    exit 1
fi

OUTPUT="speedup.txt"

# Write to both console and file
{
    echo "=== SPEEDUP SUMMARY ==="
    echo "Threads | Update (ms) | Speedup | Compute (ms) | Render (ms)"
    echo "--------|-------------|---------|--------------|------------"

    # Read baseline (1 thread) times
    baseline_data=$(grep "^1 " speedup_data.txt)
    baseline_update=$(echo $baseline_data | awk '{print $2}')
    baseline_compute=$(echo $baseline_data | awk '{print $3}')
    baseline_render=$(echo $baseline_data | awk '{print $4}')

    if [ -z "$baseline_update" ]; then
        echo "Error: No baseline data found"
        exit 1
    fi

    # Calculate speedup for each thread count
    while read threads update compute render; do
        speedup=$(echo "scale=2; $baseline_update / $update" | bc)
        printf "%7s | %11.3f | %7.2fx | %12.3f | %11.3f\n" "$threads" "$update" "$speedup" "$compute" "$render"
    done < speedup_data.txt
} | tee "$OUTPUT"

echo ""
echo "Results saved to $OUTPUT"