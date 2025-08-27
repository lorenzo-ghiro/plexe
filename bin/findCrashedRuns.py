#!/usr/bin/env python3

from glob import glob
import code  # code.interact(local=dict(globals(), **locals()))
import argparse
import os
import pandas as pd
import re
from tqdm import tqdm
import xml.etree.ElementTree as ET
from tabulate import tabulate

parser = argparse.ArgumentParser(description='''Explore given folders try to understand 
    which runs generated some collision''')
parser.add_argument('--glob', '-g', required=True,
                    help='glob pointing to *collision.xml files')
parser.add_argument('--writecsv', '-w', default=False, action='store_true',
                    help='Activate this option if you want to save the csv of crashed simulations')
args = parser.parse_args()

#filePattern = re.compile("platoform_(\d+)_(\d+)_([-+]?(?:\d*\.*\d+))_(\d+)_collision.xml")
#re.compile("platoform_(\d+)_(\d+)_([-+]?(?:\d*\.*\d+))_(\d+).csv")

print(f"Analysing results available in {args.glob}...")
files = glob(args.glob)

if not files:
    exit("no file matching glob find")
    
wrongs = []
for f in tqdm(files):
    #print(f)
    xml = None
    try:
        xml = ET.parse(f)
    except Exception as e:
        if (str(e) == "unclosed token: line 3, column 0"):
            continue  # means that simulation is still running
        print(e)
        continue

    if not xml.getroot().tag == 'collisions':
        raise Exception(f"WTF with {f}")

    collisions = list(xml.getroot())
    if not collisions:
        # No collisions detected, all right!
        continue
    else:
        '''
        form, vSstartPos, v1startPos, v2startPos = filePattern.search(f).groups()
        vSstartPos, v1startPos, v2startPos = int(vSstartPos), int(v1startPos), int(v2startPos)
        if (v1startPos >= vSstartPos):
            continue
        if (vSstartPos - v1startPos <= 6):
            continue
        '''
        facts = collisions[0].attrib
        crashTime, v1, v2 = float(
            facts['time']), facts['collider'], facts['victim']
        lane = int(facts['lane'].split('_')[-1])
        # if lane != 1:
        #    print("Wow, not a collision bug over middle lane!!!\nCheck " + f)
        wrongs.append([f, crashTime, v1.replace(
            "vtypeauto.", ""), v2.replace("vtypeauto.", ""), lane])

#code.interact(local=dict(globals(), **locals()))
df = pd.DataFrame(wrongs, columns=['file', 'crashTime', 'collider', 'victim', 'lane'])
df = df.sort_values(['crashTime'])
if (args.writecsv):
    df.to_csv("crashed.csv", index=False)
print(tabulate(df, showindex=False, headers='keys', tablefmt='simple'))
#print(df.to_csv(index=False, sep='\t'))
