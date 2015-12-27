<?php
	header('Content-type: text/plain');
	if(isset($_GET['apikey']) && ($_GET['apikey']=='apikey1' || $_GET['apikey']=='apikey2')){
		if(isset($_GET['open']) && $_GET['open']=='true')
        	{
        		file_put_contents ( 'status.txt' , 'true,'.time() );
			echo "true";
       		}else   if(isset($_GET['open']) && $_GET['open'] =='false')
        	{
                	file_put_contents ( 'status.txt' , 'false,'.time() );
			echo "false";
        	}
        }else{
		$str=file_get_contents('status.txt');
        	$arr=explode(',',$str);
        	$open=$arr[0];
        	$date=$arr[1]; //lastchange: upadate via file see explode/implode
		echo "API Key Missing!\n";
        	echo $open;
	}
?>

