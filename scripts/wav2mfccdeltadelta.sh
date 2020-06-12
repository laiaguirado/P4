#!/bin/bash
## \file
## \TODO This file implements a very trivial feature extraction; use it as a template for other front ends.
## 
## Please, read SPTK documentation and some papers in order to implement more advanced front ends.

# Base name for temporary files
base=/tmp/$(basename $0).$$ 

# Ensure cleanup of temporary files on exit
trap cleanup EXIT
cleanup() {
   \rm -f $base.*
}


if [[ $# != 4 ]]; then

   echo "$0 mfcc_order nfilter input.wav output.mfcc.accel"
   exit 1

fi

mfcc_order=$1
nfilter=$2
inputfile=$3
outputfile=$4


UBUNTU_SPTK=1

if [[ $UBUNTU_SPTK == 1 ]]; then

   # In case you install SPTK using debian package (apt-get)
   X2X="sptk x2x"
   FRAME="sptk frame"
   MFCC="sptk mfcc"
   DELTA="sptk delta"

else

   # or install SPTK building it from its source
   X2X="x2x"
   FRAME="frame"
   MFCC="mfcc"
   DELTA="delta"

fi

# Main command for feature extration
sox $inputfile -t raw -e signed -b 16 - | $X2X +sf | $FRAME -l 240 -p 80 |
	$MFCC -s 8000 -n $nfilter -l 240 -m $mfcc_order > $base.mfcc 

#$DELTA  -l 240 -m $mfcc_order -d -0.5 0 0.5 $base.mfcc > $base.mfcc.dif
$DELTA -l 240 -m $mfcc_order -d 1.0 -2 1.0 $base.mfcc > $base.mfcc.accel

# Our array files need a header with the number of cols and rows:
ncol=$((mfcc_order))
nrow=`$X2X +fa < $base.mfcc.accel | wc -l | perl -ne 'print $_/'$ncol', "\n";'`


# Build fmatrix file by placing nrow and ncol in front, and the data after them
echo $nrow $ncol | $X2X +aI > $outputfile
cat $base.mfcc.accel >> $outputfile


exit