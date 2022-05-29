#!/bin/bash
./sdstore proc-file 0 out0 encrypt decrypt encrypt &
./sdstore status &
./sdstore proc-file 1 out1 encrypt decrypt &
./sdstore status &
./sdstore proc-file 2 out2 encrypt nop nop nop &
./sdstore status &
./sdstore proc-file 3 out3 nop encrypt &
./sdstore status &