var API_KEY = '2d8234836d81e3d7530cece45a8a07e2';

var gb = require('geckoboard')(API_KEY);
var request = require('request');
var moment = require('moment');
var d3 = require('d3');
var mysql = require('mysql');
var dataset =  require('dataset');
var cron = require('node-cron');

//var raw_data = Array();


cron.schedule('*/15 * * * *', function(){
	
var raw_data = Array();

var connection = mysql.createConnection({
	host	: '192.168.0.116',
	user	: 'root',
	password: 'root',
	database: 'HAS'
});

connection.connect();

var DATA = connection.query('select MOMENT, TOTAL from RecordList where TIMESTAMPDIFF(minute, MOMENT, now())<15', function(error, results, fields){
	console.log(results);
	
	var array = {moment: null, total: 0 };
	var i = 0;

	results.forEach(function(item){
		
		i = i + 1;
		item.MOMENT.setHours(item.MOMENT.getHours()+8);
		//console.log(item.MOMENT);
		
		array.moment= item.MOMENT;
		array.total= array.total + item.TOTAL;
		
	});
	array.total = Math.ceil(array.total / i);
	//console.log(array);

	raw_data.push(array);
	
	console.log(raw_data);
});

connection.end();



gb.datasets.findOrCreate(
	{
		id: 'smeswp4_db2',
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
		
			console.log('Dataset Found');
	
		
			dataset.post( 		
				raw_data,			
				{delete_by: 'moment'},
				function(err){
					if(err){
						console.error(err);
						return;
						}
			console.log('Data append');
			}
		);
	}
);

});