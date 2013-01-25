#!/bin/bash
#
#  mkisofs -pad -f -J -R -o .iso
#  -f follow symlinks
#  -J Joliet extentions
#  -R RockRidge extensions
#

VERSION="0.12"
AUTHOR="tcarland@gmail.com"

VERBOSE=0
DRYRUN=0
RESET=0
MKISO="mkisofs -pad -f"
target=
path="./"

usage()
{
    echo ""
    echo "Usage: $0 [options] path/to/save/file.iso"
    echo "    options:"
    echo "      -h|--help    = Display usage information"
    echo "      -n|--dry-run = Show command but don't execute"
    echo "      -o|--object  "
    echo "      -t|--target  = iso target "
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

if [ $DRYRUN -eq 1 ]; then
    echo "  DRYRUN enabled"
fi

if [ -z "$target" ]; then
    echo "no target defined"
    usage
    exit 1
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
    echo "$cmd"
else
    ($cmd)
fi

exit 0





