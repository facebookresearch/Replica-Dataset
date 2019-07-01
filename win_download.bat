set TARGET=c:\\replica
cd /d %TARGET%
for %%p in (a b c d e f g h i j k l m n o p) do (
c:\\tools\\wget https://github.com/facebookresearch/Replica-Dataset/releases/download/v1.0/replica_v1_0.tar.gz.parta%%p
)
