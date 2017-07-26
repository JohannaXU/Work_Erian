#!/usr/bin/perl

$file = "./output/host_file";

$gate_no = $ARGV[0];
$train_file = $ARGV[1];

$num_args = $#ARGV +1;
if ($num_args != 2) {
	die "Must enter number of gates:\n\t\tFor example: ./run.perl 5 trainfile\n";
}else{
	# Use the open() function to create the file.
	unless(open FILE, '>'.$file) {
		# Die with error message 
		# if we can't open it.
		die "\nUnable to create $file\n";
	}
	$i = 1;
	# Write text to the file.
	for($i; $i <= $gate_no; $i++)
	{
		print FILE "board$i\n";
		#print FILE "ubuntu\n";
	}
	
	$i = 1;
	# copy model
	for($i; $i <= $gate_no; $i++){
		$cpy_host = "scp ./output/host_file root\@board$i:/home/ubuntu/DIOPS_DSVM/output";
		print "$cpy_host\n\n";
		system("$cpy_host");
	}

	# close the file.
	close FILE;
	$run = "mpirun -np $gate_no --hostfile $file ./bin/mpi-svm-train -s 0 -t 0 -d 3 -g 0.0 -r 0.0 -n 0.5 -m 40.0 -c 1.0 -e 0.001 -p 0.1 -G $gate_no ./data/$train_file ./model/$train_file.model";
	print "$run\n";
	system("$run");
	
	$i = 2;
	# copy model
	for($i; $i <= $gate_no; $i++){
		$cpy_model = "scp -r ./model root\@board$i:/home/ubuntu/DIOPS_DSVM/";
		print "$cpy_model\n\n";
		system("$cpy_model");
	}
}
