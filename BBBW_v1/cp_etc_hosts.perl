#!/usr/bin/perl

# copy /etc/hosts
$i = 1;
$gate_no = 5;
for($i; $i <= $gate_no; $i++){
	$cpy_hosts = "scp /etc/hosts root\@board$i:/etc";
	print "$cpy_hosts\n\n";
	system("$cpy_hosts");
}


