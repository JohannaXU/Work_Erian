<?php
// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints ( '192.168.1.241' )->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect ( $keyspace );

if (isset ( $_REQUEST ['MACAdd'] )) {
	$MACAdd = $_REQUEST ['MACAdd'];
	$query = "SELECT Position From has.macrecord WHERE MACAdd = '$MACAdd'";
	$statement_select_position = new Cassandra\SimpleStatement ( $query );
	$future = $session->executeAsync ( $statement_select_position );
	$position;
	$result = $future->get ();
	
	foreach ( $result as $row ) {
		if ($row ['position'] == "") {
			$arr = array (
					'Position' => ( string ) $row ['null'] 
			);
		} else {
			$arr = array (
					'Position' => ( string ) $row ['position'] 
			);
			$output [] = $arr;
		}
	}
}

if ($output == null) {
	
	$arr = array (
			'Position' => 'null' 
	);
	$output [] = $arr;
}

echo json_encode ( $output );

