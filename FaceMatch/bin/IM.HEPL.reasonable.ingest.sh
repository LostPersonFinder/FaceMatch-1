#!/bin/bash
ImageMatcher -fm:$1 -lst ../Lists/HEPL/500.reasonable/64.GT.lst -ndx:out HEPL.reasonable.64.$1.ndx -p ../Data/HEPL/Images/Original/ -m:i $2 $3 $4 $5
