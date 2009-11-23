#!/bin/sh

for filename in $( ls *.jpg ); do
    convert $filename -thumbnail 64x64 $filename
done

