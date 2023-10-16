#!/bin/bash

# EXEC = ./Application
EXEC = $1

sudo perf record -F 99 -g -- ${EXEC}
sudo perf script | ./stackcollapse-perf.pl > out.folded
./flamegraph.pl out.folded > flamegraph.svg