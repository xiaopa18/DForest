# dataid='audio'
# queryid='uniform1000'
# e=42516.318408
# blockdim=1
# datatrstime=267
# querytrstime=0.0008
# page=32768
# rou=2239.98
# rs=(76067	86026	97792	111782	129327)
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

# dataid='deep1M'
# queryid='uniform1000'
# e=0.336915
# blockdim=1
# datatrstime=14052
# querytrstime=0.0013
# page=131072
# rs=(0.906499	0.960435	1.018052	1.080718	1.15147)
# rou=0.00323721
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

# dataid='enron'
# queryid='uniform1000'
# e=30.428929
# blockdim=1
# datatrstime=8550
# querytrstime=0.0102
# page=32768
# rs=(66	71	80	94	125)
# rou=0.169469
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

# dataid='notre'
# queryid='uniform1000'
# e=137.647246
# blockdim=1
# datatrstime=2552
# querytrstime=0.0006
# rs=(338	358	381.728206	409.686012	446.324971)
# rou=1.96629
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

# dataid='nuswide'
# queryid='uniform1000'
# e=17.446457
# blockdim=1
# datatrstime=569
# querytrstime=0.003636
# rs=(28.9165	30.2023	31.759363	33.830307	37.051409)
# rou=0.0523061
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

# dataid='sun'
# queryid='uniform1000'
# e=45610.814237
# blockdim=1
# page=32768
# datatrstime=1463.449478
# querytrstime=0.004234075546
# rs=(101897	108819	116851	126805	140587)
# rou=740.66
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

# dataid='trevi'
# queryid='uniform1000'
# e=527.648227
# blockdim=1
# datatrstime=75945.325
# querytrstime=0.07018661499023438
# page=32768
# rs=(2175	2380	2631	2965	3473)
# rou=2.41071
# rss="${#rs[@]} ${rs[@]}"
# ./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

dataid='sift10M'
queryid='uniform1000'
e=154.849558
blockdim=1
datatrstime=114323.0598
querytrstime=0.0006463527679443359
page=131072
rs=(269)
rou=2.14763
rss="${#rs[@]} ${rs[@]}"
./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou

dataid='glove'
queryid='uniform1000'
e=2.514118
blockdim=1
datatrstime=81700
querytrstime=0.007252931594848633
rs=(6.039538)
page=32768
rou=0.0104507
rss="${#rs[@]} ${rs[@]}"
./main_storage ${dataid} ${queryid} ${e} ${blockdim} ${page} ${datatrstime} ${querytrstime} ${rss} $rou