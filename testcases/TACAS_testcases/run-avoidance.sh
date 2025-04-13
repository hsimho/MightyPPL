#!/bin/bash
FILES="
1
2
3
4prime
"
for f in $FILES
do
    printf "\n"
    echo "components $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_tck/build/demo ./avoidance/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach tck $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./avoidance/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach nfp $f..."
    (time timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./avoidance/$f.mitl) |& tail -n 8
    printf "\n"
done

