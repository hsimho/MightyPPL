#!/bin/bash
FILES="
X-2-02
X-2-2i
X-3-02
X-3-2i
X-4-02
X-4-2i
p-X-2-02
p-X-2-2i
p-X-3-02
p-X-3-2i
p-X-4-02
p-X-4-2i
"
for f in $FILES
do
    printf "\n"
    echo "components $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_tck/build/demo ./parametric/unsat/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach tck $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./parametric/unsat/$f.mitl out.tck |& tail -n 1)) |& tail -n 10
    echo "reach nfp $f..."
    (time timeout -s SIGKILL 300 ../MightyPPL_reach/build/demo ./parametric/unsat/$f.mitl) |& tail -n 8
    printf "\n"
done

