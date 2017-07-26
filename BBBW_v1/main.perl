#!/usr/bin/perl

system("cpufreq-set -f 800000");

#$run_sniffer = "./bin/Sniffer_train -r 6 -c 7 -n 5 -s DataBaseIP -t PadIP -m MacAddress";
$run_sniffer = "./bin/Sniffer_train -r 3 -c 4 -n 5 -s 192.168.0.101 -t 192.168.0.107 -m E0-CB-EE-A4-E7-BB";
#$run_sniffer = "./bin/Sniffer_train -r 3 -c 4 -n 5 -s 192.168.0.102 -t 192.168.0.109 -m E0-CB-EE-A4-E8-85";
print "$run_sniffer\n";
system("$run_sniffer");
