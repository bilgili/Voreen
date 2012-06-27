#!/usr/bin/perl
# Replaces tabs by 4 spaces, removes trailing whitespace on line end, adds newline to EOF

use strict;
use warnings;
use File::Find;
use File::Copy;

my @dirs = qw(src include apps/voreenve apps/simple apps/tests apps/voltool);
my $interesting = 'cpp|h|frag|vert|cu|cl';
my $skip_dirs = '\.svn|\.moc|\.obj|\.ui|ext';

chdir('..');
find(\&wanted, @dirs);

sub wanted {
    if ($_ =~ /^($skip_dirs)$/) {
        $File::Find::prune = 1;
        return;
    };
    if ((-f $_) and ($_ =~ /\.($interesting)$/)) {
        update_untabify($_);
    }
}

sub update_untabify {
    my $file = shift;
    open F, $file or die "Can't open $file: $!";
    my @f = <F>;
    close F;

    my $tmpfile = $file . ".tmp";
    open F, ">", $tmpfile or die "Can't open $tmpfile for writing: $!";
    my $line = 1;
    my $changed = 0;
    my $tabs = 0;
    my $trailing = 0;

    my $l = '';
    foreach (@f) {
        $l = $_;
        if ($l =~ s/\t/    /g) {
            $changed = 1;
            $tabs++;
        }
        if ($l =~ s/\s+\n$/\n/) {
            $changed = 1;
            $trailing++;
        }

        print F $l;
        $line++;
    }
    my $changeinfo = '';
    $changeinfo .= "$tabs tabs" if ($tabs > 0);
    if ($trailing > 0) {
        $changeinfo .= ', ' if $changeinfo ne '';
        $changeinfo .= "$trailing trailing" ;
    }
    if ($l !~ /\n$/) {
        print F "\n";
        $changed = 1;
        $changeinfo .= ', ' if $changeinfo ne '';
        $changeinfo .= 'added newline to end';
    }

    close F;
    if ($changed) {
        move($tmpfile, $file);
        print $File::Find::name . ": $changeinfo\n";
    } else {
        unlink($tmpfile);
    }
}
