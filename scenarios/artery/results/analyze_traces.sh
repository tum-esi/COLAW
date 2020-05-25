#!/bin/sh
scavetool x $1.sca $1.vec -o $1.csv

python3 proof_relationship.py $1.csv

python3 proof_size.py $1.csv