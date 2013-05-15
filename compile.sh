#!/bin/sh
export REVISION=`svn info --xml | grep -m 1 -E 'revision *\= *"[0-9]+" *>' | grep -oE '[0-9]+'`
case "$(uname -s)" in
  *Darwin*|*BSD*)
    : ${THREADS:=$(sysctl -n hw.ncpu)} ;;
  *)
    : ${THREADS:=`cat /proc/cpuinfo | grep processor | wc -l`} ;;
esac
ARG_LENGTH=$# 
STRCOMPILE="Compiling"
COMPILEDIR="release_build"
COMPILEFLAGS=""
BUILDTYPE="release"
if [ "$2" = "debug" ]; then
  COMPILEDIR="debug-build"
  COMPILEFLAGS="-D CMAKE_BUILD_TYPE=DEBUG"
  BUILDTYPE="debug"
fi
if [ "$1" = 1 ]; then
  STRCOMPILE="Recompiling"
  rm -rf $COMPILEDIR
fi
[ -d $COMPILEDIR ] || mkdir $COMPILEDIR
cd $COMPILEDIR
cmake $COMPILEFLAGS ..
STRTHREADS="threads"
if [ $THREADS = 1 ]; then
  STRTHREADS="thread"
fi
echo "$STRCOMPILE Hopmod r$REVISION using $THREADS $STRTHREADS ($BUILDTYPE build)"
TS_START=`date +%s`
make -j$THREADS 
make install >> /dev/null
TS_END=`date +%s`
TS_DIFF=`echo $TS_END $TS_START | awk '{print $1 - $2}'`
echo "Took $TS_DIFF Seconds"
