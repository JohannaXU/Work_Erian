#!/bin/bash

CURL='/usr/bin/curl'
URL="http://192.168.0.116/html/indoor_positioning/mysqlphpfiles/curl_dataset2_15min.php"


DATA="$($CURL $URL)"

echo "$DATA"

curl_dataset="$(curl https://api.geckoboard.com/datasets/smeswp4_db2/data -X POST -u '2d8234836d81e3d7530cece45a8a07e2:' -H 'Content-Type: application/json' -d '{"data": ['"$DATA"']}')"

echo "$curl_dataset"
