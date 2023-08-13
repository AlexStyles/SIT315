#!/bin/bash

for i in {1..50}; do
  ./parallel.o >> 1m_parallel.log
  ./sequential.o >> 1m_sequential.log
done