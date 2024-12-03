#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <MQTTClient.h> 
#include "settingErrHandler.h" //file with array definitions and length
#include "send_mqtt_msg.c" //file with MQTT functions

// this mqtt token is set as global var to ease up this program
volatile MQTTClient_deliveryToken deliveredtoken;

// This function is called upon when a message is successfully delivered through mqtt
void delivered(void *context, MQTTClient_deliveryToken dt) {
    
    printf("Message with token value %d delivery confirmed\n", dt);
    printf( "-----------------------------------------------\n" );    
    deliveredtoken = dt;
}

// This function is called upon when an incoming message from mqtt is arrived
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *error_in = message->payload;
    char  error_out[ errorOutput_LEN ] = "";
    
    // print incoming message
    printf( "msgarrvd: error_in: <%s>\n", error_in );   
    
    // format error out msg
    sprintf( error_out, "%s + Some additional text here", error_in );
    printf( "msgarrvd: error_out: <%s>\n", error_out );   

    // Create a new client to publish the error_out message
    MQTTClient client = (MQTTClient)context;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token2;

    pubmsg.payload = error_out;
    pubmsg.payloadlen = strlen( error_out );
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    //Publish the error_out message on PUB TOPIC 
    MQTTClient_publishMessage(CLIENT, topic2, &pubmsg, &token2);
    printf("Publishing to topic %s\n", topic2);
    
    // Validate that message has been successfully delivered
    int rc = MQTTClient_waitForCompletion(CLIENT, token2, timeout);
    printf("Message with delivery token %d delivered, rc=%d\n", token2, rc);
    printf( "Msg out:\t<%s>\n", error_out );

    // Close the outgoing message queue
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

// This function is called upon when the connection to the mqtt-broker is lost
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}


int main() {
   // Open MQTT client for listening
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Define the correct call back functions when messages arrive
    MQTTClient_setCallbacks(client, client, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribing to topic %s for client %s using QoS%d\n\n", topic2, CLIENT, QoS);
    MQTTClient_subscribe(CLIENT, topic2, QoS);

    // Keep the program running to continue receiving and publishing messages
    for(;;) {
        ;
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
