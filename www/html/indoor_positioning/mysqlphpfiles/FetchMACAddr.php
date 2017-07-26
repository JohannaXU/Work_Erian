<?php
$conn = mysqli_connect ( "localhost", "root", "root", "HAS" );

/* check connection */
if (mysqli_connect_errno ()) {
	printf ( "Connect failed: %s\n", mysqli_connect_error () );
	exit ();
}

/* return name of current default database */
if ($result = mysqli_query ( $conn, "SELECT DATABASE()" )) {
	$row = mysqli_fetch_row ( $result );
	// printf("Default database is %s.\n", $row[0]);
	mysqli_free_result ( $result );
}

// $_ID = $_REQUEST['ID'];
// $query = ("SELECT Position FROM MACRecord where MACAdd = '38:EC:E4:6F:F5:B8' ");

if (isset ( $_REQUEST ['Position'] )) {
	$_Position = $_REQUEST ['Position'];
	// $query = ("SELECT MACAdd FROM MACRecord WHERE Position = '$_Position' AND Record < 25");
	// $query = ("SELECT MACAdd FROM MACRecord WHERE Position = '$_Position' AND Record < 25 AND timestampdiff(second, Timestamp, now()) < 100");
	$query = ("SELECT MACAdd FROM MACRecord WHERE Position = '$_Position' AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 1");
}
// echo $query;

if ($result = mysqli_query ( $conn, $query )) {
	
	while ( $row = mysqli_fetch_assoc ( $result ) ) {
		$output [] = $row;
	}
}

if ($output == null) {
	$arr [] = array (
			'MACAdd' => "No Records" 
	);
	// $output[] = '{"RSSI1":"NULL","RSSI2":"NULL","RSSI3":"NULL","RSSI4":"NULL"}';
	print (json_encode ( $arr )) ;
} else {
	
	print (json_encode ( $output )) ;
}

mysqli_close ( $conn );

?>
