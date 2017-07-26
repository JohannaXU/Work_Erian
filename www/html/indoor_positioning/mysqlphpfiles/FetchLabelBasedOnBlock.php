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

if(isset($_REQUEST['ID']))
{
$_ID = $_REQUEST['ID'];
$query = (" SELECT LABEL FROM MACTrain where ID= '$_ID' ");
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
