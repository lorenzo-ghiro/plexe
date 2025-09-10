import pandas as pd
import code  # code.interact(local=dict(globals(), **locals()))
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FixedLocator
import seaborn as sns
import sys

plt.rcParams['text.usetex'] = True

colors = ['whitesmoke', '#8dd3c7', '#ffffb3', '#bebada', '#fb8072', '#80b1d3', '#fdb462', '#b3de69', '#fccde5']
custom_palette = sns.color_palette(colors)



NODETECT = 12345 # aka "undected reaction time", hardcoded time in traffic/HeterogeneousControllerTrafficManager.h

FILE = sys.argv[1]

outname = FILE.replace(".csv", ".pdf")

if 'attack' in outname:
    colors = ['#b3de69', '#fccde5']
    custom_palette = sns.color_palette(colors)
else:
    colors = ['#8dd3c7', '#ffffb3', '#bebada', '#fb8072', '#80b1d3', '#fdb462', '#b3de69', '#fccde5']
    custom_palette = sns.color_palette(colors)

df = pd.read_csv(FILE)
df['mistype'] = df.sim.apply(lambda s: s.split("_")[2])

labels = {"genuine": r"$gen$",
          "constPos": r"$C_{pos}$",
          "randomPos": r"$R_{pos}$",
          "randomOffset": r"$O_{pos}$",
          "randomSpeed": r"$R_{spd}$",
          "randomOffsetSpeed": r"$O_{spd}$",
          "eventualStop": r"$EvSt$",
          "disruptive": r"$Dis$",
          "dataReplay": r"$D_{rep}$"}

if not 'attack' in outname:
    lbls = list(labels.values())[1:]
else:
    lbls = list(labels.values())[7:9]

misb2index = {"genuine": 0,
          "constPos": 1,
          "randomPos": 2,
          "randomOffset": 3,
          "randomSpeed": 4,
          "randomOffsetSpeed": 5,
          "eventualStop": 6,
          "disruptive": 7,
          "dataReplay": 8}


df["labels"] = df.mistype.apply(lambda s: labels[s])
df["indexlabel"] = df.mistype.apply(lambda s: misb2index[s])

# Keep only those with defense active (otherwise reacTime is undefined)
df = df[df.sim.str.contains("_full_")]

totrecords = len(df)
undetected = df[df.react == NODETECT]

print(f"UNDETECTED RATIO = {len(undetected) / totrecords * 100}%")

sns.catplot(data=df[df.react != NODETECT], y="react", x="indexlabel", kind="boxen", orient="v",
            height=2.7, aspect=4/3, zorder=3, palette=custom_palette)
#sns.swarmplot(data=df, y="react", x="labels", orient="v", size=2)

if not 'attack' in outname:
    plt.xticks(range(0, 8), labels=lbls, fontsize=10)
else:
    plt.xticks(range(0, 2), labels=lbls, fontsize=10)
plt.xlabel("Misbehaviors")
plt.ylabel("Reaction Time [s]")
plt.grid(which='both', axis='y', zorder=-1.0)


ax = plt.gca()
if 'attack' in outname:
    dis = df[df.sim.str.contains("disr")]
    datarep = df[df.sim.str.contains("dataRep")]

    unddis = dis[dis.react == NODETECT]
    unddr = datarep[datarep.react == NODETECT]

    dr_dis = (1 - (len(unddis) / len(dis)))*100
    dr_dr  = (1 - (len(unddr) / len(datarep)))*100
    plt.text(1/4, 1.1, f"$D_r = {dr_dr:.2f}$\\%", horizontalalignment='center',
        verticalalignment='center', transform=ax.transAxes, fontsize=10)

    plt.text(3/4, 1.1, f"$D_r = {dr_dis:.2f}$\\%", horizontalalignment='center',
        verticalalignment='center', transform=ax.transAxes, fontsize=10)

plt.subplots_adjust(left=0.16, bottom=0.18, right=0.98, top=0.9)
plt.savefig(outname, format="pdf")