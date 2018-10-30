#!/bin/bash
inLst=$1 # e.g. ../Lists/ColorFERET/9.GT.lst
outLst=$2 # e.g. 9.GT.lst
imgPath=$3 # e.g. ../Data/ColorFERET/images/all/
FaceFinder -fd:off -fd:sub: -lst:in $inLst -lst:out $outLst -p $imgPath
