
const char* wifi_ssid = "your_wifi_ssid"; //mine was Moxee Hotspot83_2.4G
const char* wifi_password = "your wifi password";
const char* storage_password = "this is whatever is in your storage_password column in your tenant table"; //to ensure someone doesn't store bogus data on your server. should match value in the storage_password column in you user table for your user
//data posted to remote server so we can keep a historical record
//url will be in the form: http://your-server.com:80/weather/data.php?data=
const char* url_get = "/weather/data.php";
const char* host_get = "yourhost.com";
const int ir_pin = 14; //set to the value of an actual data pin if you want to send data via ir from your controller on occasion


const int connection_retry_number = 22;
