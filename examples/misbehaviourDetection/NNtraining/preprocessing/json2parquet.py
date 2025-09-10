import sys
import pandas as pd
from pathlib import Path
import code #code.interact(local=dict(globals(), **locals()))
import math

columns_keep = ['sendTime', 'sender', 'senderPseudo', 'posx', 'posy', 'spdx', 'spdy', 'acl', 'hed', 'messageID']

def euclidean_norm(x, y):
    return math.sqrt(math.pow(x,2) + math.pow(y,2))

def calculate_horizontal_angle(x, y):
    angle = math.degrees(math.atan2(y, x))
    angle = angle % 360
    return angle

def process_dataframe(df):
    df['posx'] = df['pos'].apply(lambda x: x[0])
    df['posy'] = df['pos'].apply(lambda x: x[1])

    df['spdx'] = df['spd'].apply(lambda x: x[0])
    df['spdy'] = df['spd'].apply(lambda x: x[1])

    df['hed'] = df['hed'].apply(lambda x: calculate_horizontal_angle(x[0], x[1]))

    df['acl_angle'] = df['acl'].apply(lambda x: calculate_horizontal_angle(x[0], x[1]))
    df['acl'] = df['acl'].apply(lambda x: euclidean_norm(x[0], x[1]))
    df['acl'] = df.apply(
        lambda row: -row['acl'] if abs(row['hed'] - row['acl_angle']) > 90 else row['acl'], axis=1)
    return df[columns_keep]

def json_to_parquet(file_path):
    try:
        file_path = Path(file_path)
        if not file_path.is_file():
            print(f"File not found: {file_path}")
            return

        # Read the JSON file
        isGroundTruth = "traceGroundTruth" in file_path._str
        df = pd.read_json(file_path, lines=True)
        if not isGroundTruth:
            df = df[df.type==3].copy() # Keep only BSM (type==3), discard all others (e.g. GPS data type==2)

        df = process_dataframe(df)
        output_path = file_path.with_suffix('.parquet')
        df.to_parquet(output_path, index=False)

        print(f"Converted: {file_path} -> {output_path}")
    except Exception as e:
        print(f"Error processing {file_path}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python json2parquet.py <file_path>")
        sys.exit(1)

    json_to_parquet(sys.argv[1])