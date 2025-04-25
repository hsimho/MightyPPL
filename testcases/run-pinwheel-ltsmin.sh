#!/bin/bash
FILES="
3457
3458
2_7_9_9_16 
2_7_9_10_16
3_7_8_8_9_10
3_7_8_8_9_11  
4_6_8_9_9_9_15
4_6_8_9_9_9_16
5_5_8_9_9_9_11
5_5_8_9_9_9_12  
6_7_7_7_8_8_10
6_7_7_7_8_8_11
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "ltsmin $f...(1)"
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./pinwheel/$f.xml.ltl --ltl-semantics=textbook --strategy=cndfs --threads=1 --size=24 -u 0 ./pinwheel/$f.so))
    echo "ltsmin $f...(2)"
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./pinwheel/$f.xml.ltl --ltl-semantics=textbook --strategy=cndfs --threads=2 --size=24 -u 0 ./pinwheel/$f.so))
    echo "ltsmin $f...(4)"
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./pinwheel/$f.xml.ltl --ltl-semantics=textbook --strategy=cndfs --threads=4 --size=24 -u 0 ./pinwheel/$f.so))
    echo "ltsmin $f...(8)"
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./pinwheel/$f.xml.ltl --ltl-semantics=textbook --strategy=cndfs --threads=8 --size=24 -u 0 ./pinwheel/$f.so))
    echo "ltsmin $f...(16)"
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./pinwheel/$f.xml.ltl --ltl-semantics=textbook --strategy=cndfs --threads=16 --size=24 -u 0 ./pinwheel/$f.so))
    echo "ltsmin $f...(32)"
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./pinwheel/$f.xml.ltl --ltl-semantics=textbook --strategy=cndfs --threads=32 --size=24 -u 0 ./pinwheel/$f.so))
    printf "\n"
    printf "\n"
done

