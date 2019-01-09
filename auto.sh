#!/bin/bash

llc_policy=pd
core=1
executable=bimodal-no-no-${llc_policy}-${core}core

trace="bzip2_183B"
n_warm=1
n_sim=100

# if [ ! -f bin/$executable ]; then
  ./build_champsim.sh bimodal no no $llc_policy $core
# fi

if [ ! -f run_champsim.sh ]; then
  cp scripts/run_champsim.sh ./
fi

echo "./run_champsim.sh $executable $n_warm $n_sim $trace"
./run_champsim.sh $executable $n_warm $n_sim $trace
