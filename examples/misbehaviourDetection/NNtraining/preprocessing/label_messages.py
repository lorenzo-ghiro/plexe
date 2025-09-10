import os
import argparse
import pandas as pd
import numpy as np
import code #code.interact(local=dict(globals(), **locals()))
from tqdm import tqdm
import re


cols2check = ['sender', 'senderPseudo', 'posx', 'posy', 'spdx', 'spdy', 'acl', 'hed']

attack_label_map = {
    "ConstPos": 1,
    "RandomPos": 2,
    "PosOffset": 3,
    "RandomSpeed": 4,
    "SpeedOffset": 5,
    "EventualStop": 6,
    "Disruptive": 7,
    "DataReplay": 8,
}


def label_row(row, gtdf, labelnum):
    gtr = gtdf[gtdf.messageID==row.messageID]
    if (np.isnan(row.messageID)):
        return -404
    elif len(gtr) < 1:
        # NO RECORD IN THE GROUND_TRUTH
        # print(f"NO RECORD IN THE GROUND_TRUTH for msgID = {messageID}")
        return -101 #-101 aka Unknown messageID
    elif len(gtr) > 1:
        # TOO MANY RECORDS IN THE GROUND_TRUTH... retransmission
        # exit("RETRANSMISSIONs should not happen")
        return -505
    gtr = gtr.iloc[0]
    # Specific condition for 'sendTime' where the absolute difference must be greater than 0.01
    if abs(row.sendTime - gtr.sendTime) > 0.01:
        return labelnum
    # General condition for all other columns where any difference sets the label to 1
    for col in cols2check:
        if row[col] != gtr[col]:
            return labelnum
    return 0 # all right, 0 means "genuine"


def main(json_path, gt_path, attack_name):
    labelnum = attack_label_map.get(attack_name)
    if labelnum is None:
        raise ValueError(f"Unknown attack name: {attack_name}")

    df = pd.read_parquet(json_path)
    gtdf = pd.read_parquet(gt_path)

    tqdm.pandas()
    df['label_reason'] = df.progress_apply(lambda row: label_row(row, gtdf, labelnum), axis=1)
    df['label'] = df['label_reason'].apply(lambda x: 0 if x == 0 else labelnum)

    output_path = os.path.join(os.path.dirname(json_path), f"{attack_name}_labeledSummary.parquet")
    df.to_parquet(output_path)

    print(f"Processing complete. Output written to {output_path}")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Label log messages using ground truth.')
    parser.add_argument('json_path', type=str, help='Path to the merged traceJSON.parquet file')
    parser.add_argument('gt_path', type=str, help='Path to the merged traceGroundTruthJSON.parquet file')
    parser.add_argument('attack_name', type=str, help='Attack name')
    args = parser.parse_args()

    main(args.json_path, args.gt_path, args.attack_name)
