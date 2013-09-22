#!/bin/sh
cd /home/thunderliu/projects/tsnetfw/src
make clean
cd ../..
tar cjf tsnetfw.tar.bz2 tsnetfw
sz tsnetfw.tar.bz2

