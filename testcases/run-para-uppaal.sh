#!/bin/bash
FILES="
F-4-2i
G-4-2i
U-4-02
U-4-2i
X-2-02
X-2-2i
"
for f in $FILES
do
    printf "\n"
    printf "\n"
    echo "uppaal $f..."
    (time timeout -s SIGKILL 300 verifyta ./parametric/$f.xml ./parametric/$f.q) |& tail -n 8
    printf "\n"
    printf "\n"
done

