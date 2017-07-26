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
	mysqli_free_result ( $result );
}

if (isset ( $_REQUEST ['MACAdd'] )) {
	$_MACAdd = $_REQUEST ['MACAdd'];
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
	$query_select_rssi_fields = $query_select_rssi_fields . " FROM MACRecord where MACAdd = '$_MACAdd'";
	//echo $query_select_rssi_fields;
	if ($result = mysqli_query ( $conn, $query_select_rssi_fields )) {
		
		while ( $row = mysqli_fetch_assoc ( $result ) ) {
			
			$output [] = $row;
		}
	}
	
	// $query = ("SELECT RSSI1,RSSI2,RSSI3,RSSI4,RSSI5 FROM MACRecord where MACAdd='$_MACAdd'");
}

if ($output == null) {
	// $arr [] = array (
	// 'RSSI1' => "null",
	// 'RSSI2' => "null",
	// 'RSSI3' => "null",
	// 'RSSI4' => "null",
	// 'RSSI5' => "null"
	// );
	
	// print (json_encode ( $arr )) ;
	$arr = array ();
	for($j = 1; $j <= $total_rssi_fields; $j ++) {
		$arr ["RSSI$j"] = "null";
	}
	$output [] = $arr;
}

print (json_encode ( $output )) ;

mysqli_close ( $conn );

?>
