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
