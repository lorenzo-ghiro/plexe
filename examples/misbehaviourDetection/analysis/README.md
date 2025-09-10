## Runtime Evaluation with PLEXE

Then, the Hybrid MDS is evaluated in run-time simulations:

```console
cd $PLEXE_DIR/examples/misbehaviourDetection
./run -u Cmdenv -c <CONFIGURATION> -r <SIMULATION_RUN_NUMBER>
```

Where ```<CONFIGURATION>``` can be:
* ```Misbehavior4``` - platoon in isolation;
* ```TrafficAround``` - platoon surrounded by background traffic, where the misbehaving vehicle belongs to the background traffic;
* ```TrafficAroundPlatoonMisb``` - platoon with background traffic, where the misbehaving vehicle belongs to the platoon.

The **Main Parameters** that characterize vehicles and communications in the simulation experiments are the following:

|                                              | Parameter                       | Value                                              |
|----------------------------------------------|---------------------------------|----------------------------------------------------|
| Scenario                                     | Road Type                       | 3-Lane Highway                                     |
|                                              | Duration                        | 120 s                                              |
|                                              | Misb. Start Time                | Uniform[15,30]s                                    |
|                                              | Default CACC                    | PLOEG                                              |
|                                              | PLOEG Headway                   | 0.5 s                                              |
|                                              | Autonomous Controller           | ACC                                                |
|                                              | ACC Headway                     | 1.2 s                                              |
|                                              | Beaconing Frequency             | 10 Hz                                              |
|                                              | Platoon Size                    | 4                                                  |
|                                              | Leader Speed                    | 100 km/h                                           |
|                                              | Speed Oscillation Amplitude & Freq | 5 km/h, 0.1 Hz                                  |
|                                              | Background vehicles when present | 9                                                 |
|                                              | Repetitions per Experiment      | 100                                                |
| Communication                                | L2-technology                   | dual radio 802.11p                                 |
|                                              | Tx power                        | 100 mW                                             |
|                                              | Broadcast MCS                   | 3 Mbit/s                                           |
|                                              | Unicast MCS                     | 12 Mbit/s                                          |
|                                              | Rx sensitivity                  | -94 dBm                                            |

Once run all the simulations of a specific configuration, to evaluate the run-time performances of the Hybrid MDS, the collision rate and the reaction time are computed.

In the ```$PLEXE_DIR/examples/misbehaviourDetection/analysis``` folder run the following script:

```console
findCrashedRuns.py --glob "../results/*collision.xml" -w
```
This will generate, in the current folder, the ```cr_pl<PLATOON_SIZE>_<CONFIGURATION>.csv``` file, which is passed as argument to the script to compute the collision rate:

```console
python compareCollisionRate.py cr_pl<PLATOON_SIZE>_<CONFIGURATION>.csv
```

Then, always in the ```$PLEXE_DIR/examples/misbehaviourDetection/analysis``` folder run the following bash script:

```console
./extractReactionTimes.sh $(nproc) <PLATOON_SIZE> <DEFENSE_TYPE> > ../results/rt_pl<PLATOON_SIZE>_<CONFIGURATION>.csv
```

This will generate, in the ```$PLEXE_DIR/examples/misbehaviourDetection/results``` folder, the ```rt_pl<PLATOON_SIZE>_<CONFIGURATION>.csv``` file, which is passed as argument to the script to compute the reaction time:

```console
python reacTimes.py rt_pl<PLATOON_SIZE>_<CONFIGURATION>.csv
```
