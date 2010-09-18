#!/bin/sh
THREADS=`cat /proc/cpuinfo | grep processor | wc -l`
ARG_LENGTH=$# 
STRCOMPILE="Compiling"
if [ "$ARG_LENGTH" > 0 -a "$1" = 1 ]; then
  STRCOMPILE="Recompiling"
  rm -rf release-build
fi
mkdir release-build
cd release-build 
cmake .. >> /dev/null
STRTHREADS="threads"
if [ $THREADS = 1 ]; then
  STRTHREADS="thread"
fi
echo "$STRCOMPILE Hopmod using $THREADS $STRTHREADS\n"
TS_START=`date +%s`
make -j$THREADS 
make install >> /dev/null
TS_END=`date +%s`
TS_DIFF=`echo $TS_END $TS_START | awk '{print $1 - $2}'`
echo "\nTook $TS_DIFF Seconds"
