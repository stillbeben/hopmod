#!/bin/sh

DEST=$1

mkdir -p $DEST

ln -s $PWD/bin $DEST/bin
ln -s $PWD/lib $DEST/lib
ln -s $PWD/script $DEST/script
ln -s $PWD/share $DEST/share

mkdir $DEST/conf
mkdir $DEST/log
mkdir $DEST/log/game
mkdir $DEST/log/demo

cp conf/server.conf $DEST/conf/server.conf
cp conf/maps.conf $DEST/conf/maps.conf
