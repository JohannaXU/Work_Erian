<?php
$cluster = Cassandra::cluster ()->withContactPoints('192.168.1.241')-> build ();// connects to localhost by default

$keyspace = 'has';
$session = $cluster->connect ( $keyspace ); // create session, optionally scoped to a keyspace

if (isset ( $_REQUEST ['Position'] )) {
	$position = $_REQUEST ['Position'];
	$nowtime = time ();
	
	/*Get Count since last one minute*/
	$timeoneminutebefore = $nowtime - 60;
	
	$datelastminute = (string) date ( 'Y-m-d H:i:s', $timeoneminutebefore );
	
	$query = "SELECT COUNT(*) FROM HAS.MACRecord where position = ".$position."  and record = 1 and timestamp < dateof(now()) and timestamp > '$datelastminute' allow filtering";
	//echo $query;	
	$statement = new Cassandra\SimpleStatement ($query);
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	
	foreach ( $result as $row ) {
		$arr = array (
				'COUNT(*)' => ( string ) $row ['count']
		);
		$output [] = $arr;
	}
	
	print(json_encode($output));
	
}