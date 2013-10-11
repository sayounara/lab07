#!/bin/bash

for (( EXPO=3 ; EXPO<=28; EXPO+=1 )); do
    (( SIZE=2**EXPO ))
    echo "# running stride with $SIZE"
    ./stride $SIZE
done


