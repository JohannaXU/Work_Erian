<?php
// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints('192.168.1.241')->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect ( $keyspace );

/**
 * ********* Get Total Number of RSSI Fields****
 */
$statement_rssi_count = new Cassandra\SimpleStatement ( 'select count(*) from system.schema_columns where keyspace_name = ? and columnfamily_name = ?' );

$future = $session->executeAsync ( $statement_rssi_count, new Cassandra\ExecutionOptions ( array (
		'arguments' => array (
				$keyspace,
				'macrecord' 
		) 
) ) );
$total_rssi_fields;
$result = $future->get ();

foreach ( $result as $row ) {
	$total_rssi_fields = $row ['count'] - 4;
}

// echo $total_rssi_fields;

if (isset ( $_REQUEST ['MACAdd'] )) {
	$_MACAdd = $_REQUEST ['MACAdd'];
	$query = "SELECT ";
	for($i = 1; $i <= $total_rssi_fields; $i ++) {
		$query .= "RSSI$i";
		if ($i < $total_rssi_fields) {
			$query .= ",";
		}
		if ($i == $total_rssi_fields) {
			$query .= " from has.macrecord where MACAdd='$_MACAdd'";
		}
	}
}
// echo $query;

$statement_rssi_count = new Cassandra\SimpleStatement ( $query );
$future = $session->executeAsync ( $statement_rssi_count );
$result = $future->get ();
foreach ( $result as $row ) {
	$arr = array ();
	for($j = 1; $j <= $total_rssi_fields; $j ++) {
		$arr ["RSSI$j"] = ( string ) round ( $row ['rssi' . $j], 0 );
	}
	$output [] = $arr;
}


if ($output == null) {
	$arr = array ();
	for($j = 1; $j <= $total_rssi_fields; $j ++) {
		$arr ["RSSI$j"] = "null";
	}
	$output [] = $arr;
}

echo json_encode ( $output );