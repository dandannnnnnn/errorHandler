//All variable names
#define IPaddress     "tcp://192.168.0.101:1883" //IP-address:Port
#define QoS         1

#define timeMQTT_sub	50000L


#define CLIENT    ""
#define PAYLOAD "Hello from Raspberry Pi!"

#define topic1   "MB_subMQTT"
//#define errorTopic	SUB_TOPIC
#define testTopic	"MB_testing"
#define topic2   "MB_publishMQTT"

#define topicLEN   120

#define timestampLEN     80
#define appLEN   160
#define sevCode_LEN     5
#define errorCode_LEN    7
#define errorMSG_LEN		150
#define errorParam_LEN    150
#define errorOutput_LEN     2048


//-------------------------------------
//-------------------------------------
//Defining fields and default strings below
//-------------------------------------
//-------------------------------------

#define sevCode_field 0
#define app_field 1
#define errorCode_field 2
#define errorParam_field 3

#define sevCode_DEFAULT "4"
#define SEV1 '1'
#define SEV4 '4'
#define app_DEFAULT "XXXXXX"
#define errorCode_DEFAULT "MMMXXXX"
#define errorMSG_DEFAULT "xxxxxx"
 
#define fileName "./Error_msg_"
#define fileName_LEN 130
#define MAX_RECORDS 10000
#define timeout 500L