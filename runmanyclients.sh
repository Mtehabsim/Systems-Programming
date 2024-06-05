#!/bin/bash

# Run client with param 2 and Name.txt three times
for i in {1..3}; do
    ./client 2 Name.txt &
done

# Run client with param 1 three times
for i in {1..3}; do
    ./client 1 &
done

# Wait for all background processes to finish
wait

echo "All instances of client have completed"
