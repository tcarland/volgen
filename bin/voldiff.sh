#!/usr/bin/env bash
#
#  voldiff.sh 
#
PNAME=${0##\/.*}
AUTHOR="tcarland@gmail.com"
VERSION="v20.12"

voldir=".volgen"
prefix=

# -----------------------------------

usage()
{
    printf "Usage: $PNAME <prefix_match> \n"
    printf "  Where 'prefix_match' is the directory/file prefix name to match"
}

version()
{
    printf "$PNAME $VERSION"
}

# -----------------------------------

while [ $# -gt 0 ]; do
    case "$1" in
        'help'|-h|--help)
            usage
            exit 0
            ;;
        'version'|-V|--version)
            version
            exit 0
            ;;
        *)
            prefix="$1"
            shift $#
            ;;
    esac
    shift
done


if [ ! -d $voldir ]; then
    echo "$PNAME Error: volgen directory '$voldir' not found."
    echo " Run volgen first to create the metadata directory."
    exit 1
fi

if [ -z "$prefix" ]; then
    echo "$PNAME Error: match prefix not provided."
    usage
    exit 1
fi


mmw=$( ls . | grep -i "$prefix" )
vols=$( ls .volgen/ | grep "Volume_" )
tsz=0

declare -A saved
declare -A sizes
declare -a missing

for vol in $vols; do
    items=$( ls -1 ".volgen/${vol}/" )

    for f in $items; do
        saved[$f]="$vol"
    done

    size=$( du -L -m -c .volgen/${vol} | grep total 2>/dev/null | awk '{ print $1 }' )
    tsz=$((tsz + size))

    printf " $vol  =  $size Mb\n"
    sizes[$vol]=$size
done 

missing=()

for dir in $mmw; do 
    if [ ! -d $dir ]; then 
        continue
    fi
    if [ ! -v saved[$dir] ]; then
        missing+=("$dir")
    fi
done

ntotal=${#mmw}
nsaved=${#saved[@]}
nmissed=${#missing[@]}

printf " ----------------- \n";
printf " Total size:   : $tsz Mb \n\n";
printf " Total items   : $ntotal \n";
printf " Saved items   : $nsaved \n\n";
printf " Missing items : $nmissed \n";
printf " ----------------- \n";

tsz=0
for m in ${missing[@]}; do 
    size=$( du -L -m -c ./$m/ | grep total 2>/dev/null | awk '{ print $1 }' )
    printf " $m  [ $size Mb ] \n"
    tsz=$(( $tsz + $size ))
done

printf " ----------------- \n";
printf " Total size missing: $tsz Mb\n";

exit 0
