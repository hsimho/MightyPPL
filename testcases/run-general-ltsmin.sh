#!/bin/bash
FILES="
F-12
F-45
F-2-12
F-3-12
G-3-12
G-4-12
U-3-12
U-4-12
R-3-12
R-4-12
R-5-12
X-1-12
X-2-12
theta-3-100_1000
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "ltsmin $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 opaal2lts-mc --ltl=./general/$f.ltl --ltl-semantics=textbook --strategy=cndfs --threads=1 --size=24 -u 0 ./general/$f.so))
    printf "\n"
    printf "\n"
done

