#!/bin/bash
for((i=0; i<20; i++))
do
./sdstore proc-file in out1 nop &
./sdstore proc-file in out1 nop &
./sdstore proc-file in out1 nop &
./sdstore proc-file in out1 nop &
./sdstore proc-file in out1 nop &
./sdstore status &
./sdstore proc-file in out1 nop &
./sdstore proc-file in out1 nop &
done