<?php
// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints('192.168.1.241')->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect($keyspace);

if(isset($_REQUEST['ID']))
{
	$_ID = $_REQUEST['ID'];
	$query = (" SELECT LABEL FROM HAS.MACTrain where ID = $_ID allow filtering");
	$statement = new Cassandra\SimpleStatement ($query);
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	foreach ( $result as $row ) {
		$arr = array (
				'LABEL' => ( string ) $row ['label']
		);
		$output [] = $arr;
	}
}

if($output == null)
{
	$arr[] = array('LABEL' => "NULL");
	$output [] = $arr;
}

echo json_encode ($output);