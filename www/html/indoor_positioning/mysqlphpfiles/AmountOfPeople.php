<?php
$conn = mysqli_connect("localhost", "root", "root", "HAS");

/* Check Connection */
if (mysqli_connect_errno())
{
	printf("Connect failed: %s\n", mysqli_connect_error());
	exit();
}

/* return name of current default database */
if ($result = mysqli_query($conn, "SELECT DATABASE()"))
{
	$row = mysqli_fetch_row($result);
	mysqli_free_result($result);
}

/* Select queries return a resultset */
if ($result = mysqli_query ($conn, "select count(distinct label)  from MACTrain" )) {
	$rows = $result->fetch_array ();

	$total_number_of_records = $rows [0];
	// echo $total_number_of_records;
	$maxlabel = substr ( $total_number_of_records, 0, 2 );
	// ~ echo $maxposition;
}

function change_key($array, $oldkey, $newkey)
{
	if(!array_key_exists($oldkey, $array))
		return $array;
	$keys = array_keys($array);
	$keys[array_search($oldkey, $keys)] = $newkey;

	return array_combine($keys, $array);
}

$oldkey = 'COUNT(*)';
$newkey = 'Occupancy0223';

$query = "SELECT COUNT(*) FROM MACRecord WHERE Position is not NULL AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 5"; 
if($result = mysqli_query($conn, $query))
{
	if($row = mysqli_fetch_assoc($result))
	{
		$output=change_key($row, $oldkey, $newkey);
	}
}

$Position=1;
while ($Position <= $maxlabel)
{
	$query = "SELECT COUNT(*) AS'$Position'"."FROM MACRecord WHERE Position = '$Position' AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 5"; 
	//$query = "SELECT COUNT(*) AS'$Position'"."FROM MACRecord WHERE Position = '$Position' AND Record < 25"; 
	$oldkey = strval($Position);
//	echo $oldkey;
	$newkey = 'Cubicle'.strval($Position);
//	echo $newkey;
    if($result = mysqli_query($conn,$query)) 
    {
		while ($row = mysqli_fetch_assoc($result)) 
		{	
			$row=change_key($row, $oldkey, $newkey);
			$output= $output + $row;
		}				
	}
	
	$Position++;
}
printf (json_encode($output));

mysqli_close($conn);

?>




