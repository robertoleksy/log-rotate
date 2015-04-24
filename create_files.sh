#!/bin/bash
for i in {1..5}
do
	filename="test.log.$i"
	touch "$filename"
	touch "$filename.gz"
done
