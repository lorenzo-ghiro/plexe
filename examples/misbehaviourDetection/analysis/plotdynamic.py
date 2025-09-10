import pandas as pd
import code #code.interact(local=dict(globals(), **locals()))
import matplotlib.pyplot as plt
import sys
import matplotlib.lines as mlines

plt.rcParams['text.usetex'] = True
plt.rcParams['axes.grid'] = True

time = 40.0
df = pd.read_parquet(sys.argv[1])
df = df.sort_values(by=['time', 'nodeId'])


# Plot distance on axs[0], speed on axs[1]
fig, axs = plt.subplots(2, figsize=(5, 4.2), sharex=True)

node_ids = df['nodeId'].unique()
colors = ['#66c2a5','#fc8d62','#8da0cb']
#colors = ['k','b','r']

leglines = []
for i, node_id in enumerate(node_ids):
    if node_id != 0:
        subset = df[df['nodeId'] == node_id]
        legl, = axs[0].plot(subset['time'], subset['distance'],
            label=f'Vehicle {node_id}',
            color=colors[i-1],
            linestyle='-',
            linewidth=2)
        axs[1].plot(subset['time'], subset['speed'],
            label=f'Vehicle {node_id}',
            color=colors[i-1],
            linestyle='-',
            linewidth=2)
        leglines.append(legl)

axs[1].set_xlabel('Time [s]', labelpad=20)
axs[1].set_xlim(0,90)

axs[0].set_ylabel('Front Distance [m]')
axs[1].set_ylabel('Speed [m/s]')



# Sections
time_marker = time

axs[0].axvline(x=time_marker, color='r', linestyle='--', zorder=10)
axs[1].axvline(x=time_marker, color='r', linestyle='--', zorder=10)
axs[0].text(time_marker-1.1, 20, 'Misbehavior', color='red', fontsize=10,
rotation=90, verticalalignment='bottom', horizontalalignment='center')
axs[1].text(time_marker-1.1, 24, 'Misbehavior', color='red', fontsize=10,
rotation=90, verticalalignment='bottom', horizontalalignment='center')

axs[0].axvline(x=64, color='k', linestyle='--', zorder=10)
axs[1].axvline(x=64, color='k', linestyle='--', zorder=10)
axs[0].text(64-1.1, 15, 'End Emer. Prot.', color='k', fontsize=10,
rotation=90, verticalalignment='bottom', horizontalalignment='center')

axs[1].text(64-1.1, 23.6, 'End Emer. Prot.', color='k', fontsize=10,
rotation=90, verticalalignment='bottom', horizontalalignment='center')


noteStyle = {'xytext': (0, -26), 'textcoords': 'offset points', 'ha':'center',
    'va':'top', 'fontsize':9, 'fontstyle':'italic', 'annotation_clip': False}
# Place the label below the x-axis using `annotate`
axs[1].annotate("CACC", xy=(20, 24), **noteStyle)
axs[1].annotate("Gap Control", xy=(54, 24), **noteStyle)

axs[1].annotate("ACC", xy=(78, 24), **noteStyle)

plt.figlegend(handles=leglines, bbox_to_anchor=(0.9, 1.01), ncol=3)
plt.xlim(1,90)
plt.subplots_adjust(left=0.1, bottom=0.2, right=0.95, top=0.92, hspace=0.1)
plt.savefig('evolutionProtocol.pdf')
