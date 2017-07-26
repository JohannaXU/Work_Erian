#!/usr/bin/perl

print "topping ...\n";
while(1)
{
        system ("top -b -d 1 -n 3 >> top-file.txt");
}
