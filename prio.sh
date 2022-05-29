#!/bin/bash
./sdstore proc-file -p 5 0 out0 bcompress &
./sdstore proc-file -p 2 1 out1 nop encrypt &
./sdstore status &
./sdstore proc-file -p 1 2 out2 encrypt nop &
./sdstore proc-file -p 0 3 out3 encrypt decrypt &