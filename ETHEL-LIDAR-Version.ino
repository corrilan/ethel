    /*
    * ETHEL - 
    * Ultrasonic Sensor HC-SR04 and Arduino to detect stopped vehicle
    * Mini ENC28J60
    * TFMini
     */
    /* Post a message to Twitter  */
    #include <SPI.h>
    //#include <Ethernet.h>
    //#include <UIPEthernet.h>
    //#include <EtherCard.h>
    //#include <Twitter.h>
    #include <TimeLib.h>
    //#include <time.h>
    #include <SoftwareSerial.h>
    #include <TFMini.h>
    
    #define CONGESTING_VEHICLE_LANE 40 //Distance to congesting vehicle lane 
    #define CONGESTING_VEHICLE_FAR_CURB 50 //Distance to congesting vehicle lane far curb
    #define VERY_LONG_DELAY_MS 900000 // 15 minutes
    #define LONG_DELAY_MS 30000 // 30 seconds
    #define SHORT_DELAY_MS 5000 // 5 seconds    

static byte myip[] = {192,168,2,100};
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte mygateway[] = {192,168,2,1};
static byte mydns[] = {8,8,8,8};
static byte mymask[] = {255,255,255,0};
static byte session;
//byte Ethernet::buffer[700];

#define TOKEN   ""
   // Twitter twitter("");
const char website[] PROGMEM = "arduino-tweet.appspot.com";

    // Variable for Twitter message
    char msg[50];

    // Setup software serial port 
    SoftwareSerial mySerial(3, 7);      // Uno RX (TFMINI TX Green), Uno TX (TFMINI RX White)
    TFMini tfmini;
    
    // Defines variables
    long duration;
    long interval = 5000;  // Initially set interval to 30 seconds
    int congestion_count = 0;
    int free_flowing_count = 0;
    String interval_code_ran = "false";
    String congested = "unknown";
    uint16_t distance;

    unsigned long previousMillis = 0;   // will store last time LED was updated
    
