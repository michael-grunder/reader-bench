#!/bin/bash

(cd hiredis && git checkout master)
make
mv bench bench-master
mv raw-bench raw-bench-master
mv incr-bench incr-bench-master

(cd hiredis && git checkout unlimited-depth)
make
mv bench bench-unlimited-depth
mv raw-bench raw-bench-unlimited-depth
mv incr-bench incr-bench-unlimited-depth
