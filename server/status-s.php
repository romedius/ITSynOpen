<?php
	header('Content-type: text/plain');
	$str=file_get_contents('status.txt');
        $arr=explode(',',$str);
        $open=$arr[0];
        $date=$arr[1]; //lastchange: upadate via file see explode/implode
        echo $open;
?>

