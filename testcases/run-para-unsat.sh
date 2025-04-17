#!/bin/bash
FILES="
X-2-02
X-2-2i
p-X-2-02
p-X-2-2i
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "size $f..."
    (timeout -s SIGKILL 300 ../build/demo ./parametric/unsat/$f.mitl --fin out.tck --tck) |& tail -n 9 |& head -n 5
    printf "\n"
    echo "tck comp $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/demo ./parametric/unsat/$f.mitl --fin out.tck --tck --noflatten |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "tck flat $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/demo ./parametric/unsat/$f.mitl --fin out.tck --tck |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "fp flat $f..."
    (time timeout -s SIGKILL 300 ../build/demo ./parametric/unsat/$f.mitl --fin) |& tail -n 8
    printf "\n"
    printf "\n"
done
