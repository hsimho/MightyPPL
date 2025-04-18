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
    printf "\n"
    echo "size $f..."
    (timeout -s SIGKILL 300 ../build/mitppl ./hoxha/$f.mitl --inf out.tck --tck) |& tail -n 9 |& head -n 5
    printf "\n"
    echo "tck comp $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/mitppl ./hoxha/$f.mitl --inf out.tck --tck --noflatten |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "tck flat $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/mitppl ./hoxha/$f.mitl --inf out.tck --tck |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "fp flat $f..."
    (time timeout -s SIGKILL 300 ../build/mitppl ./hoxha/$f.mitl --inf) |& tail -n 8
    printf "\n"
    printf "\n"
done

