#!/bin/bash
for((i=0; i<4; i++))
do
./sdstore proc-file in out0 bcompress &
./sdstore proc-file in out1 nop encrypt &
./sdstore status &
./sdstore proc-file in out2 encrypt nop &
./sdstore proc-file in out3 encrypt decrypt &
done