<?php
// connects to localhost by default
$cluster = Cassandra::cluster ()->withContactPoints('192.168.1.241')->build ();

// Connect to Cassandra Database
$keyspace = 'has';
$session = $cluster->connect ( $keyspace );

/**
 * ****Get Maximum Position******
 */
$statement_getmax_blocks = new Cassandra\SimpleStatement ( 'select system.max(label) from has.mactrain' );
$future = $session->executeAsync ( $statement_getmax_blocks );

$maxposition;
$result = $future->get ();

foreach ( $result as $row ) {
	$maxposition = $row ['system.max(label)'];
}

/**
 * *********Get position count********
 */
$nowtime = time ();

/* Get Count since last one minute */
$timeoneminutebefore = $nowtime - 60;
$datelastminute = ( string ) date ( 'Y-m-d H:i:s', $timeoneminutebefore );

$position = 1;

while ( $position <= $maxposition ) {
	$query = "SELECT COUNT(*) AS position FROM has.macrecord where position = $position and record = 1 and timestamp < dateof(now()) and timestamp > '". $datelastminute."' allow filtering";
	$statement = new Cassandra\SimpleStatement ( $query );
	$future = $session->executeAsync ( $statement );
	$result = $future->get ();
	foreach ( $result as $row ) {
		$arr = array (
				$position => ( string ) $row ['position']
		);
		$output [] = $arr;
	}
	$position ++;
}

if ($output == null) {
	$arr [] = array (
			'Position' => "NULL"
	);
	$output [] = $arr;
}

print (json_encode ( $output )) ;
