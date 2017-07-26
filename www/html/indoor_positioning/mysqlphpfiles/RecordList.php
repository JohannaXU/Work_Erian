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

$query_1 = "INSERT INTO RecordList (MOMENT) VALUES (NOW()) ";
mysqli_query($conn, $query_1);


$query = "SELECT COUNT(*) FROM MACRecord WHERE Position is not NULL AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 5"; 
//$query = "SELECT COUNT(*) FROM MACRecord WHERE Position is not NULL AND Record < 25"; 
if($result = mysqli_query($conn, $query))
{
	if($row = mysqli_fetch_assoc($result))
	{
		printf (json_encode($row));
		//$query_1 = "INSERT INTO RecordList (TOTAL_COUNT) VALUES ('$row')";
		$value = current($row);
		$query_2 = "UPDATE RecordList SET TOTAL = $value WHERE TIMESTAMPDIFF(minute, MOMENT, now()) = 0";
		mysqli_query($conn, $query_2);
	}
}

$Position=1;
$column = 'BLOCK';
while ($Position <= $maxlabel)
{
	$query = "SELECT COUNT(*) AS'$Position'"."FROM MACRecord WHERE Position = '$Position' AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 5"; 
	//$query = "SELECT COUNT(*) AS'$Position'"."FROM MACRecord WHERE Position = '$Position' AND Record < 25"; 
		
    if($result = mysqli_query($conn,$query)) 
    {
		while ($row = mysqli_fetch_assoc($result)) 
		{	
			printf (json_encode($row));
			$column_name = $column.$Position;
			//printf (json_encode($column_name));
			$value = current($row);
			$query_3 = "UPDATE RecordList SET $column_name = $value WHERE TIMESTAMPDIFF(minute, MOMENT, now()) = 0";
			mysqli_query($conn, $query_3);
		}				
	}
	
	$Position++;
}

echo "Finished";

mysqli_close($conn);
?>

