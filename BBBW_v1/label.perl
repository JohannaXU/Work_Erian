#!/usr/bin/perl

$gate_no = $ARGV[0];
$row = $ARGV[1];
$column = $ARGV[2];
$database_ip = $ARGV[3];
$num_args = $#ARGV +1;

if ($num_args != 4) {
	die "Must enter number of gates:\n\t\tFor example: ./label.perl 5[gate_num] 4[space_row] 12[space_column] 192.168.1.189[database ip]\n";
}else{
	$run_label = "./bin/label_generate -r $row -c $column -h $database_ip -t 100 -b 1";
	print "$run_label\n\n";
	system("$run_label");
	
	$i = 1;
	# Write text to the file.
	for($i; $i <= $gate_no; $i++){
		$cpy_data = "scp -r ./data root\@board$i:/home/ubuntu/DIOPS_DSVM/";
		print "$cpy_data\n\n";
		system("$cpy_data");
	}
}
