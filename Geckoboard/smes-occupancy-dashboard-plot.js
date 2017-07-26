var API_KEY = '2d8234836d81e3d7530cece45a8a07e2';

var gb = require('geckoboard')(API_KEY);
var request = require('request');
var moment = require('moment');
var d3 = require('d3');
var mysql = require('mysql');

//var x_array = []
//var y_array = []

var geckoboard_url_line_chart = "https://push.geckoboard.com/v1/send/164421-189f8d00-12a2-0135-85a3-22000abade87";

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


var requestLoop = setInterval(function(){

var x_array = [];
var y_array = [];	
	
var connection = mysql.createConnection({
	host	: '192.168.0.116',
	user	: 'root',
	password: 'root',
	database: 'HAS'
});

connection.connect();

var X_get = connection.query('select MOMENT from RecordList where TIMESTAMPDIFF(hour, MOMENT, now())<48', function(error, X, fields){

	X.forEach(function(item){
		item.MOMENT.setHours(item.MOMENT.getHours()+8);
		x_array.push(item.MOMENT)
	});
	//console.log(x_array);
});

var Y_get = connection.query('select TOTAL from RecordList where TIMESTAMPDIFF(hour, MOMENT, now())<48', function(error, Y, fields){

	Y.forEach(function(item){
		y_array.push(item.TOTAL)
	});
		//console.log(y_array);
	
	var number_record = {
	"api_key":	API_KEY,
	"data":{
		"x_axis":{
			type: "datetime",
			labels: x_array
		},

		"series": [{
			"name": "Device Number",
			"data": y_array		
			}]
		}		
	};

	geckoPush(geckoboard_url_line_chart, number_record);
		
});

connection.end();

}, 300000);







	









	
  
  