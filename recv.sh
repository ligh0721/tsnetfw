#!/bin/sh
cd /home/thunderliu/projects
rm tsnetfw.tar.bz2
rz
tar xjf tsnetfw.tar.bz2
cd tsnetfw/src
make clean
make

