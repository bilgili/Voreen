#!/usr/bin/perl

use strict;
use warnings;
use File::Find;
use File::Copy;

my $header_file = "header.txt";

my @dirs = qw(.);
my $interesting = 'cpp|h';
my $skip_dirs = '\.svn|\.moc|\.obj|\.ui';

open HEADER, $header_file or die "Can't open $header_file: $!";
my @header = <HEADER>;
close HEADER;

find(\&wanted, @dirs);

sub wanted {
    if ($_ =~ /^($skip_dirs)$/) {
        $File::Find::prune = 1;
        return;
    };
    if ((-f $_) and ($_ =~ /\.($interesting)$/)) {
        update_header($_);
    }
}

sub update_header {
    my $file = shift;
    print $File::Find::name . "\n";
    open F, $file or die "Can't open $file: $!";
    my @f = <F>;
    close F;

    my $tmpfile = $file . ".tmp";
    open F, ">", $tmpfile or die "Can't open $tmpfile for writing: $!";
    print F @header;
    print F "\n";
    my $inblock = 0;
    my $wait_for_start = 1;
    my $line = 1;
    my $do_replace = 1;
    foreach (@f) {
        if ($line < 15 and (not $inblock) and ($_ =~ m/auto-generated|copyright|(\(c\))/i)) {
            print "  * Copyright or auto-generated marker found in $file:$line, skipping this file\n";
            $do_replace = 0;
            last;
        }
        if ($line < 5 and not $inblock) {
            if ($_ =~ m!/\*{30,}!) {
              $inblock = 1;
              $wait_for_start = 0;
            }
        }
        if ($wait_for_start && $_ !~ m/^\s*$/) {
            $wait_for_start = 0;
        }
        print F $_ if not ($inblock or $wait_for_start);
        $line++;

        if ($inblock) {
          if ($_ =~ m!\*{30,}/!) {
                $inblock = 0;
                $wait_for_start = 1;
          }
        }
    }
    close F;
    if ($do_replace) {
        move($tmpfile, $file);
    } else {
        unlink($tmpfile);
    }
}
