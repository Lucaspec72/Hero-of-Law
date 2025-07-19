#!/bin/sh

for dir in */; do
  (cd "$dir" && make clean >/dev/null && make -j) &
done
wait

if command -v py >/dev/null 2>&1; then
    py -3 ../../../tool/inject_file.py inject.csv 
else
    python3 ../../../tool/inject_file.py inject.csv
fi


