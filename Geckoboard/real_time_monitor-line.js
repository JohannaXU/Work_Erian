var API_KEY = '2d8234836d81e3d7530cece45a8a07e2';


var gb = require('geckoboard')(API_KEY);
var request = require('request');
var sql = require("seriate");
var moment = require('moment');
var d3 = require("d3");
var cron = require('node-cron');

var num = 0;
	
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

var geckoboard_url_total_number = "https://push.geckoboard.com/v1/send/164421-0fa1f5b0-12c0-0135-6dbc-22000b498417";

/*-------------------------------------------- */
//cron.schedule('00 00 * * *', function(){

var num_record = [];
var count = 0;
var half_hour_amount = 0;
var half_hour_average = 0;

var requestLoop = setInterval(function(){

	count = count + 1;

request({
    url: url,
    json: true
}, function (error, response, body) {
	if (!error && response.statusCode === 200) {
		num = Number(body.Occupancy0223);
		console.log(num); // Print the json response

		half_hour_amount = half_hour_amount + num;

		if (count == 30)
		{
			half_hour_average = Math.round(half_hour_amount/30);
			num_record.push(half_hour_average);
			half_hour_amount = 0;
			count = 0;
		}
		console.log(num_record);
    }

/* The NUMBER display*/	
var occupancy_num = {
	"api_key": API_KEY,
	"data": {
     "item": [
        { "text": "Occupancy Number", "value": num},
        num_record
		]
  }
}

geckoPush(geckoboard_url_total_number,occupancy_num);

});
}, 60000);

//});





