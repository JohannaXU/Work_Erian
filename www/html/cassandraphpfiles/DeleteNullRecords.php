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

/**
 * *********Select All Records***************
 */
$query_select_rssi_fields = "SELECT block,id,";

for($i = 1; $i <= $total_rssi_fields; $i ++) {
	$query_select_rssi_fields = $query_select_rssi_fields . "RSSI$i";
	if ($i < $total_rssi_fields) {
		$query_select_rssi_fields = $query_select_rssi_fields . ",";
	}
}

$query_select_rssi_fields = $query_select_rssi_fields . " FROM HAS.MACTrain";

// echo $query_select_rssi_fields;

/**
 * *********Delete All NuLL RSSI Records***************
 */
$statement_select_rssi = new Cassandra\SimpleStatement ( $query_select_rssi_fields );

$future = $session->executeAsync ( $statement_select_rssi );
$result = $future->get();


foreach ( $result as $row ) {
	for($j = 1; $j <= $total_rssi_fields; $j ++) {
		if ($row ['rssi' . $j] == null) {
			$deletequery = "DELETE FROM HAS.MACTrain where block = " . $row ['block'] . " and id = " . $row ['id'];			
			$statement_deletequery = new Cassandra\SimpleStatement($deletequery);
			$session->executeAsync($statement_deletequery);
		}
	}
}

/**
 * *********Return Count of HAS.MACTrain***************
 */
$query_selectcount = "SELECT COUNT(*) FROM HAS.MACTrain";
$statement_returncount = new Cassandra\SimpleStatement ( $query_selectcount );
$future = $session->executeAsync ( $statement_returncount );
$result_count = $future->get ();

foreach ( $result_count as $row ) {
	$count = $row ['count'];
}

$arr = array (
		'COUNT(*)' => ( string ) $count 
);
$output [] = $arr;
echo json_encode ( $output );


