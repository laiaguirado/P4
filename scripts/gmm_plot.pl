#!/usr/bin/perl -w

if (@ARGV  == 1 && $ARGV[0] =~ /-h/) {
    print <<EOF;
$0 [gmmfile.txt] > gmmplot.cmd

Execute this script to create a gnuplot command file.
You can edit and modify the output file and then execute
from the gnuplot console, 
call  gmmplot.cmd
EOF
exit;
}

$c = 1.0;

$max_x = $max_y = -1e30;
$min_x = $min_y = 1e30;

while (<>) {
    # Look for lines with mean (mu)
    next if (! /^mu.*=([-.0-9]+)\t([-.0-9]+)/);

    $mu_x = $1; $mu_y = $2;
    $_ = <>; # read next line
        
    if (! /^sig.*=([-.0-9]+)\t([-.0-9]+)/) {
	print STDERR "Format error: sigma not found after mu, line $.\n";
	exit 1;
    }
    $sig_x = $1; $sig_y = $2;
    push @cmds, "plot $mu_x + $c * $sig_x * cos(t), $mu_y + $c * $sig_y * sin(t) title '' ";	

    $v = $mu_x + 2 * $c * $sig_x;
    $max_x = $v if $v > $max_x;
    $v = $mu_x - 2 * $c * $sig_x;
    $min_x = $v if $v < $min_x;

    $v = $mu_y + 2 * $c * $sig_y;
    $max_y = $v if $v > $max_y;
    $v = $mu_y - 2 * $c * $sig_y;
    $min_y = $v if $v < $min_y;

} 


print<<EOF;
set multiplot
set xrange [$min_x:$max_x]
set yrange [$min_y:$max_y]
#plot "yourfile.dat"
set parametric
set trange [0:2*pi]
EOF

foreach $cmd (@cmds) {
    print "$cmd\n";
}


print "unset parametric";
exit 0;

