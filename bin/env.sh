#!/bin/sh

export PATH="$PATH:."
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:bin"
export LUA_PATH="$LUA_PATH;script/package/?.lua"
export LUA_CPATH="$LUA_CPATH;bin/lib?.so"
