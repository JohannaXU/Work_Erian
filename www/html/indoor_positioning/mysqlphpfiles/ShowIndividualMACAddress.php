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

if(isset($_REQUEST['MACAdd']))
{
	$MACAdd = $_REQUEST['MACAdd'];
	$query = "SELECT Position From MACRecord WHERE MACAdd='$MACAdd'";
}

if($result = mysqli_query($conn,$query)) {

while ($row = mysqli_fetch_assoc($result)) 
 {
   $output[]=$row;
 }
}

print (json_encode($output));

mysqli_close($conn);

?>

