import argparse
import pandas as pd
from tqdm import tqdm
import numpy as np
import math
import code  # code.interact(local=dict(globals(), **locals()))
import os

selected_training_features = ['sendTime',
                              'posx', 'posy', 'spdx', 'spdy', 'acl']
selected_other_features =  ['sendTime', 'posx', 'posy', 'spdx', 'spdy', 'acl', 'hed', 'label']


def normalize_sequence(sequence_df):
    """
    Apply a transformation where the first row of the sequence is used
    as a reference and subtracted from all rows in the sequence.
    Then apply the labeling policy.
    """
    # Subtract the first row from all rows, column-wise
    ref_row = sequence_df.iloc[0]
    # "Progressive kinematic differences"
    normalized_sequence = sequence_df - ref_row
    return normalized_sequence

def label_sequence(sequence_df, policy, trainingFeatures):
    chosenLabel = None
    if policy == "first":
        chosenLabel = sequence_df['label'].iloc[0]
    elif policy == "last":
        chosenLabel = sequence_df['label'].iloc[-1]
    elif policy == "mostFrequent":
        # Take first if multiple modes
        chosenLabel = sequence_df['label'].mode().iloc[0]
    else:
        raise ValueError(f"Unknown labeling policy: {policy}")

    return chosenLabel
    


def validate_sequence(seq, window_width, CAMinterval=1, multiplier=2):
    if len(seq) < 2:
        raise ValueError("Sequence must contain at least 2 messages.")

    start_time = seq.iloc[0]['sendTime']
    end_time = seq.iloc[-1]['sendTime']

    max_allowed_span = multiplier * window_width * CAMinterval

    if (end_time - start_time) > max_allowed_span:
        raise ValueError(f"Invalid sequence: duration {end_time - start_time:.3f}s exceeds limit of {max_allowed_span:.3f}s")


def generate_sequences(df, window_width, window_style, policy, CAMinterval, multiplier, trainingFeatures):
    """
    Generate all sequences of consecutive rows of length 'window_width',
    label each sequence, and return a list of (sequence_df, label) tuples.
    """
    X, Y = [], []
    if len(df) < window_width:
        return None, None

    total = len(df) - window_width + 1
    df = df.sort_values(by='sendTime')
    if window_style == "jumping":
        step = window_width
    elif window_style == "sliding":
        step = 1
    else:
        exit(f"Unsopported window_style = {window_style}")
    for start in range(0, total, step):
        sequence = df.iloc[start:start + window_width]
        try:
            validate_sequence(sequence, window_width, CAMinterval, multiplier)
        except ValueError:
            continue

        if (trainingFeatures):
            sequence = normalize_sequence(sequence)

        y = label_sequence(sequence, policy, trainingFeatures)

        if trainingFeatures:
            x = sequence[1:len(sequence)][selected_training_features].values
        else:
            # dont normalize with ref_row when generating sequences not for training purposes (e.g., for testing RuleMDS)
            x = sequence[selected_other_features].values
        X.append(x)
        Y.append(y)
    return X, Y


def create_dataset(df, window_width, window_style, policy, CAMinterval, multiplier):
    genuine = df[df.label==0]
    malicious = df[df.label!=0]
    max_num_malicious_slices = math.floor(len(malicious) / window_width)
    max_num_genuine_slices = math.floor(len(genuine) / window_width)

    # Group malicious pairs by Tx,Rx, then create maximum number of valid sequences
    groups = malicious.groupby(['senderPseudo', 'veh'])
    pbar = tqdm(total=max_num_malicious_slices, desc="Creating malicious sequences...")
    Xmal, Ymal = [], []
    for _, gr in groups:
        x, y = generate_sequences(gr, window_width, window_style, policy, CAMinterval, multiplier, trainingFeatures=True)
        if not y and not x:
            continue #dont save invalid/incomplete sequences
        Xmal += x
        Ymal += y
        pbar.update(len(y))  # manually increment by 1
    pbar.close()
    
   
    print(f"{len(Ymal)} malicious slices created!")
    print("Now generate genuine slices...")

    # Group genuine pairs by Tx,Rx, then create requested number of sequences
    groups = genuine.groupby(['senderPseudo', 'veh'])
    pbar = tqdm(total=max_num_genuine_slices, desc="Creating genuine sequences...")
    Xgen, Ygen = [], []
    for _, gr in groups:
        x, y = generate_sequences(gr, window_width, window_style, policy, CAMinterval, multiplier, trainingFeatures=True)
        if not y and not x:
            continue #dont save invalid/incomplete sequences
        Xgen += x
        Ygen += y
        pbar.update(len(y))  # manually increment by 1
    pbar.close()

    print(f"{len(Ygen)} genuine slices created!")
    
    X = Xmal+Xgen
    Y = Ymal+Ygen
    assert len(X) == len(Y)
    print(f"{len(Ygen) / len(Y)} = genRatio, {len(Ymal) / len(Y)} = malRatio")
    return X, Y
    

def main(parquet_path, window_width, window_style, policy, CAMinterval, multiplier):
    # Load the dataframe only necessary columns
    columns = ['sendTime', 'senderPseudo', 'posx',
               'posy', 'spdx', 'spdy', 'acl', 'label', 'veh']
    df = pd.read_parquet(parquet_path, columns=columns)
    # Exclude unlabeled messages
    df = df[df.label != -1]

    # Creating the dataset (maximum number of sequences)
    X, Y = create_dataset(df, window_width, window_style, policy, CAMinterval, multiplier)

    print("Reshaping x sequences for storing as parquet...")
    flattenedX = [m.reshape(-1) for m in X]
    # to restore as matrix later do
    # restored_matrices = [row.values.reshape(4, 6) for _, row in dfx.iterrows()]

    # Output X,Y training data as parquet
    print("Saving x sequences and y labels")
    dfx = pd.DataFrame(flattenedX)

    dirpath = os.path.dirname(parquet_path)
    commonname = f"sequences_ww_{window_width}_ws_{window_style}_policy_{policy}.parquet"
    xseqpath = os.path.join(dirpath, f"x{commonname}") 
    dfx.to_parquet(xseqpath)

    yseqpath = os.path.join(dirpath, f"y{commonname}") 
    dfy = pd.DataFrame(Y)
    dfy.to_parquet(yseqpath)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Process sequences from parquet file.")
    parser.add_argument("parquet_file", type=str,
                        help="Path to input .parquet file")
    parser.add_argument("window_width", type=int, choices=range(1, 101),
                        help="Sliding window width (integer from 1 to 100)")
    parser.add_argument("window_style", type=str, choices=["jumping", "sliding"],
                        help="window_style: 'jumping', or 'sliding'")
    parser.add_argument("labeling_policy", type=str,
                        choices=["first", "last", "mostFrequent"],
                        help="Labeling policy: 'first', 'last', or 'mostFrequent'")
    parser.add_argument("--CAMinterval", type=float, default=1,
                        help="Sending time between CAM messages")
    parser.add_argument("--multiplier", type=float, default=3,
                        help="""Multiplier applied to window width and CAM interval to validate sequences.
                        Sequences longer than (time_multiplier * window_width * cam_interval) seconds will be discarded.
                        Default is 3.0.""")

    args = parser.parse_args()
    main(args.parquet_file, args.window_width, args.window_style, args.labeling_policy, args.CAMinterval, args.multiplier)
