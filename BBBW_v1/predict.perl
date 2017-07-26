#!/usr/bin/perl

$row = $ARGV[0];
$col = $ARGV[1];
$num_args = $#ARGV +1;

if ($num_args != 2) {
	die "Must enter number of rows & columns:\n\t\tFor example: ./predict.perl 4[space_row] 12[space_column]\n";
}else{
	$run_predict = "./bin/svm-predict ./data/testfile ./model/trainfile.model ./output/output";
	print "$run_predict\n";
	system("$run_predict");

	print "\t\tblocks\taccuracy\n";
	$distance_max=6;
	for($i=0;$i<=$distance_max;$i++)
	{
		$accuracy_analysis_run = "./bin/accuracy-analysis -d $i -r $row -c $col ./output/output";
		#print "\t$accuracy_analysis_run\n";
		system("$accuracy_analysis_run");
	}
}
