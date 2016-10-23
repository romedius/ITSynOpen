<?php
    header('Content-type: application/json');
    $str=file_get_contents('status.txt');
    $arr=explode(',',$str);
    $open=$arr[0];
    $date=$arr[1]; //lastchange: upadate via file see explode/implode
    echo <<< eot
{
  "api":"0.13",
  "space":"IT-Syndikat",
  "logo":"http://it-syndikat.org/api/images/its_l.png",
  "url":"http://it-syndikat.org",
  "location":{
    "address":"Tschamlerstrasse 3, 6020 Innsbruck, Austria",
    "lat":47.2578,
    "lon":11.3961
  },
  "state":{
    "open":$open,
    "lastchange":$date,
    "icon":{
      "open":"http://it-syndikat.org/api/images/ITS_Open.png",
      "closed":"http://it-syndikat.org/api/images/ITS_Closed.png"
    }
  },
  "contact":{
    "phone":"+43512563468",
    "twitter":"@ItSyndikat",
    "email":"wir@it-syndikat.org",
    "ml":"its-public@lists.catbull.com"
  },
  "issue_report_channels":["email"],
  "ext_ccc": "chaostreff"
}

eot;

?>
