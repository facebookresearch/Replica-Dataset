set TARGET=c:\\replica
cd /d %TARGET%
if EXIST merge.tar.gz (
    echo error merge.tar.gz exists, delete it please
    goto :eof
)
copy /b replica_v1_0.tar.gz.part* merge.tar.gz

REM Then run: tar -xvzf merge.tar.gz
