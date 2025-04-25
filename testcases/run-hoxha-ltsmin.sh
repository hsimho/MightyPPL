#!/bin/bash
FILES="
1
2
3
4
5
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "ltsmin $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./hoxha/$f.ltl --ltl-semantics=textbook --strategy=cndfs --threads=1 --size=24 -u 0 ./hoxha/$f.so))
    printf "\n"
    printf "\n"
done

