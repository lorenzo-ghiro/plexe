import os
import sys
import pandas as pd
import numpy as np
sys.path.append(os.path.abspath("../NNtraining/preprocessing"))
import sequencer as NNsqnr

import code  # code.interact(local=dict(globals(), **locals()))

np.random.seed(1234) # for consistent extraction same subsets of tx-rx pairs

class Sequencer:
    def __init__(self, parquet_path, dataFraction):
        df = pd.read_parquet(parquet_path)
        gtpath = os.path.dirname(parquet_path)+"/merged_traceGroundTruth.parquet"
        self.gt = pd.read_parquet(gtpath)

        df = df[df.label != -1]  # Exclude unlabeled messages
        df['sender'] = df['sender'].astype(int)
        self.df = df
        self.genuine = df[df.label == 0]
        self.malicious = df[df.label != 0]

        self.seldf = self.extract_data_fraction(dataFraction)

    # def __repr__(self):
    #     return (f"Message(sendTime={self.sendTime:.2f}, x={self.posx:.2f}, y={self.posy:.2f}, "
    #     f"spdx={self.spdx:.2f}, spdy={self.spdy:.2f}, acl={self.acl:.2f}, "
    #     f"hed={self.hed:.2f}, label={self.label})")

    def extract_data_fraction(self, dataFraction):
        # select dataFraction groups out of all genuine groups and out of all malicious groups
        ug = self.genuine.sender.unique()
        selgen = np.random.choice(ug, size=int(
            len(ug)*dataFraction), replace=False)

        um = self.malicious.sender.unique()
        selmal = np.random.choice(um, size=int(
            len(um)*dataFraction), replace=False)

        selgenuine = self.genuine[self.genuine.sender.isin(selgen)]
        selmalicious = self.malicious[self.malicious.sender.isin(selmal)]
        seldf = pd.concat([selgenuine, selmalicious])

        return seldf

    def generate_sequences(self, gr):
        gr = gr.sort_values('sendTime')
        X, Y = NNsqnr.generate_sequences(gr, window_width=5, window_style="sliding", policy="last",
                                    CAMinterval=1, multiplier=100, trainingFeatures=False)
        Xn, Yn = NNsqnr.generate_sequences(gr, window_width=5, window_style="sliding", policy="last",
                                    CAMinterval=1, multiplier=100, trainingFeatures=True)

        return X, Xn, Y
