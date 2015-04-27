#!/bin/bash
for i in {1..5}
do
	filename="test.log.$i"
	touch "$filename"
	echo "asfasfasdfasdfasdfasfdfdasf" > "$filename"
done
for i in {6..10}
do
	filename="test.log.$i"
	touch "$filename.gz"
done
