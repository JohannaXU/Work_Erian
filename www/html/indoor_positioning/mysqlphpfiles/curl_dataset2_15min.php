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

$query = "SELECT MOMENT, TOTAL FROM RecordList WHERE TIMESTAMPDIFF(minute, MOMENT, now()) < 15"; 
//$query = "SELECT MOMENT, TOTAL FROM RecordList"; 

if($result = mysqli_query($conn, $query))
{
	$data = array('moment' => 'null', 'total' => 'null');
	$i = 0;
	while($row != 0)
	{
		if($row = mysqli_fetch_assoc($result))
		{
			$data["total"] = $data["total"] + $row["TOTAL"];
			$data["moment"] = $row["MOMENT"];
//			printf (json_encode($row));
			$i = $i + 1;
		}
	}
	$data["total"] = ceil(((float)$data["total"])/$i);
// 	printf ($data["total"]);
}

//$data["total"] = (int)$data["total"];
$data["moment"] = date('Y-m-d\TH:i:s\Z', strtotime($data["moment"]));

printf (json_encode($data));
//return $data;

mysqli_close($conn);

?>



