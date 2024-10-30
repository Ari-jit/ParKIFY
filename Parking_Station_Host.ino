#include "WiFiS3.h"
#include <WiFiClient.h>

#include <Wire.h>

#include <RTC.h>

// Replace with your network credentials
const char* ssid = "**********";
const char* password = "**********";

WiFiServer server(80);  // Create a server that listens on port 80

//Slot left
int slot1 = 0;
int slot2 = 0;
int slot3 = 0;

//Vehicle entry and exit No.
int VehicleEntry = -1 ;
int VehicleExit = -1 ;

//To determine the best parking point
int PositionPoint = 0;
int ParkingPoint = 0;

//For calculating the amount
String VehicleInfo = "";
double AmountDue = 0.0;

//Time
int hour = 0;
int minute = 0;

//No. of registered vehicle
const int Listed = 6;
String Tag[Listed] = {"C3981E4","E3BDC0D9","13116AA4","39F29E4","A3B846E4","135F1BE4"}; //Change the RFID Data accordingly

//Data of entry and exit time of vehicles
int EntryTimeHour[Listed] = {0 , 0 , 0 , 0, 0, 0};
int EntryTimeMinute[Listed] = {0 , 0 , 0 , 0, 0, 0};
int ExitTimeHour[Listed] = {0 , 0 , 0 , 0, 0, 0};
int ExitTimeMinute[Listed] = {0 , 0 , 0 , 0, 0, 0};


