#!/bin/bash
./sdstore proc-file in out1 nop  &
P1=$!
./sdstore proc-file in out2 encrypt &
P2=$!
wait $P1 $P2