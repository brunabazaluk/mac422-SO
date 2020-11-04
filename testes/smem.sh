#!/bin/bash

for i in `seq 1 5`
do
	echo `/usr/bin/time -v ./a.out 10 10 2>&1 | grep Maximum`
done

