#!/bin/bash
FILES="
p-F-2-02
p-F-2-2i
p-F-3-02
p-F-3-2i
p-F-4-02
p-F-4-2i
p-G-2-02
p-G-2-2i
p-G-3-02
p-G-3-2i
p-G-4-02
p-G-4-2i
p-R-2-02
p-R-2-2i
p-R-3-02
p-R-3-2i
p-R-4-02
p-R-4-2i
p-U-2-02
p-U-2-2i
p-U-3-02
p-U-3-2i
p-U-4-02
p-U-4-2i
"
for f in $FILES
do
    printf "\n"
    echo "components $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_tck/build/demo ./parametric/past/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach tck $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./parametric/past/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach nfp $f..."
    (time timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./parametric/past/$f.mitl) |& tail -n 8
    printf "\n"
done

