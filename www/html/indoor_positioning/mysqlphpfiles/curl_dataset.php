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

$query = "SELECT MOMENT, TOTAL FROM RecordList WHERE TIMESTAMPDIFF(minute, MOMENT, now()) < 5"; 
//$query = "SELECT MOMENT, TOTAL FROM RecordList"; 

if($result = mysqli_query($conn, $query))
{
	while($row != 0)
	{
		if($row = mysqli_fetch_assoc($result))
		{
			$output = $row;
		}
	}
}

$data = array('moment' => 'null', 'total' => 'null');

//$data["moment"] = $output["MOMENT"];
$data["total"] = (int)$output["TOTAL"];
//$data["moment"] = date(DATE_ISO8601, strtotime($output["MOMENT"]));
$data["moment"] = date('Y-m-d\TH:i:s\Z', strtotime($output["MOMENT"]));

printf (json_encode($data));
//return $data;

mysqli_close($conn);

?>



