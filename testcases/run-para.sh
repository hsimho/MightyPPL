#!/bin/bash
FILES="
F-4-2i
G-4-2i
U-4-02
U-4-2i
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "size $f..."
    (timeout -s SIGKILL 300 ../build/mitppl ./parametric/$f.mitl --fin out.tck --tck) |& tail -n 9 |& head -n 5
    printf "\n"
    echo "tck comp $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/mitppl ./parametric/$f.mitl --fin out.tck --tck --noflatten |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "tck flat $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/mitppl ./parametric/$f.mitl --fin out.tck --tck |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "fp flat $f..."
    (time timeout -s SIGKILL 300 ../build/mitppl ./parametric/$f.mitl --fin) |& tail -n 8
    printf "\n"
    printf "\n"
done

