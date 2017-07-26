#!/usr/bin/perl

$sniffer_predict = "./bin/Sniffer_predict -n 5 -s 192.168.0.116";

system("sudo cpufreq-set -g performance");

system("sudo ifconfig wlan0 down");
system("sudo iwconfig wlan0 mode monitor");
system("sudo ifconfig wlan0 up");


while(1)
{
	printf "$sniffer_predict\n";
	system("$sniffer_predict");
}
