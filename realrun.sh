#!/bin/bash

gcc sortserver.c -o sortserver -lssl -lcrypto
gcc hashserver.c -o hashserver -lssl -lcrypto
gcc combinedclient.c -o client -lssl -lcrypto

./sortserver &
./hashserver &

sleep 2

for i in {1..3}; do
    ./client 2 Name.txt &
done

for i in {1..3}; do
    ./client 1 &
done

wait
