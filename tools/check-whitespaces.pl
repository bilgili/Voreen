#!/usr/bin/perl
# Checks source files for use of tabs and outputs results in JUnit format for use with Hudson.

use strict;
use warnings;
use File::Find;

my @dirs = qw(src include apps/voreenve apps/simple apps/voltool);
my $interesting = 'cpp|h|frag|vert|cu|cl';
my $skip_dirs = '\.svn|\.moc|\.obj|\.ui';
my $total_tabs = 0;
my @messages = ();

chdir('..');
find({ wanted => \&wanted, no_chdir => 1 }, @dirs);

my $result = "";

if ($total_tabs > 0) {
  $result = qq(<error message="found $total_tabs lines with tabs in source code, use spaces instead!">);
  foreach (@messages) {
      $result .= $_ . "\n";
  }
  $result .= '</error>';
}

    print << "(EOF)";
<testsuite name="CodeTests">
   <testcase classname="Source.Style" name="tabs">
      $result
   </testcase>
</testsuite>
(EOF)

sub wanted {
    if ($_ =~ /^($skip_dirs)$/) {
        $File::Find::prune = 1;
        return;
    };
    if ((-f $_) and ($_ =~ /\.($interesting)$/)) {
        check_tabs($_);
    }
}

sub check_tabs {
    my $file = shift;
    open F, $file or die "Can't open $file: $!";
    my @f = <F>;
    close F;

    my $line = 1;
    my $changed = 0;
    my $tabs = 0;
    my $trailing = 0;

    my $l = '';
    foreach (@f) {
        $l = $_;
        if ($l =~ /\t/) {
            $changed = 1;
            $tabs++;
        }
        $line++;
    }
    close F;

    if ($tabs > 0) {
        $total_tabs += $tabs;
        push @messages, "$file: found $tabs lines with tabs";
    }
}
