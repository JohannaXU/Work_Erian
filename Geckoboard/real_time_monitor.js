var API_KEY = '2d8234836d81e3d7530cece45a8a07e2';


var gb = require('geckoboard')(API_KEY);
var request = require('request');
var sql = require("seriate");
var moment = require('moment');
var d3 = require("d3");

var num = 0;
var num_record = [];
	
var url = "http://192.168.0.116/html/indoor_positioning/mysqlphpfiles/AmountOfPeople.php";

var geckoPush = function(geckoboard_widget_push_url,input) {
  //console.log(input);
  request.post({url: geckoboard_widget_push_url, json: true, form: JSON.stringify(input)}, 
                function(err, res, body) {
                  if(err) {
                    console.log(err);
                  }
                  console.log(body);
                }
  );
};

var geckoboard_url_total_number = "https://push.geckoboard.com/v1/send/164421-dbc9ca40-129e-0135-dfb2-22000b248df5";

//var geckoboard_url_line_chart = "https://push.geckoboard.com/v1/send/164421-8fbb0390-0bc8-0135-b5d7-22000b4a867a";

var requestLoop = setInterval(function(){
	
request({
    url: url,
    json: true
}, function (error, response, body) {
	if (!error && response.statusCode === 200) {
		num = Number(body.Occupancy0223);
        console.log(num); // Print the json response
    }

/* The NUMBER display*/	
var occupancy_num = {
	"api_key": API_KEY,
	"data": {
     "item": [
        { "text": "Occupancy Number", "value": num},
		]
  }
}

geckoPush(geckoboard_url_total_number,occupancy_num);

});
}, 30000);





