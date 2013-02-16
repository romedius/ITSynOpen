<?php
	header('Content-type: application/json');
	$str=file_get_contents('status.txt');
        $arr=explode(',',$str);
        $open=$arr[0];
        $date=$arr[1]; //lastchange: upadate via file see explode/implode
        echo <<< eot
{
  "api":"0.12",
  "space":"IT-Syndikat",
  "url":"http://it-syndikat.org",
  "icon":{
    "open":"http://it-syndikat.org/images/ITS_Open.png",
    "closed":"http://it-syndikat.org/images/ITS_Closed.png"
  },
  "address":"Tschamlerstrasse 3, 6020 Innsbruck, Austria",
  "contact":{
    "phone":"+43512563468",
    "twitter":"@ItSyndikat",
    "email":"wir@it-syndikat.org",
    "ml":"its-public@lists.catbull.com"
  },
  "open":$open,
  "lastchange":$date,
  "logo":"http://it-syndikat.org/images/its_l.png",
  "lat":47.2578,
  "lon":11.3961
}

eot;

?>

