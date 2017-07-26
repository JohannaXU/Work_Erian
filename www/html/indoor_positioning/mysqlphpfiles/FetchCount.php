<?php
$conn = mysqli_connect("localhost", "root", "root", "HAS");

/* check connection */
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}

/* return name of current default database */
if ($result = mysqli_query($conn, "SELECT DATABASE()")) {
    $row = mysqli_fetch_row($result);
   // printf("Default database is %s.\n", $row[0]);
    mysqli_free_result($result);
}

//$_ID = $_REQUEST['ID'];
//$query = ("SELECT Position FROM MACRecord where MACAdd = '38:EC:E4:6F:F5:B8' ");

if(isset($_REQUEST['Block']))
{
	$query_total_rssi_fields = "select count(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name ='MACTrain' AND column_name LIKE '%RSSI%'" ;
	
	if($result = mysqli_query($conn,$query_total_rssi_fields)){
		$row = $result->fetch_array();;
		$total_rssi_fields = $row[0];
	}
	
	$_Block = $_REQUEST['Block'];
	$query = "SELECT COUNT(*) FROM MACTrain WHERE "; 
	for($i =1; $i<$total_rssi_fields;$i++)
	{
		$query = $query."RSSI$i and ";
	}
	$query = $query."RSSI$i IS NOT NULL AND Block = '$_Block' ";
	
	//$query = ("SELECT COUNT(*) FROM MACTrain WHERE Block = '$_Block' ");
}
else
{
	printf("Please identify Block No.");
    exit();
}
//echo $query;



if($result = mysqli_query($conn,$query)) {
	while ($row = mysqli_fetch_assoc($result)) 
	{
		$output[]=$row;
	}
}
print(json_encode($output));


mysqli_close($conn);

?>
