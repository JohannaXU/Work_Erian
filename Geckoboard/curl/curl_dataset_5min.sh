#!/bin/bash

CURL='/usr/bin/curl'
URL="http://192.168.0.116/html/indoor_positioning/mysqlphpfiles/curl_dataset.php"


DATA="$($CURL $URL)"

#echo "$DATA"

curl_dataset="$(curl https://api.geckoboard.com/datasets/smeswp4/data -X POST -u '2d8234836d81e3d7530cece45a8a07e2:' -H 'Content-Type: application/json' -d '{"data": ['"$DATA"']}')"

#curl_dataset="$(curl https://api.geckoboard.com/datasets/smeswp4/data -X POST -u '2d8234836d81e3d7530cece45a8a07e2:' -H 'Content-Type: application/json' -d '{"data": [{"moment": "2017-05-03T01:55:00Z", "total": 19}]}')"

#echo "$curl_dataset"