void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

  RTC.begin();

  RTCTime initialTime(17, Month::OCTOBER, 2024, 15, 35, 0, DayOfWeek::WEDNESDAY, SaveLight::SAVING_TIME_ACTIVE);

  // Set the initial time if RTC is not running
  if (!RTC.isRunning()) {
    RTC.setTime(initialTime);
  }

  // Connect to WiFi
  if (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi");
    while (true);
  }

  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  // RTC
  RTCTime currentTime;
  RTC.getTime(currentTime);

  //Print the current Date and Time in Serial monitor
  Serial.print("Current time: ");
  Serial.print(currentTime.getDayOfMonth());
  Serial.print("/");
  Serial.print(Month2int(currentTime.getMonth()));
  Serial.print("/");
  Serial.print(currentTime.getYear());
  Serial.print(" - ");
  Serial.print(currentTime.getHour());
  Serial.print(":");
  Serial.print(currentTime.getMinutes());
  Serial.print(":");
  Serial.println(currentTime.getSeconds());



  //Receive data from each parking station
  Wire.requestFrom(7, sizeof(slot1)); // Request the first parking station
  if (Wire.available() >= sizeof(slot1)) {
    Wire.readBytes((char*)&slot1, sizeof(slot1)); // Receive the ffirst parking station data
    Serial.print("Slot left in Parking Station 1 : ");
    //Separating data
    int data = (slot1);
    Serial.println(data);
    {
      if((data%10)!=0)
        VehicleExit == (data%10)-1;
      data = data/10;
      if((data%10)!=0)
        VehicleEntry == (data%10)-1;
      slot1 = (data/10)-1;

      if(VehicleExit!= -1){
        ExitTimeHour[VehicleExit]= currentTime.getHour();
        ExitTimeMinute[VehicleExit]= currentTime.getMinutes();
      }
      if(VehicleEntry!= -1){
        EntryTimeHour[VehicleExit]= currentTime.getHour();
        EntryTimeMinute[VehicleExit]= currentTime.getMinutes();
      }
    }
  }
  VehicleEntry = -1;
  VehicleExit = -1;
  delay(10);

  Wire.requestFrom(8, sizeof(slot2)); // Request the second parking station
  if (Wire.available() >= sizeof(slot2)) {
    Wire.readBytes((char*)&slot2, sizeof(slot2)); // Receive the second parking station data
    Serial.print("Slot left in Parking Station 2 : ");
    //Separating data
    int data = (slot2);
    Serial.println(data);
    {
      data = data + 256;
      if((data%10)!=0)
        VehicleExit == (data%10)-1;
      data = (data/10);
      if((data%10)!=0)
        VehicleEntry == (data%10)-1;
      slot2 = (data/10)-1;
      
      if(VehicleExit!= -1){
        ExitTimeHour[VehicleExit]= currentTime.getHour();
        ExitTimeMinute[VehicleExit]= currentTime.getMinutes();
      }
      if(VehicleEntry!= -1){
        EntryTimeHour[VehicleExit]= currentTime.getHour();
        EntryTimeMinute[VehicleExit]= currentTime.getMinutes();
      }
    }
  }
  VehicleEntry = -1;
  VehicleExit = -1;
  delay(10);

  Wire.requestFrom(9, sizeof(slot3)); // Request the third parking station
  if (Wire.available() >= sizeof(slot3)) {
    Wire.readBytes((char*)&slot3, sizeof(slot3)); // Receive the third parking station data
    Serial.print("Slot left in Parking Station 3 : ");
    //Separating data
    int data = (slot3);
    Serial.println(data);
    {
      data = data + 256;
      if((data%10)!=0)
        VehicleExit == (data%10)-1;
      data = data/10;
      if((data%10)!=0)
        VehicleEntry == (data%10)-1;
      slot3 = (data/10)-1;
      
      if(VehicleExit!= -1){
        ExitTimeHour[VehicleExit]= currentTime.getHour();
        ExitTimeMinute[VehicleExit]= currentTime.getMinutes();
      }
      if(VehicleEntry!= -1){
        EntryTimeHour[VehicleExit]= currentTime.getHour();
        EntryTimeMinute[VehicleExit]= currentTime.getMinutes();
      }
    }
  }
  VehicleEntry = -1;
  VehicleExit = -1;
  delay(10);

  // Listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          // If the line is blank, the HTTP request has ended
          if (currentLine.length() == 0) {
            // Send the HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // HTML content
            client.println("<!DOCTYPE html><html>");
            client.println("<head><title>ParKIFY</title><style>");
            client.println("body { background-color: silver; font-family: Arial, sans-serif; text-align: center; }");
            client.println("h1 { font-size: 72px; color: blue; }");
            client.println(".box { width: 20vw; height: 20vw; display: inline-block; margin: 10px; border-radius: 15px; text-align: center; font-size: 24px; position: relative; }");
            client.println(".orange-box { background-color: orange; }");
            client.println(".green-box { background-color: green; color: yellow; }");
            client.println(".cyan-box { background-color: cyan; }");
            client.println(".box p.value { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 48px; }");
            client.println(".input-group { display: flex; justify-content: space-between; margin: 20px 0; }");
            client.println("input[type='text'], select { width: 400px; height: 50px; font-size: 24px; font-weight: bold; }");
            client.println("input[type='submit'] { width: 200px; height: 50px; font-size: 24px; font-weight: bold; }");
            client.println(".top-right { position: absolute; top: 10px; right: 10px; text-align: left; }");
            client.println("</style></head><body>");

            client.println("<div class='top-right'>");
            client.println("<a href='https://github.com/Ari-jit' style='color: black; font-size: 24px;'>github.com/Ari-jit</a><br>");
            client.println("<a href='https://instagram.com/arijit_ghosh_acg' style='color: purple; font-size: 24px;'>instagram.com/arijit_ghosh_acg</a><br>");
            client.println("<a href='https://linkedin.com/in/arijit-ghosh-82809522b' style='color: blue; font-size: 24px;'>linkedin.com/in/arijit-ghosh-82809522b</a>");
            client.println("</div>");

            client.println("<h1>ParKIFY</h1>");

            client.println("<div class='box orange-box'><p>Slot left in</p><p>Parking Station 1</p><p class='value'>" + String(slot1) + "</p></div>");
            client.println("<div class='box green-box'><p>Slot left in</p><p>Parking Station 2</p><p class='value'>" + String(slot2) + "</p></div>");
            client.println("<div class='box cyan-box'><p>Slot left in</p><p>Parking Station 3</p><p class='value'>" + String(slot3) + "</p></div>");

            client.println("<div class='input-group'>");
            client.println("<form action='/updateInt' method='get'>");
            client.println("<p style='font-size: 24px; font-weight: bold;'>Enter Position Point:</p>");
            client.println("<select name='PositionPoint' onchange='updateDropdownValue(this.value)'>");
            for(int i = 1; i <= 15; i++) {
              client.println("<option value='" + String(i) + "'>" + String(i) + "</option>");
            }
            client.println("</select><br>");
            client.println("<input type='submit' value='Enter'>");
            client.println("<p id='intValue' style='font-size: 24px; font-weight: bold;'>" + String(ParkingPoint) + "</p>");
            client.println("</form>");

            client.println("<form action='/updateStr' method='get'>");
            client.println("<p style='font-size: 24px; font-weight: bold;'>Enter Vehicle ID:</p>");
            client.println("<input type='text' name='VehicleInfo'><br>");
            client.println("<input type='submit' value='Enter'>");
            client.println("<p id='strValue' style='font-size: 24px; font-weight: bold;'>" + String(AmountDue) + "</p>");
            client.println("</form>");
            client.println("</div>");

            client.println("<script>");
            client.println("function updateDropdownValue(value) { document.getElementById('intValue').innerText = value; }");
            client.println("setInterval(function() { fetch('/data').then(response => response.json()).then(data => { document.getElementById('value1').innerText = data.slot1; document.getElementById('value2').innerText = data.slot2; document.getElementById('value3').innerText = data.slot3; document.getElementById('intValue').innerText = data.ParkingPoint; document.getElementById('strValue').innerText = data.AmountDue; }); }, 1000);");
            client.println("</script>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            break;
          }
          else { // If you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r') { // If you got anything else but a carriage return character, append it to the currentLine
          currentLine += c;

          // Check if GET request and extract data
          if (currentLine.indexOf("GET /updateInt?PositionPoint=") >= 0) {
            int pos = currentLine.indexOf("PositionPoint=") + 14;
            int endPos = currentLine.indexOf('&', pos);
            if (endPos == -1) {
              endPos = currentLine.length();
            }
            PositionPoint = currentLine.substring(pos, endPos).toInt();
            Serial.println("PositionPoint received: " + String(PositionPoint));
            //Finding Parking Point for the give point
            parking_point(PositionPoint);
            Serial.println("ParkingPoint calculated: " + String(ParkingPoint));
          }
          
          else if (currentLine.indexOf("GET /updateStr?VehicleInfo=") >= 0) {
            int pos = currentLine.indexOf("VehicleInfo=") + 12;
            int endPos = currentLine.indexOf(' ', pos);
            if (endPos == -1) {
              endPos = currentLine.length();
            }
            VehicleInfo = currentLine.substring(pos, endPos);
            VehicleInfo.trim();
            Serial.println("VehicleInfo received: " + VehicleInfo);
            //Calculating amount for particular Vehicle ID
            amount(VehicleInfo);
            Serial.println("AmountDue calculated: " + String(AmountDue));
          }
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}


/*
If time grater than 1 hour then per hour Rs. 50.
If time less than 1 hour but grater than half an hour then Rs. 10 added every ten minutes excluding the Rs. 30. And total will be calculated.
If time less than half an hour then Rs. 10 is added every ten minutes
If time less than 10 minutes then Rs. 10 is added.
*/
void amount(String vehicle){
  int vehicleNo = findIndexOf(Tag, Listed, vehicle);
  CalculateTimeDifference(EntryTimeHour[vehicleNo] , EntryTimeMinute[vehicleNo] , ExitTimeHour[vehicleNo] , ExitTimeMinute[vehicleNo]);
  if(hour < 1){
    if(minute < 30){
      if(minute < 10 && minute !=0)
       AmountDue = 10;
       else
       AmountDue = 0;
    }
    else{
      AmountDue = ((minute-30)/ 10)*10 + 30;
    }
  }
  else
   AmountDue = hour*50;
}

//Fing the closest parking station and if it is full it suggests the next closed one
void parking_point(int point){
  if(point <= 5){
    if(slot1 !=0)
     ParkingPoint = 1;
    else if(slot2 !=0)
     ParkingPoint = 2;
    else if(slot3 !=0)
     ParkingPoint = 3;
    else
     ParkingPoint = 0;
      
  }
  else if(point <= 10){
    if((point - 5) < (10 - point)){
      if(slot1 !=0)
       ParkingPoint = 1;
      else if(slot2 !=0)
       ParkingPoint = 2;
      else if(slot3 !=0)
       ParkingPoint = 3;
      else
       ParkingPoint = 0;
    }
    else{
      if(slot2 !=0)
       ParkingPoint = 2;
      else if(slot1 !=0)
       ParkingPoint = 1;
      else if(slot3 !=0)
       ParkingPoint = 3;
      else
       ParkingPoint = 0;
    }
  }
  else {
    if((point - 10) < (15 - point)){
    if(slot2 !=0)
      ParkingPoint = 2;
    else if(slot3 !=0)
      ParkingPoint = 3;
    else if(slot1 !=0)
      ParkingPoint = 1;
    else
      ParkingPoint = 0;
    }
    else{
      if(slot3 !=0)
        ParkingPoint = 3;
      else if(slot2 !=0)
        ParkingPoint = 2;
      else if(slot1 !=0)
        ParkingPoint = 1;
      else
        ParkingPoint = 0;
    }
}

int findIndexOf(String arr[], int size, String target) {
  for (int i = 0; i < Listed; i++) {
    if (arr[i] == target) {
      return i;
    }
  }
  return -1;
}

void CalculateTimeDifference(int initialHour, int initialMinute, int finalHour, int finalMinute) {
  int initialTotalMinutes = initialHour * 60 + initialMinute;
  int finalTotalMinutes = finalHour * 60 + finalMinute;
  
  int diffMinutes = finalTotalMinutes - initialTotalMinutes;

  hour = diffMinutes / 60;
  minute = diffMinutes % 60;

  Serial.print("Time difference: ");
  Serial.print(hour);
  Serial.print(" hours and ");
  Serial.print(minute);
  Serial.println(" minutes");
}
