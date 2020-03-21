#!/bin/bash
#PBS -l walltime=00:10:00,nodes=7:ppn=4
#!/bin/bash
DIRNAME=`dirname "$0"`
make -C $DIRNAME out-64 || exit -1
rm -rf $DIRNAME/build/res.txt
for ((i = 1; i <= 1000; ++i))
do
$DIRNAME/build/out-64 $i 10000000 1 >> build/res.txt
done
exit $?
