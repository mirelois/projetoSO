#!/bin/bash
for((i=0; i<10; i++))
do
./sdstore proc-file large.test out1 encrypt &
./sdstore proc-file in out2 bcompress &
./sdstore proc-file in out3 nop encrypt &
./sdstore proc-file in out4 encrypt nop &
./sdstore proc-file in out5 encrypt decrypt &
./sdstore status &
./sdstore proc-file in out6 bcompress bdecompress &
./sdstore proc-file in out7 nop nop nop &
done