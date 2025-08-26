import pandas as pd
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor
from glob import glob
import argparse
from new_sequencer import Sequencer
from rule_mds import Rulemds
from ai_mds import Aimds
import numpy as np

import code  # code.interact(local=dict(globals(), **locals()))


DECIDER_THRESHOLD = 0


def process_file(args):
    parquet_path, dataFraction, model_path, scaler_path, MCdropRep, MCdropCU, ARTenabled = args
    sqnr = Sequencer(parquet_path, dataFraction)

    rmds = Rulemds(ARTenabled)
    aimds = Aimds(model_path, scaler_path, MCdropRep, MCdropCU)

    mds_results = []
    groups = sqnr.seldf.groupby(['sender', 'veh'])

    # Generate all sequences of messages sent by each sender to each receiver(veh)
    # Try to evaluate all sequences with both RULEmds and AImds
    for ix, gr in tqdm(groups):
        X, Xn, Y = sqnr.generate_sequences(gr)

        mem_score = 0
        # discard if sequence completely "bad" (see sequence validation routine)
        if not X or not Xn:
            continue

        # for each 5-long window within each sequence
        for i in range(len(X)):
            x, xn, y = X[i], Xn[i], Y[i]
            msg_prev, msg = x[-2], x[-1]

            rxveh = ix[1]
            rxveh_gt = sqnr.gt[sqnr.gt.sender == rxveh]

            prev_memscore = mem_score

            # Use RULEmds
            metrics, mem_score, scores, errors = rmds.evaluate_message_pair(
                msg_prev, msg, mem_score, rxveh_gt)
            lr, cr = (None, None) if np.isnan(mem_score) else rmds.predict(metrics, scores)

            # Use also AImds
            nonbinlai, mu, CL = aimds.evaluate_sequence(xn, y)
            lai = 0 if nonbinlai == 0 else 1
            cai = mu*CL

            # CONFIDENCES SCORE FUSION
            mds_out = CSF(lr, cr, lai, cai)
            pred = 1 if mds_out > DECIDER_THRESHOLD else 0

            dicts = [{'tl': y, 'mdspl': pred, 'mdso': mds_out, 'lr': lr, 'cr': cr, 'lai': lai,
                      'cai': cai, 'muai': mu, 'CLai': CL, 'nblai': nonbinlai}, metrics, scores, errors]
            merged = {k: v for d in dicts for k, v in d.items()}

            mds_results.append(merged)

    return mds_results


def CSF(lr, cr, lai, cai):
    rterm = cr * (2 * lr - 1) if lr else 0
    aiterm = cai * (2 * lai - 1)

    return rterm + aiterm


def main(jobs, dataFraction, model_path, scaler_path, MCdropRep, MCdropCU, ARTenabled):
    files = glob("../NNtraining/Data/*/*_labeledSummary.parquet")
    out = []

    if jobs == 1:
        for f in tqdm(files):
            res = process_file(
                [f, dataFraction, model_path, scaler_path, MCdropRep, MCdropCU, ARTenabled])
            out.extend(res)
    else:
        with ProcessPoolExecutor(max_workers=jobs) as executor:
            args = [[f, dataFraction, model_path, scaler_path,
                     MCdropRep, MCdropCU, ARTenabled] for f in files]
            results = list(
                tqdm(executor.map(process_file, args), total=len(files)))
            for res in results:
                out.extend(res)

    mds_out = pd.DataFrame(out)

    artlabel = "wART" if ARTenabled else "noART"

    outname = f"mds_datafr_{int(dataFraction*1000)}_MCdropRep_{MCdropRep}_MCdropCU{int(MCdropCU*100)}_{artlabel}.parquet"
    mds_out.to_parquet(outname)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run rule-based MDS evaluation with optional parallelism.")
    parser.add_argument("-j", "--jobs", type=int, default=1,
                        help="Number of parallel jobs (default: 1 = no parallelism)")
    parser.add_argument('--dataFraction', type=float, default=1.0,
                        help='Fraction of data to evaluate (0 < f <= 1.0)')
    parser.add_argument('--model', type=str, required=True,
                        help='Path to trained Keras model (.keras)')
    parser.add_argument('--scaler', type=str, required=True,
                        help='Path to feature scaler (.pkl)')
    parser.add_argument('--MCdropRep', type=int, default=10,
                        help='MC Dropout forward passes')
    parser.add_argument('--MCdropCU', type=float, default=0.10,
                        help='Confidence Uncertainty as relative half-width (e.g., 0.10 = ±10%%)')
    parser.add_argument('--art', action="store_true", help='enable art score to be computed by RULEmds not otherwise')
    args = parser.parse_args()
    main(args.jobs, args.dataFraction, args.model,
         args.scaler, args.MCdropRep, args.MCdropCU, args.art)
