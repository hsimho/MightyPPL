#!/bin/bash
FILES="
F-12
F-45
F-910
F-1920
F-3940
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
theta-4-100_1000
theta-5-100_1000
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "size $f..."
    (timeout -s SIGKILL 300 ../build/demo ./general/$f.mitl --inf out.tck --tck) |& tail -n 9 |& head -n 5
    printf "\n"
    echo "tck flat $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/demo ./general/$f.mitl --inf out.tck --tck |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "tck flat noback $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/demo ./general/$f.mitl --inf out.tck --tck --noback |& tail -n 1)) |& tail -n 10
    printf "\n"
    printf "\n"
done

