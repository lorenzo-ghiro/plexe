import os
import glob
import pandas as pd
import sys
import re
import code #code.interact(local=dict(globals(), **locals()))

# traceJSON-9999-9997-A0-28466-7.parquet
# traceJSON_vehindex_oppid_attackerflag_timestamp_timeWindow.parquet|json
logregex = re.compile(r"traceJSON-(\d+)-(\d+)-A(\d+)-(\d+)-(\d+)\.(?:json|parquet)")

def merge_parquets(file_list, add_veh_column=False):
    dfs = []
    for idx, file in enumerate(sorted(file_list)):
        try:
            df = pd.read_parquet(file)
            if add_veh_column:
                vehindex, oppid, attackerflag, timestamp = map(int,logregex.search(file).groups()[:4])
                df['veh'] = vehindex
            dfs.append(df)
        except Exception as e:
            print(f"Error while reading {file}: {e}")
    return pd.concat(dfs, ignore_index=True) if dfs else pd.DataFrame()


def main(attack_dir):
    json_files = glob.glob(os.path.join(attack_dir, "VeReMi*/traceJSON*.parquet"))
    gt_files = glob.glob(os.path.join(attack_dir, "VeReMi*/traceGroundTruth*.parquet"))

    if not json_files:
        print(f"No traceJSON files found in {attack_dir}")
    if not gt_files:
        print(f"No traceGroundTruth files found in {attack_dir}")

    json_merged = merge_parquets(json_files, add_veh_column=True)
    gt_merged = merge_parquets(gt_files)

    if not json_merged.empty:
        json_out = os.path.join(attack_dir, "merged_traceJSON.parquet")
        json_merged.to_parquet(json_out)
        print(f"Saved: {json_out}")

    if not gt_merged.empty:
        gt_out = os.path.join(attack_dir, "merged_traceGroundTruth.parquet")
        gt_merged.to_parquet(gt_out)
        print(f"Saved: {gt_out}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 mergeparquet.py <attack_directory>")
        sys.exit(1)
    attack_directory = sys.argv[1]
    main(attack_directory)
