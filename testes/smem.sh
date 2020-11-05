#!/bin/bash

for i in `seq 1 30`
do
	echo `/usr/bin/time -v ./a.out $1 $2 2>&1 | grep -E '(Maximum | time)'`
	#echo $1 $2
done

