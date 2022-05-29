#!/bin/bash
for ((i=0;i<4;i++))
  do
    rm -f $i;
    rm -f out$i;
done

for ((i=0;i<4;i++)); do
    fallocate -l 200M $i;
done