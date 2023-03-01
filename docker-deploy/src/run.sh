#!/bin/bash
make clean
make all
echo 'start running proxy server...'
./main &
/bin/bash
