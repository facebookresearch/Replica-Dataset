#!/usr/bin/env bash

if [ "$1" != "" ]; then
  echo "Downloading and decompressing Replica to $1"
else
  echo "Specify a path to download and decompress Replica to!"
  exit 1
fi

for p in {a..q}
do 
  wget https://github.com/facebookresearch/Replica-Dataset/releases/download/v1.0/replica_v1_0.tar.gz.parta$p
done

cat replica_v1_0.tar.gz.part?? | unpigz -p 32  | tar -xvC $1
