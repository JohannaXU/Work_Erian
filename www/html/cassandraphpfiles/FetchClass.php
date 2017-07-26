<?php

// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints ( '192.168.1.241' )->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect ( $keyspace );

/**
 * ****Get total Number of blocks******
 */
$statement_getmax_blocks = new Cassandra\SimpleStatement ( 'select system.max(block) from has.mactrain' );
$future = $session->executeAsync ( $statement_getmax_blocks );

$total_numberblocks;
$result = $future->get ();

foreach ( $result as $row ) {
	$total_numberblocks = $row ['system.max(block)'];
}

$i = 1;
$query = ("SELECT Block,LABEL FROM MACTrain where block = %d and id = %d");

/**
 * ****Get Block, Label for all the blocks******
 */
while ( $i <= $total_numberblocks ) {
	$query_select_block_label = sprintf ( $query, $i, ($i * 10000 + 1) );
	$statement = new Cassandra\SimpleStatement ( $query_select_block_label );
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	foreach ( $result as $row ) {
		
		$arr = array (
				'Block' => ( string ) $row ['block'],
				'LABEL' => ( string ) $row ['label'] 
		);
		$output [] = $arr;
	}
	$i ++;
	echo "\n";
}

if ($output == null) {
	$arr = array (
			'Block' => "NULL",
			'LABEL' => "NULL" 
	);
	$output [] = $arr;
}

echo json_encode ( $output );