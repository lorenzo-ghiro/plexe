#!/usr/bin/env bash

# Usage check
# for Misbehavior<platoon_size> simulations
if [ $# -ne 3 ]; then
    echo "Usage: $0 <num_cores> <platoon_size> <defense_type>"
    exit 1
fi

# Usage check
# for TrafficAround simulations
# if [ $# -ne 2 ]; then
#     echo "Usage: $0 <num_cores> <defense_type>"
#     exit 1
# fi

NUM_CORES="$1"
PLATOON_SIZE="$2" # to comment for TrafficAround simulations
DEFENSE="$3"

# Function to control parallel jobs
function wait_for_jobs {
    while [ "$(jobs -rp | wc -l)" -ge "$NUM_CORES" ]; do
        sleep 1
    done
}

echo "sim,react,misbt"
for f  in $(ls ../results/Misbehavior${PLATOON_SIZE}_${DEFENSE}_*.sca)
# for f  in $(ls ../results/TrafficAroundNoisyMisb_${DEFENSE}_*.sca)
# for f  in $(ls ../results/TrafficAroundPlatoonMisb_${DEFENSE}_*.sca)
do
	wait_for_jobs
	{
		DATA=$(opp_sca2csv.pl -F reactionTime -F timeMisbehavior "$f" | tail -n 1 | awk '{print $2 "," $3}')
		echo "${f%.sca},$DATA"
	} &
done

wait