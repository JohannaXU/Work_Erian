var API_KEY = '2d8234836d81e3d7530cece45a8a07e2';

var gb = require('geckoboard')(API_KEY);
var request = require('request');
var moment = require('moment');
var d3 = require('d3');
var mysql = require('mysql');
var dataset =  require('dataset');
var cron = require('node-cron');

var raw_data = Array();
//var array = {moment: null, total: null };

//cron.schedule('*/5 * * * *', function(){

var connection = mysql.createConnection({
	host	: '192.168.0.116',
	user	: 'root',
	password: 'root',
	database: 'HAS'
});

connection.connect();

var DATA = connection.query('select MOMENT, TOTAL from RecordList where TIMESTAMPDIFF(hour, MOMENT, now())<40', function(error, results, fields){
	//console.log(results);
	
	results.forEach(function(item){
		var array = {moment: null, total: null };
		
		item.MOMENT.setHours(item.MOMENT.getHours()+8);
		//console.log(item.MOMENT);
		
		array.moment=(item.MOMENT);
		array.total=(item.TOTAL);
		//console.log(array);
		
		raw_data.push(array);
	});
	
	//console.log(raw_data);
});

connection.end();



gb.datasets.findOrCreate(
	{
		id: 'smeswp4',
		fields:{
			moment:{
				type: 'datetime',
				name: 'MOMENT'
			},
			total:{
				type: 'number',
				name: 'TOTAL'
				}
		}
		},
		function(err, dataset){
			if(err){
				console.error(err);
				return;
			}
		
			console.log('Dataset created');
	
		
			dataset.put( 			
				raw_data,

				function(err){
					if(err){
						console.error(err);
						return;
						}
			console.log('Dataset created and data added');
			}
		);
	}
);

//});