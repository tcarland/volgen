#!/bin/bash
#
#  mkisofs -pad -f -J -R -o .iso
#  -f follow symlinks
#  -J Joliet extentions
#  -R RockRidge extensions
#

VERSION="0.15"
AUTHOR="tcarland@gmail.com"

VERBOSE=0
DRYRUN=0
RESET=0
LOOP=0
MKISO="mkisofs -pad -f"
target=
path="./"

usage()
{
    echo ""
    echo "Usage: $0 [options] path/to/convert/toiso/"
    echo "    options:"
    echo "      -h|--help    = Display usage information"
    echo "      -n|--dry-run = Show command but don't execute"
    echo "      -L|--loop    = mount target iso on loop device"
    echo "      -o|--object  or "
    echo "      -t|--target  = output .iso target path and name "
    echo "      -v|--verbose = Enable verbose output in mkisofs"
    echo "      -V|--version = Display version information"
    echo ""
}

version()
{
    echo "$0 v$VERSION ($AUTHOR)"
}


# process command arguments
while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        -n|--dry-run)
            DRYRUN=1
            ;;
        -L|--loop)
            LOOP=1
            ;;
        -o|--object)
            target="$2"
            shift
            ;;
        -t|--target)
            target="$2"
            shift
            ;;
        -r|--reset-perms)
            RESET=1
            ;;
        -v|--verbose)
            VERBOSE=1
            ;;
        -V|--version)
            version
            exit 0
            ;;
        *)
            path="$1"
            shift
            ;;
    esac
    shift
done

if [ -z "$target" ]; then
    echo "No iso target defined!"
    usage
    exit 1
fi

version

if [ -e $target ] && [ $LOOP -eq 0 ]; then
    echo "Target exists, overwrite (y/n)?"
    select yn in "Yes" "No"; do
        case $yn in
            Yes) echo "  Overwriting $target"; break;;
            No ) echo "  You said $yn, stopping.."; exit;;
        esac
    done
fi

if [ $DRYRUN -eq 1 ]; then
    echo "  DRYRUN enabled"
fi
if [ $LOOP -eq 1 ]; then
    echo "  Loop enabled, mounting iso using /dev/loop0"
fi

echo "Using iso target: '$target'"

cmd="$MKISO"

if [ $VERBOSE -eq 1 ]; then
    cmd="$cmd -v"
fi

if [ $RESET -eq 1 ]; then
    cmd="$cmd -r"
else
    cmd="$cmd -R"
fi

cmd="$cmd -J -o $target -publisher $AUTHOR $path"

if [ $DRYRUN -eq 1 ]; then
    echo ""
    if [ ${LOOP} -eq 1 ]; then
        echo "mount -t iso9660 -o ro,loop=/dev/loop0 $target /mnt/cdrom"
    else
        echo "$cmd"
    fi
    echo ""
else
    if [ $LOOP -eq 1 ]; then
        ( mount -t iso9660 -o ro,loop=/dev/loop0 $target /mnt/cdrom )
    else
        ($cmd)
    fi
fi

exit 0





