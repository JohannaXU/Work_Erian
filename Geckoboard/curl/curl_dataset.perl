#!/usr/bin/perl

$run_python = "curl http://192.168.0.116/html/indoor_positioning/mysqlphpfiles/curl_dataset.php";
#$run_python = "curl http://192.168.0.102/indoor_positioning/mysqlphpfiles/curl_dataset.php";

$mysql_data = system("$run_python");

printf ("\n");

$curl_dataset = "curl https://api.geckoboard.com/datasets/smeswp4/data -X POST -u '2d8234836d81e3d7530cece45a8a07e2:' -H 'Content-Type: application/json' -d '{\"data\": [$output]}'";

printf("$curl_dataset");
system("$curl_dataset");



