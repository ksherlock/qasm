#!/bin/bash

X=
if [ "$1""L" != "L" ] ; then
	X=_$1
fi

OUTDIR=./m32out

rm -rf $OUTDIR
mkdir -p $OUTDIR

SRC=`ls ./testdata | grep -E '^([0-9]+)(.*)\.[Ss]'`

#SRC=`ls ./testdata | grep -E '^([0-9]+)(.*)\.[Ss]' | grep -i 2007`

for S in $SRC ; do

	S1=$S
	S1=${S1/.S/}
	S1=${S1/.s/}

	BASE=${S/.S/}
	BASE=${BASE/.s/}
	cd ./testdata
	merlin32$X . $S 2>/dev/null >/dev/null
	#merlin32 . $S 2>/dev/null 

	R=?$
	cd - >/dev/null
	cp ./testdata/$S1 $OUTDIR/$S1.bin 2>/dev/null
	rm -f ./testdata/*.txt 2>/dev/null
	rm -f ./testdata/$S1 2>/dev/null
	R=?$

done
ls $OUTDIR



