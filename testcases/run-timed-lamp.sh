#!/bin/bash
FILES="
1
2
3
4
"

echo "Compiling the version of MightyPPL with model..."
cp -f ../MightyPPL_lamp.cpp ../MightyPPL.cpp
cd ../build && make
cd ../testcases
printf "\n"

sleep 2

for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "size $f..."
    (timeout -s SIGKILL 300 ../build/mitppl ./timedlamp/$f.mitl --inf out.tck --tck) |& tail -n 9 |& head -n 5
    printf "\n"
    echo "tck comp $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/mitppl ./timedlamp/$f.mitl --inf out.tck --tck --noflatten |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "tck flat $f..."
    (time timeout -s SIGKILL 300 $(timeout -s SIGKILL 300 ../build/mitppl ./timedlamp/$f.mitl --inf out.tck --tck |& tail -n 1)) |& tail -n 10
    printf "\n"
    echo "fp flat $f..."
    (time timeout -s SIGKILL 300 ../build/mitppl ./timedlamp/$f.mitl --inf) |& tail -n 8
    printf "\n"
    printf "\n"
done


echo "Revert to the original version of MightyPPL..."
cp -f ../MightyPPL_original.cpp ../MightyPPL.cpp
cd ../build && make
cd ../testcases
printf "\n"

