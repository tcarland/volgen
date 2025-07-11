#!/usr/bin/env bash
#
#  voldiff.sh
#
#  @file    voldiff.sh
#  @author  Timothy C. Arland <tcarland at gmail dot com>
PNAME=${0##\/.*}
VERSION="v25.05"

voldir=".volgen"
prefix=
mkdiff=0

declare -A saved
declare -A sizes
declare -a missing

# -----------------------------------

usage="
A diff tool for matching the volumes generated by 'volgen' against
the current working directory to list untracked items.

Synopsis:
  $PNAME [options] <prefix_match>

Where 'prefix_match' is the directory/file prefix name to match.

Options:
  -h|--help          : Show usage info and exit.
  -d|--voldir <path> : Alternate location to '.volgen' meta dir.
  -V|--version       : Show version info and exit.
"

version="$PNAME $VERSION"

# -----------------------------------

while [ $# -gt 0 ]; do
    case "$1" in
    -d|--voldir)
        voldir="$2"
        shift
        ;;
    'help'|-h|--help)
        echo "$usage"
        exit 0
        ;;
    'version'|-V|--version)
        echo "$version"
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
    echo "$usage"
    exit 1
fi

mmw=$(ls . | grep -i "$prefix")
vols=$(ls .volgen/ | grep "Volume_")
tsz=0

for vol in $vols; do
    items=$(ls -1 ".volgen/${vol}/")

    for f in $items; do
        saved[$f]="$vol"
    done

    size=$(du -L -m -c .volgen/${vol} | grep total 2>/dev/null | awk '{ print $1 }')
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
    size=$(du -L -m -c ./$m/ | grep total 2>/dev/null | awk '{ print $1 }')
    printf " $m  [ $size Mb ] \n"
    tsz=$(( $tsz + $size ))
done

printf " ----------------- \n";
printf " Total size missing: $tsz Mb\n";

exit 0