//byte Ethernet::buffer[700];
//Stash stash;

    void setup()
    {
      Serial.begin(115200); // Starts the serial communication
/*
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
  
  ether.printIp("My IP: ", ether.myip);
  // ether.printIp("Netmask: ", ether.mymask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
  ether.printIp("Server: ", ether.hisip);
  */    
      mySerial.begin(TFMINI_BAUDRATE);  // Initialize the data rate for the SoftwareSerial port
      tfmini.begin(&mySerial); // Initialize the TF Mini sensor
      //tfmini.setSingleScanMode();  // Initialize single measurement mode with external trigger
      settime();
      //ether.staticSetup(myip,mygateway,mydns,mymask);
      //ether.dhcpSetup();
      //Ethernet.begin(mac, ip);
      sprintf(msg, "%d:%d:%d:%d:%d:%d Booting...", hour(now()),minute(now()),second(now()),day(now()),month(now()),year(now()));
      Serial.println(msg);
      //tweet();
    }

    void loop()
    {
      //check_response();
      String while_ran = "false";
      while (while_ran != "true")
      {
        check_distance();
        unsigned long currentMillis = millis(); 
       
        if (currentMillis - previousMillis >= interval)
        {
          // save the last time you ran
          previousMillis = currentMillis;
          //check_distance();
          while_ran = "true";
        }
        //check_distance();
      }

      Serial.println("Still running.  Going to check congestion...");
      check_congestion();
    }


    void check_distance()
    {
      //check_response();
      
      distance = tfmini.getDistance();  // Take one TF Mini distance measurement
      //Serial.println(distance);
    }

    void check_congestion()
    {
      if (congested != "true" && distance >= CONGESTING_VEHICLE_LANE && distance <= CONGESTING_VEHICLE_FAR_CURB)
      {
        Serial.println("Possible congesting vehicle.");
        congestion_count++;
        interval = SHORT_DELAY_MS;
        String break_out = "false";
        for (int i=0; (i < 5 && break_out != "true"); i++)
        {
          String while_ran = "false";
          while (while_ran != "true" && break_out != "true")
          {
            check_distance();
            
            unsigned long currentMillis = millis(); 
       
            if (currentMillis - previousMillis >= interval)
            {
              // save the last time you ran
              previousMillis = currentMillis;
        
              //check_distance();
          
              if (distance >= CONGESTING_VEHICLE_LANE && distance <= CONGESTING_VEHICLE_FAR_CURB)
              {
                Serial.println("Possible congesting vehicle.");
                congestion_count++;
              }

              else
              {
                Serial.println("False alarm");
                congestion_count = 0;
                interval = LONG_DELAY_MS;
                break_out = "true";
                //break;
              }
            
              while_ran = "true";
            }
           }
        }
        
        if (congestion_count == 6)
        {
          Serial.println("Confirmed congesting vehicle.");
          congested = "true";
          congestion_count = 0;
          //delay(VERY_LONG_DELAY_MS); //Assume it takes 15 minutes to reopen
          interval = LONG_DELAY_MS;
          sprintf(msg, "%d:%d:%d:%d:%d:%d Congested.", hour(now()),minute(now()),second(now()),day(now()),month(now()),year(now()));
          //tweet();
         }
      }
     
      if (congested != "false" && distance >= CONGESTING_VEHICLE_FAR_CURB)
      {
        Serial.println("Possible free flowing traffic.");
        free_flowing_count++;
        interval = SHORT_DELAY_MS;
        String break_out = "false";
        for (int i=0; (i < 5 && break_out != "true"); i++)
        {
          String while_ran = "false";
          while (while_ran != "true" && break_out != "true")
          {
            check_distance();
            
            unsigned long currentMillis = millis(); 
       
            if (currentMillis - previousMillis >= interval)
            {
              // save the last time you ran
              previousMillis = currentMillis;
        
              //check_distance();
          
              if (distance >= CONGESTING_VEHICLE_FAR_CURB)
              {
                Serial.println("Possible free flowing traffic.");
                free_flowing_count++;
              }

              else
              {
                Serial.println("False alarm");
                free_flowing_count = 0;
                interval = LONG_DELAY_MS;
                break_out = "true";
                //break;
              }
              
              while_ran = "true";
             }
             //check_distance(); 
            }
          }
        
          if (free_flowing_count == 6)
          {
            Serial.println("Confirmed free flowing traffic.");
            congested = "false";
            free_flowing_count = 0;
            //delay(VERY_LONG_DELAY_MS); //Assume it takes 15 minutes to reopen
            interval = LONG_DELAY_MS;
            sprintf(msg, "%d:%d:%d:%d:%d:%d Free flowing.", hour(now()),minute(now()),second(now()),day(now()),month(now()),year(now()));
            //tweet();
          } 
      }
    }        
    /*
    void tweet()
    {
  Serial.println("Sending tweet...");
  byte sd = stash.create();

  //const char tweet[] = "message";
  stash.print("token=");
  stash.print(TOKEN);
  stash.print("&status=");
  stash.println(msg);
  stash.save();
  int stash_size = stash.size();

  // Compose the http POST request, taking the headers below and appending
  // previously created stash in the sd holder.
  Stash::prepare(PSTR("POST http://$F/update HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "Content-Length: $D" "\r\n"
    "\r\n"
    "$H"),
  website, website, stash_size, sd);

  // send the packet - this also releases all stash buffers once done
  // Save the session ID so we can watch for it in the main loop.
  session = ether.tcpSend();
    }
*/
    void settime()
    {
      //timeStatus();
      setTime(1530375099);
    }
    /*
    void check_response()
    {
      ether.packetLoop(ether.packetReceive());

      const char* reply = ether.tcpReply(session);
      if (reply != 0)
      {
        Serial.println("Got a response!");
        Serial.println(reply);
      }  
    }
    */
