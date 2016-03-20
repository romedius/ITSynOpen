<?php
	header('Content-type: text/plain');
	if(isset($_GET['apikey']) && ($_GET['apikey']=='apikey1' || $_GET['apikey']=='apikey2')){
		$str=file_get_contents('pinged.txt');
        	echo $str;
     		file_put_contents ( 'pinged.txt' , 'false' );
        }else{
		$str=file_get_contents('pinged.txt');
		echo "API Key Missing!\n";
        	echo $str;
	}
?>

