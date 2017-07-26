<?php 
ini_set('error_reporting', E_ALL);
$conn = mysqli_connect ("localhost", "root", "root", "HAS");

/* check connection */
if (mysqli_connect_errno ()) {
	printf ( "Connect failed: %s\n", mysqli_connect_error () );
	exit ();
} else {
echo "connected";
} 
?>
