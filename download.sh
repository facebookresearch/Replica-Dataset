#!/usr/bin/env bash

# Abort on error
set -e

if [ "$1" != "" ]; then
  echo -e "\nDownloading and decompressing Replica to $1. The script can resume\npartial downloads -- if your download gets interrupted, simply run it again.\n"
else
  echo "Specify a path to download and decompress Replica to!"
  exit 1
fi

for p in {a..q}
do
  # Ensure files are continued in case the script gets interrupted halfway through
  wget --continue https://github.com/facebookresearch/Replica-Dataset/releases/download/v1.0/replica_v1_0.tar.gz.parta$p
done

# Create the destination directory if it doesn't exist yet
mkdir -p $1

cat replica_v1_0.tar.gz.part?? | unpigz -p 32  | tar -xvC $1

#download, unzip, and merge the additional habitat configs
wget http://dl.fbaipublicfiles.com/habitat/Replica/additional_habitat_configs.zip -P assets/
unzip -qn assets/additional_habitat_configs.zip -d $1