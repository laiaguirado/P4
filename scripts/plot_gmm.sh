#!/bin/bash

if [[ $# < 1 ]]; then
   echo 
   echo "USAGE: $0 file.gmm [fmatrix.dat]"
   echo "Plot 2D gmm (ellipsis) and data (points)"
   echo 
   echo "This script creates the file plot.gp"
   echo "with the gnuplot commands to plot the gmm." 
   echo 
   echo "Check plot.gp, execute gnuplot, and enter the command"
   echo '  call "plot.gp"'
   echo
   exit -1
fi

# This bash script creates the file plot.cmd with 
# the gnuplot commands to show the gmm
#
#-------------------------------------------------------

# Transform gmm file into gnuplot commands (parametric ellipsis)

gmmName=$1; shift

base=/tmp/gmmplot$$
gmm_show $gmmName > $base.gmm_txt ||\
   (echo "Error executing gmm_show -- check path"; exit 1)

perl -ne '
  $c = 1.0;
  $max_x = $max_y = -1e30; $min_x = $min_y = 1e30;

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
    push @cmds, "plot $mu_x + $c * $sig_x * cos(t), $mu_y + $c * $sig_y * sin(t) title \"\" ";	

    # update max/min x/y to set the plot range
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
    set parametric
    set trange [0:2*pi]
  EOF

  foreach $cmd (@cmds) {
    print "$cmd\n";
  }

  print "unset parametric\n";' < $base.gmm_txt > plot.gp
\rm -f $base.gmm_txt

# Plot, with (blue) dots, the input data

if [[ $# > 0 ]]; then
   fmatrix_show -H $* | cut -f 1,2 > fmatdat.txt
   echo "plot \"fmatdat.txt\" with dots ls 3 title \"\"" >> plot.gp
fi 

echo -n "plot.gp: "; ls -l plog.gp
echo  "Execute gnuplot, and enter the command"
echo '  call "plot.gp"'

exit 0
