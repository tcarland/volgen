#!/bin/bash
#
#  mkisofs -pad -f -J -R -o .iso
#  -f follow symlinks
#  -J Joliet extentions
#  -R RockRidge extensions
#  -r RockRidge with reset perms
#
#  @file    mkiso.sh
#  @author  Timothy C. Arland <tcarland@gmail.com>
#
#  Copyright (c) 2009-2012 Timothy C. Arland <tcarland@gmail.com>
#
PNAME=${0##*\/}
VERSION="0.4.1"
AUTHOR="tcarland@gmail.com"

VERBOSE=0
DRYRUN=0
RESET=0
LOOP=0
MKISO="mkisofs -pad -f -J"
target=
path="./"


usage()
{
    echo ""
    echo "Usage: $PNAME [options] [path/to/archive]"
    echo "    options:"
    echo "      -h|--help        = Display usage information"
    echo "      -n|--dry-run     = Show command but don't execute"
    echo "      -L|--loop        = mount target iso on loop device"
    echo "      -o|--object      = output to target .iso image"
    echo "      -t|--target      = same --object"
    echo "      -r|--reset-perms = RockRidge with reset file permissions"
    echo "      -v|--verbose     = Enable verbose output in mkisofs"
    echo "      -V|--version     = Display version information"
    echo ""
}

version()
{
    echo "$PNAME v$VERSION ($AUTHOR)"
}


ask()
{
    while true; do
        local prompt
        local default

        if [ "${2:-}" == "Y" ]; then
            prompt="Y/n"
            default="Y"
        elif [ "${2:-}" == "N" ]; then
            prompt="y/N"
            default="N"
        else
            prompt="y/n"
            default=
        fi

        read -p "$1 [$prompt] " REPLY

        if [ -z "$REPLY" ]; then
            REPLY=$default
        fi

        case "$REPLY" in
            Y*|y*) return 0 ;;
            N*|n*) return 1 ;;
        esac
    done
}

#----------------
# MAIN
#----------------

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
    if ask "Target exists, do you want to overwrite?" N; then
        echo "  Overwriting $target"
    else
        echo "  You said no, stopping.."
        exit 0
    fi
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

# RockRidge protocol
if [ $RESET -eq 1 ]; then
    cmd="$cmd -r"
else
    cmd="$cmd -R"
fi


cmd="$cmd -o $target -publisher $AUTHOR $path"


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
