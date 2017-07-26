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
				'mactrain'
		)
) ) );
$total_rssi_fields;
$result = $future->get ();

foreach ( $result as $row ) {
	$total_rssi_fields = $row ['count'] - 4;
}

if (isset ( $_REQUEST ['ID'] )) {
	$_ID = $_REQUEST ['ID'];
	
	$query_select_rssi_fields = "SELECT ";
	for($i = 1; $i <= $total_rssi_fields; $i ++) {
		$query_select_rssi_fields = $query_select_rssi_fields . "RSSI$i";
		if ($i < $total_rssi_fields) {
			$query_select_rssi_fields = $query_select_rssi_fields . ",";
		}
	}
	$query_select_rssi_fields = $query_select_rssi_fields . " FROM has.mactrain where ID = " . $_ID." allow filtering";
	
	//echo  $query_select_rssi_fields;
	$statement = new Cassandra\SimpleStatement ($query_select_rssi_fields);
	
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	
	foreach ( $result as $row ) {
		for($j = 1; $j <= $total_rssi_fields; $j ++) {
			$arr ["RSSI$j"] = ( string ) round ( $row ['rssi' . $j], 0 );
		}
		$output [] = $arr;
	}
	
}

if ($output == null) {
	$arr = array ();
	for($j = 1; $j <= $total_rssi_fields; $j ++) {
		$arr ["RSSI$j"] = "null";
	}
	$output [] = $arr;
}

echo json_encode ( $output );