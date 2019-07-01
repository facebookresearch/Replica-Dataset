@echo off
set TARGET=c:\\replica

if NOT EXIST %TARGET% (
    mkdir %TARGET%
)

cd /d %TARGET%
if ERRORLEVEL 1 (
    echo failed to change directory to %TARGET%
    goto :eof
)

if EXIST merge.tar.gz (
    echo merge.tar.gz already exists, delete it please
    goto :eof
)

REM curl and tar are included in Win10 1803
REM source: https://devblogs.microsoft.com/commandline/tar-and-curl-come-to-windows/

echo Downloading parts..
for %%p in (a b c d e f g h i j k l m n o p q) do (
    curl --location --remote-name https://github.com/facebookresearch/Replica-Dataset/releases/download/v1.0/replica_v1_0.tar.gz.parta%%p
)

echo Merging parts..
copy /b replica_v1_0.tar.gz.part* merge.tar.gz
if ERRORLEVEL 1 (
    echo failed to create merge.tar.gz
    goto :eof
)

echo Unzipping merge.tar.gz..
tar -xvzf merge.tar.gz
if ERRORLEVEL 1 (
    echo failed to unzip merge.tar.gz
    goto :eof
)

echo Deleting merge.tar.gz..
del merge.tar.gz

echo Finished. 
echo (Consider deleting replica_v1_0.tar.gz.part* to save disk space)
