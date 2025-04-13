#!/bin/bash
FILES="
1
2
3
4
5
1p
2p
3p
4p
5p
"
for f in $FILES
do
    printf "\n"
    echo "components $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_tck/build/demo ./hoxha/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach tck $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./hoxha/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach nfp $f..."
    (time timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./hoxha/$f.mitl) |& tail -n 8
    printf "\n"
done

