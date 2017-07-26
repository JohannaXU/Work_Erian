<?php
$mysqli = new mysqli ( "localhost", "root", "root", "HAS" );

/* check connection */
if ($mysqli->connect_errno) {
	printf ( "Connect failed: %s\n", $mysqli->connect_error );
	exit ();
}

/* Select queries return a resultset */
if ($result = mysqli_query ( $mysqli, "SELECT MAX(Block) FROM MACTrain" )) {
	$rows = $result->fetch_array ();
	
	$total_number_of_records = $rows [0];
	// echo $total_number_of_records;
	$total_blocks = substr ( $total_number_of_records, 0, 2 );
	// ~ echo $total_blocks;
}

$i = 1;
$query = ("SELECT Block,LABEL FROM MACTrain where id = %d");

while ( $i <= $total_blocks ) {
	$query_ = sprintf ( $query, ($i * 10000 + 1) );
	// echo $query_;
	// echo "\n";
	$result = mysqli_query ( $mysqli, $query_ );
	// ~ $row = mysqli_fetch_assoc($result);
	// ~ $output[]=$row;
	
	if ($result) {
		$row = mysqli_fetch_assoc ( $result );
		$output [] = $row;
	} else {
		$output [] = array (
				'Block' => "NULL",
				'LABEL' => "NULL" 
		);
	}
	
	$i ++;
}
print (json_encode ( $output )) ;

?>
