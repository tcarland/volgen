#!/bin/bash
#
#  voldiff.sh 
#
PNAME=${0##\/.*}
AUTHOR="tcarland@gmail.com"

voldir=".volgen"
prefix="$1"


if [ ! -d $voldir ]; then
    echo "Error: volgen metadata directory not found."
    echo " Run volgen first?"
    exit 1
fi

mmw=$( ls . | grep -i "$prefix" )

vols=$( ls .volgen/ | grep "Volume_" )

declare -a saved
declare -a sizes


for vol in "$volds"; do
    items=$( ls -1 ".volgen/${vol}/" )

    for f in $items; do
        saved[$f] = $vol
    done

    size=$( du -L -m -s .volgen/${vol} )
    tsz=((tsz + size))

    printf " $vol  |  $size Mb\n"
    sizes[$vol] = $size
done 

for dir in $mmw; do 
    if [ ! -d $dir ]; then 
        continue
    fi
    #if not exists $saved[$dir]; then 
        #push missing
    #fi
done

ncur=${#mmw}
nsvd=${#saved}
nmis=${#missing}

printf " ----------------- \n";
printf " total size: $tsz Mb\n\n";
printf " total items $num_mmw \n";
printf " saved items $num_saved\n\n";
printf " missing items $sz\n";
printf " ----------------- \n";

for m in $missing; do 
    size=$( du -L -m -s ./$m/ )

    printf " $m  [ $size Mb ]"
    $tsz=(( tsz + $size ))
done

printf " ----------------- \n";
printf " total size: $tsz Mb\n";

exit 0
