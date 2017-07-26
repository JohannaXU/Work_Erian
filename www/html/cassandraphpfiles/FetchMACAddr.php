<?php
// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints('192.168.1.241')->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect ( $keyspace );

$nowtime = time ();

/* Get macadd since last one minute */
$timeoneminutebefore = $nowtime - 60;
$datelastminute = ( string ) date ( 'Y-m-d H:i:s', $timeoneminutebefore );
if (isset ( $_REQUEST ['Position'] )) {
	$_Position = $_REQUEST ['Position'];
	$query = ("SELECT macadd FROM has.macrecord where position = $_Position and record = 1 and timestamp < dateof(now()) and timestamp >'$datelastminute' allow filtering ");
	//echo $query;
	$statement = new Cassandra\SimpleStatement ( $query );
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	foreach ( $result as $row ) {
		$arr = array (
				"MACAdd" => ( string ) $row ['macadd'] 
		);
		$output [] = $arr;
	}
}

if ($output == null) {
	$arr [] = array (
			'MACAdd' => "No Records" 
	);
	print (json_encode ( $arr )) ;
} else {
	
	print (json_encode ( $output )) ;
}
