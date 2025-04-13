#!/bin/bash
FILES="
F-2-02
F-2-2i
F-3-02
F-3-2i
F-4-02
F-4-2i
G-2-02
G-2-2i
G-3-02
G-3-2i
G-4-02
G-4-2i
R-2-02
R-2-2i
R-3-02
R-3-2i
R-4-02
R-4-2i
U-2-02
U-2-2i
U-3-02
U-3-2i
U-4-02
U-4-2i
"
for f in $FILES
do
    printf "\n"
    echo "components $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_tck/build/demo ./parametric/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach tck $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./parametric/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach nfp $f..."
    (time timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./parametric/$f.mitl) |& tail -n 8
    printf "\n"
done

