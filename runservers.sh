#!/bin/bash

./sortserver &
./hashserver &
wait

echo "Both servers have completed"
