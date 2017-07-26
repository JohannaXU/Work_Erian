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

/* Select queries return a resultset */
if ($result = mysqli_query ($conn, "select count(distinct label)  from MACTrain" )) {
	$rows = $result->fetch_array ();

	$total_number_of_records = $rows [0];
	// echo $total_number_of_records;
	$maxlabel = substr ( $total_number_of_records, 0, 2 );
	// ~ echo $maxposition;
}




$Position=1;
//while ($Position <=$maxlabel)
while ($Position <= 12) /*only for CleanTech Office 0223*/
{
	$query = "SELECT COUNT(*) AS'$Position'"."FROM MACRecord WHERE Position = '$Position' AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 5"; 
	//$query = "SELECT COUNT(*) AS'$Position'"."FROM MACRecord WHERE Position = '$Position' AND Record < 25"; 
		
    if($result = mysqli_query($conn,$query)) 
    {

		while ($row = mysqli_fetch_assoc($result)) 
		{
			
		$output[]=$row;
		
		}		
	}
	else
	{
	  
		$output[] = array($Position=>"NULL");		
	}
	
	$Position++;
    
}
print (json_encode($output));

mysqli_close($conn);

?>

