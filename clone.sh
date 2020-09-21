#!/bin/bash

for ((var = 3; var < 5; var++)); do
  cp -f testfile1.txt testfile${var}.txt
  cp -f testin1.txt testin${var}.txt
  cp -f testout1.txt testout${var}.txt
done

zip testfile.zip test*.txt