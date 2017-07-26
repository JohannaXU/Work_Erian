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

if (isset ( $_REQUEST ['ID'] )) {
	$_ID = $_REQUEST ['ID'];
	$query_total_rssi_fields = "select count(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name ='MACTrain' AND column_name LIKE '%RSSI%'";
	
	$result = mysqli_query ( $conn, $query_total_rssi_fields ) or die ( 'Errant query' );
	$row = mysqli_fetch_array ( $result );
	$total_rssi_fields = $row ["count(*)"];
	$query_select_rssi_fields = "SELECT ";
	for($i = 1; $i <= $total_rssi_fields; $i ++) {
		$query_select_rssi_fields = $query_select_rssi_fields . "RSSI$i";
		if ($i < $total_rssi_fields) {
			$query_select_rssi_fields = $query_select_rssi_fields . ",";
		}
	}
	$query_select_rssi_fields = $query_select_rssi_fields . " FROM MACTrain where ID = " . $_ID;
	if ($result = mysqli_query ( $conn, $query_select_rssi_fields )) {
		
		while ( $row = mysqli_fetch_assoc ( $result ) ) {
			
			$output [] = $row;
		}
	}
}

if ($output == null) {
	for($j = 1; $j <= $total_rssi_fields; $j ++) {
		$arr ["RSSI$j"] = "null";
	}
	$output [] = $arr;
	print (json_encode ( $arr )) ;
} else {
	
	print (json_encode ( $output )) ;
}

mysqli_close ( $conn );
?>
