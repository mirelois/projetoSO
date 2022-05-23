#!/bin/bash
./sdstore proc-file in out1 nop  &
P1=$!
./sdstore proc-file -p 4 in out2 encrypt &
P2=$!
./sdstore proc-file in out3 nop nop nop &
./sdstore proc-file -p 1 out2 out4 decrypt &
./sdstore proc-file -p 3 in out5 gcompress nop &
./sdstore status
wait $P1 $P2
