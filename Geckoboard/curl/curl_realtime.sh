#!/bin/bash

CURL='/usr/bin/curl'
URL="http://192.168.0.116/html/indoor_positioning/mysqlphpfiles/AmountOfPeople_v1.php"


DATA="$($CURL $URL)"

echo "$DATA"

curl_dataset="$(curl -X POST https://push.geckoboard.com/v1/send/164421-f16072f0-16b9-0135-89d1-22000abade87 -d '{"api_key": "2d8234836d81e3d7530cece45a8a07e2", "data":{"item":[{"value":'"$DATA"', "text": "Occupancy Number"}]}}' -H "Content-Type:application/json")"

#curl_dataset="$(curl https://api.geckoboard.com/datasets/smeswp4/data -X POST -u '2d8234836d81e3d7530cece45a8a07e2:' -H 'Content-Type: application/json' -d '{"data": ['"$DATA"']}')"

#curl_dataset="$(curl https://api.geckoboard.com/datasets/smeswp4/data -X POST -u '2d8234836d81e3d7530cece45a8a07e2:' -H 'Content-Type: application/json' -d '{"data": [{"moment": "2017-05-03T01:55:00Z", "total": 19}]}')"

echo "$curl_dataset"
