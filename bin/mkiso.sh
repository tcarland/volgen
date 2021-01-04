#!/usr/bin/env bash
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
PNAME=${0##*\/}
VERSION="v20.12"

author=
target=
path=
dryrun=0
reset=0
loop=
ldev=

MKISOFS=$(which genisoimage 2>/dev/null)
MKISOARGS="-pad -f -J"

# ---------------------------------------------------------

usage()
{
    echo ""
    echo "Usage: $PNAME [options] <path>"
    echo "  -a|--author <str> : Set iso image author string"
    echo "  -h|--help         : Show usage info and exit."
    echo "  -L|--loop <mntpt> : Mount iso target as a loop device."
    echo "  -n|--dryrun       : Shows command only with no execution."
    echo "  -t|--target <img> : Name of target ISO image."  
    echo "  -r|--reset-perms  : Enable RockRidge with reset file permissions."
    echo "  -V|--version      : Show version info and exit."
    echo ""
}

version()
{
    echo "$PNAME $VERSION"
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


# ---------------------------------------------------------
# MAIN
rt=0

while [ $# -gt 0 ]; do
    case "$1" in
        -a|--author)
            author="$2"
            shift
            ;;
        'help'|-h|--help)
            usage
            exit $rt
            ;;
        -n|--dry-run|--dryrun)
            dryrun=1
            ;;
        -L|--loop)
            loop="$2"
            shift
            ;;
        -t|--target)
            target="$2"
            shift
            ;;
        -r|--reset-perms)
            reset=1
            ;;
        'version'|-V|--version)
            version
            exit $rt
            ;;
        *)
            path="$1"
            shift
            ;;
    esac
    shift
done


if [[ -z "$path" || -z "$target" ]]; then
    echo "$PNAME Error: Missing arguments."
    usage
    exit 1
fi

if [ -z "$MKISOFS" ]; then 
    MKISOFS=$(which mkisofs 2>/dev/null)
    if [ -z "$MKISOFFS" ]; then 
        echo "$PNAME Error. Neither 'genisoimage' or 'mkisofs' found in PATH."
        exit 2
    fi
fi 

if [[ -e $target && $loop -eq 0 ]]; then
    if ask "Output image exists! Do you want to overwrite?" N; then
        echo "  Overwriting target image '$target'"
    else
        echo "$PNAME Aborting.."
        exit 1
    fi
fi

if [ $dryrun -eq 1 ]; then 
    echo " -> DRYRUN Enabled."
fi

if [ -n "$loop" ]; then 
    echo " -> Loop Enabled, mounting ISO to $loop"

    if [[ -e $loop && ! -d $loop ]]; then
        echo "$PNAME Error, mount path already exists and is not a directory!"
        exit 1
    fi
    if [[ ! -e $loop ]]; then
        ( mkdir $loop )
        rt=$?
        if [ $rt -ne 0 ]; then 
            echo "$PNAME Error creating mount path '$loop'"
            exit 1
        fi
    fi
fi 


echo " -> Using ISO Target of '$target'"

cmd="$MKISOFS $MKISOARGS"

if [ $reset -eq 1 ]; then 
    cmd="$cmd -r"
else 
    cmd="$cmd -R"
fi


cmd="$cmd -o $target $path"


if [ $dryrun -eq 1 ]; then 
    echo ""
    if [ -n "$loop" ]; then 
        echo "mount -t iso9660 -o ro,loop=/dev/loop0 $target /mnt/cdrom"
    else
        echo "( $cmd )"
    fi

    echo ""
else
    if [ -n "$loop" ]; then
        ( mount -t iso9660 -o ro,loop=$ldev $target $mnt )
    else
        ( $cmd )
        rt=$?
    fi
fi 

exit $rt
