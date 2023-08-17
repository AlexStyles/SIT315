#!/bin/bash

for i in {1..50}; do
  ./vectoradd.o >> vectoradd.log
  # ./vectoradd++.o >> vectoradd++.log
  ./threads.o >> threads.log
  ./sequential.o >> sequential.log
done