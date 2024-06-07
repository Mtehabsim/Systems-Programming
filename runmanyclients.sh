#!/bin/bash

for i in {1..3}; do
    ./client 2 Name.txt &
done

for i in {1..3}; do
    ./client 1 &
done

wait

echo "All instances of client have completed"
