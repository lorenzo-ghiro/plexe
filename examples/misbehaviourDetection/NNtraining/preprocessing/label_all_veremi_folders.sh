#!/bin/bash

# Check at least one argument is given
if [ $# -lt 1 ]; then
    echo "Usage: $0 <top_level_directory> [num_cores]"
    exit 1
fi

TOP_DIR="$1"
NUM_CORES="${2:-$(nproc)}"  # Use second argument or default to number of CPU cores


# Check if directory exists
if [ ! -d "$TOP_DIR" ]; then
    echo "Error: '$TOP_DIR' is not a valid directory."
    exit 2
fi

find "$TOP_DIR" -mindepth 1 -maxdepth 1 -type d | while read -r ATTACK_DIR; do
    JSON_FILE="${ATTACK_DIR}/merged_traceJSON.parquet"
    GT_FILE="${ATTACK_DIR}/merged_traceGroundTruth.parquet"
    ATTACK_NAME=$(basename "$ATTACK_DIR")

    if [[ -f "$JSON_FILE" && -f "$GT_FILE" ]]; then
        echo "$JSON_FILE|$GT_FILE|$ATTACK_NAME"
    else
        echo "SKIP|$ATTACK_NAME"
    fi
done | grep -v '^SKIP' | xargs -P "$NUM_CORES" -I{} bash -c '
    IFS="|" read -r JSON GT ATTACK <<< "{}"
    echo "Labeling $ATTACK"
    python3 label_messages.py "$JSON" "$GT" "$ATTACK"
'
