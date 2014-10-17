#!/usr/bin/perl
#-----------------------------------------------------------------------
#
#  voldiff.pl
#
#    A difftool for matching the volumes generated by 'volgen' against 
#  the current working directory to display untracked or volumed items.
#
#  @file    voldiff.pl
#  @author  Timothy C. Arland <tcarland@gmail.com>
#-----------------------------------------------------------------------
package volgen::voldiff::main;

use strict;


#-----------------------------------------------------------------------

my $VERSION  = "0.832";
my $AUTHOR   = 'tcarland@gmail.com';
my $voldir   = ".volgen";
my $prefix   = shift;

#-----------------------------------------------------------------------

sub usage()
{
    print "Usage: voldiff.pl <prefix> in the current working dir. \n";
}


sub version()
{
    print "voldiff.pl v$VERSION by $AUTHOR\n"
}

#-----------------------------------------------------------------------

INIT:
{
    if ( $prefix =~ m/^-h/ ) {
        usage();
        exit(0);
    }
    
    if ( $prefix =~ m/^-V/ ) {
        version();
        exit(0);
    }

    if ( not -e "$voldir" ) {
        print "Error: volgen directory '$voldir' not found!\n";
        print "Have you run 'volgen' first?\n\n";
        usage();
        exit(0);
    }
}

#-----------------------------------------------------------------------

MAIN: 
{
    my $match1 = `ls . | grep -i "$prefix"`;
    my $match2 = `ls .volgen/ | grep "Volume_"`;

    my @mmw    = split('\n', $match1);
    my @vols   = split('\n', $match2);
    my $tsz    = 0;

    my %saved;
    my %sizes;
    my @missing;

    print "\n voldiff for prefix '$prefix'\n\n";
    print " Disc Size  |   Results: \n";
    print " ----------------- \n";

    foreach my $vol (@vols) {
        my $shws = `ls -1 .volgen/"$vol"/`;
        chomp $shws;
        my @foo  = split('\n', $shws);

        foreach my $f ( @foo ) {
            $saved{$f} = $vol;
        }

        my $size = `du -L -m -s .volgen/"$vol"/`;
        chomp $size;
        $size =~ s/^(\d+).+$/$1/;
        $tsz += $size;

        print " $vol  |   $size Mb\n";
        $sizes{$vol} = $size;
    }

    foreach my $d ( @mmw ) {
        next if not -d $d;
        push(@missing, $d) if not exists $saved{$d};
    }


    my $num_mmw   = scalar @mmw;
    my $num_saved = scalar keys %saved;
    my $sz        = scalar @missing;

    print " ----------------- \n";
    print " total size: $tsz Mb\n\n";
    print " total items $num_mmw \n";
    print " saved items $num_saved\n\n";

    print " missing items $sz\n";
    print " ----------------- \n";

    $tsz = 0;
    foreach my $m (@missing) {
        my $size = `du -L -m -s ./"$m"/`;
        chomp $size;
        $size    =~ s/^(\d+).+$/$1/;
        print "  $m  [ $size Mb ]\n";
        $tsz    += $size;
    }
    print " ----------------- \n";
    print " total size: $tsz Mb\n";

    exit 0;
} # MAIN

#-----------------------------------------------------------------------

1;



