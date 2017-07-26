#!/usr/bin/perl

# copy main.perl
$i = 1;
$gate_no = 5;
for($i; $i <= $gate_no; $i++){
	$cpy_main = "scp ./main.perl root\@board$i:/home/ubuntu/DIOPS_DSVM/";
	print "$cpy_main\n\n";
	system("$cpy_main");
}

# copy sniffer_predict.perl
$i = 1;
for($i; $i <= $gate_no; $i++){
	$cpy_sniffer = "scp ./sniffer_predict.perl root\@board$i:/home/ubuntu/DIOPS_DSVM/";
	print "$cpy_sniffer\n\n";
	system("$cpy_sniffer");
}
