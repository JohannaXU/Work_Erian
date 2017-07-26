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

$query = "SELECT COUNT(*) FROM MACRecord WHERE Position is not NULL AND Record < 25 AND TIMESTAMPDIFF(minute, Timestamp, now()) < 5"; 
//$query = "SELECT COUNT(*) FROM MACRecord WHERE Position is not NULL AND Record < 25";
if($result = mysqli_query($conn, $query))
{
	if($row = mysqli_fetch_assoc($result))
	{
		$num = $row["COUNT(*)"];
		printf($num);
	}
}

mysqli_close($conn);

?>




