<!--
Copypasta:

char thingSpeakAddress[] = "api.thingspeak.com";


-->
<?php


$username = ($_POST['t_user']);
$password = ($_POST['t_pass']);
$message = ($_POST['t_update']);

$url = 'http://twitter.com/statuses/update.xml';
$curl_handle = curl_init();
curl_setopt($curl_handle, CURLOPT_URL, "$url");
curl_setopt($curl_handle, CURLOPT_CONNECTTIMEOUT, 2);
curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl_handle, CURLOPT_POST, 1);
curl_setopt($curl_handle, CURLOPT_POSTFIELDS, "status=$message");
curl_setopt($curl_handle, CURLOPT_USERPWD, "$username:$password");
$buffer = curl_exec($curl_handle);
curl_close($curl_handle);
if (empty($buffer)) {
 echo "<p align=\"center\" >".'Sorry, due to an error your Twitter status could not be updated! Please check your username/password!'."</p>";
} else {
 echo "<p align=\"center\">".'Your Twitter status has successfully been updated!'."</p>";
}
?>


