<?php
$string = <<<XML 
<a>
	<b>
		<c>text</c>
		<c>stuff</c>
	</b>
</a>
XML;

$xml = new SimpleXMLElement($string);
echo $xml->asXML();
?>