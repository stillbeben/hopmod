#!/bin/bash
if [ "$#" -le 1 ]; then
 echo "usage: ./merge_sauer_svn.sh <rev-from> <rev-to>"
 exit 1
fi
svnurl="svn://svn.code.sf.net/p/sauerbraten/code/src/"
bash -c "svn merge -r$1:$2 $svnurl ." || exit 1
