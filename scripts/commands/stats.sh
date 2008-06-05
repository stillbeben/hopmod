#!/bin/sh

CONTENT=$(lynx -dump http://hop.servegame.org/getstats.php?name=$1)

echo "privmsg $2 [$CONTENT]"
