<?php
// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints('192.168.1.241')->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect ( $keyspace );

if (isset ( $_REQUEST ['Block'] )) {
	$_Block = $_REQUEST ['Block'];
	$query = "SELECT COUNT(*) FROM HAS.MACTrain WHERE block = %d";
	$query = sprintf ( $query, $_Block );
	$statement = new Cassandra\SimpleStatement ( $query );
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	foreach ( $result as $row ) {
		$arr = array (
				'COUNT(*)' => ( string ) $row ['count'] 
		);
		$output [] = $arr;
	}
}
else
{
	printf("Please identify Block No.");
	exit();
}

echo json_encode ( $output );