#!/bin/bash
for i in {1..2000}
do 
    ./sdstore proc-file in out1 nop &
done