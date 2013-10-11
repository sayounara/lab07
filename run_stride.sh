#!/bin/bash

for (( EXPO=15 ; EXPO<=30; EXPO+=1 )); do
    (( SIZE=2**EXPO ))
    echo "# running stride with $SIZE"
    ./stride $SIZE
done


