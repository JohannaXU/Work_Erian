<?php
$connection = mysql_connect('localhost', 'root', 'root');
	
	if (!$connection){
    die("Database Connection Failed" . mysql_error());
}
$select_db = mysql_select_db('HAS');

if (!$select_db){
    die("Database Selection Failed" . mysql_error());
}

$query_total_rssi_fields = "select count(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name ='MACTrain' AND column_name LIKE '%RSSI%'" ;

$result = mysql_query($query_total_rssi_fields) or die('Errant query');

if(is_resource($result) and mysql_num_rows($result)>0){
	$row = mysql_fetch_array($result);
	$total_rssi_fields = $row["count(*)"];
}
	

	
for($j =1; $j<=$total_rssi_fields;$j++)
{
	$query = "DELETE FROM MACTrain WHERE RSSI"."$j"." IS NULL";
	$result = mysql_query($query) or die('Errant query');
}

$return_count = "SELECT COUNT(*) FROM MACTrain";
$return_count_result  = mysql_query($return_count) or die('Errant query');
	
while($row = mysql_fetch_assoc($return_count_result))
{
	$output [] = $row;
}
	
	

print(json_encode($output));
mysql_close($connection);

?>
