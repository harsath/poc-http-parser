#!/bin/perl
use warnings;
use strict;

if($#ARGV+1 != 1){ print "Usage: ./script.pl <BUILD-DIR-NAME>"; exit(1); }
my $build_dir_name = $ARGV[0];
sub run_tests {
	if(-d $build_dir_name){ print "[ERROR]: Build dir already exists, remove first"; exit(1); }
	mkdir($build_dir_name);
	chdir($build_dir_name);
	system(qq/CC=gcc-9 && CXX=g++-9 cmake -GNinja .. && ninja/);
	system(qq/.\/poc_http_parser_tests/);
}

run_tests();
