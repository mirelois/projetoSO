#!/bin/bash
for((i=0; i<4; i++))
do
./sdstore proc-file 0 out0 bcompress &
./sdstore proc-file 2 out1 nop encrypt &
./sdstore status &
./sdstore proc-file 1 out2 encrypt nop &
./sdstore proc-file 3 out3 encrypt decrypt &
done