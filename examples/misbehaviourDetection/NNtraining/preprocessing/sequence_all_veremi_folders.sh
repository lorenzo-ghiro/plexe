#!/bin/bash

# Usage check
if [ $# -ne 5 ]; then
    echo "Usage: $0 <top_level_directory> <window_width> <window_style> <policy> <num_cores>"
    exit 1
fi

TOP_DIR="$1"
WINDOW_WIDTH="$2"
WINDOW_STYLE="$3"
POLICY="$4"
NUM_CORES="$5"

# Validate window width
if ! [[ "$WINDOW_WIDTH" =~ ^[0-9]+$ ]] || [ "$WINDOW_WIDTH" -lt 1 ] || [ "$WINDOW_WIDTH" -gt 100 ]; then
    echo "Error: Window width must be an integer between 1 and 100."
    exit 2
fi

# Validate window style
if [[ "$WINDOW_STYLE" != "sliding" && "$POLICY" != "jumping" ]]; then
    echo "Error: WINDOW_STYLE must be one of: sliding, jumping"
    exit 3
fi

# Validate policy
if [[ "$POLICY" != "last" && "$POLICY" != "first" && "$POLICY" != "mostFrequent" ]]; then
    echo "Error: Policy must be one of: last, first, mostFrequent"
    exit 4
fi

# Validate num cores
if ! [[ "$NUM_CORES" =~ ^[0-9]+$ ]] || [ "$NUM_CORES" -lt 1 ]; then
    echo "Error: Number of cores must be a positive integer."
    exit 5
fi

# Function to control parallel jobs
function wait_for_jobs {
    while [ "$(jobs -rp | wc -l)" -ge "$NUM_CORES" ]; do
        sleep 1
    done
}

# Find and process each file
find "$TOP_DIR" -type f -name "*_labeledSummary.parquet" | while read -r FILEPATH; do
    wait_for_jobs
    echo "Processing: $FILEPATH"
    python3 sequencer.py "$FILEPATH" "$WINDOW_WIDTH" "$WINDOW_STYLE" "$POLICY" &
done

# Wait for remaining background jobs to finish
wait
