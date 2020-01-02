#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Time.h>
#include "SoftwareSerial.h"
#include <FS.h>

SoftwareSerial ZB(13, 15);

//pin 2 as D4   =  LED
//pin 13 as D7   =  rx to ZB_TX
//pin 15 as D8   =  tx to ZB_RX

//#include <ESPAsyncWebServer.h>
ESP8266WebServer server(80);
bool key = false;
char temp[7600];
int modee, brightness;
int syncEnable, syncRole;

int bbrightness = 0;
int fadeAmount = 5;
int delayDuration = 30;
int MIC = A0;  //聲音感測模組 AO 輸出接至 A0 腳
int LED = 2;  //Arduino 板上內建 LED
int micVal;  //紀錄偵測到的音量
int background = 0;  //紀錄環境音量
int sign = 1;
int SoundMAX = 0, SoundMIN = 1000;
void setup()
{
  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(512);
  readEEPROM();
  SPIFFS.begin();

 
  if(syncEnable==1) ZB.begin(38400);

  //Serial.print("Setting soft-AP configuration ... ");
  //Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("YUN Lantern_06","yyuntech") ? "Ready" : "Failed!");
  IPAddress ap(192,168,4,1);
  IPAddress gate(192,168,4,1);
  IPAddress mask(255,255,255,0);
  WiFi.softAPConfig(ap,gate,mask);

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  //server.on("/on",LEDon);
  server.onNotFound(handleNotFound);

  server.begin();



  pinMode(LED, OUTPUT);
  pinMode(A0, INPUT);
}

void loop() {
  server.handleClient();

  if(syncEnable==1&&syncRole==1){                 //is Slave mode.
    if (ZB.available()) {
      String sData = ZB.readStringUntil('\n');
      Serial.print("ZB read: ");
      Serial.println(sData);
      if(!((sData.toFloat()<=255.0)&&(sData.toFloat()>=0.0)));
      else if(sData.length()>=8||sData.length()<=3);
      else{
        analogWrite(LED,sData.toFloat());
        Serial.print("analog OUT: ");
        Serial.println(sData.toFloat());
      }
     }
  }
  else{
    
    switch (modee) {
    case 0:
      Serial.println("Mode0 ON.");   
      Chang();
      break;
    case 1:
      Serial.println("Mode1 ON.");  
      Breathe();
      break;  
    case 2:
      Serial.println("Mode2 ON.");  
      Sound();
      break;
    case 3:
        Serial.println("Mode3 ON.");  
        Candle();
        break;
    case 4:
        Serial.println("Mode4 ON.");  
        Firefly();
        break;
    default:
      Serial.println("Mode not found.");              //kind of initializer.
      syncEnable = 0;
      modee = 0;
      brightness = 50;
      bbrightness = 0;
      break;            
    }
    if(syncEnable==1&&syncRole==0){                   //is Master
      float data = bbrightness*brightness/100.0; 
      ZB.println(String(data));
      ZB.println(String(data));
      Serial.print("Write ");
      Serial.println(String(data));
    }
    
  }
  
  
  
}


void LEDon(){
    Serial.println("LED on");
    if(key)digitalWrite(5, LOW);
    else digitalWrite(5, HIGH);
    key = !key;

    snprintf(temp, 6300,

"<html>\
  <head>\
  </head>\
  <body>\
    <script>\
      javascript:location.href='/'\
    </script>\
  </body>\
</html>"
          );
    server.send(200, "text/html", temp);

}
void handleRoot() {
  //digitalWrite(led, 1);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  int Tmode0 = (modee==0)?0:1;
  int Tmode1 = (modee==1)?0:1;
  int Tmode2 = (modee==2)?0:1;
  int Tbrightness = brightness; 
  int TsyncE = syncEnable;
  int TsyncR0 = (syncRole==0)?0:1;
  int TsyncR1 = (syncRole==1)?0:1;

  snprintf(temp, 7600,

"<!DOCTYPE html>\
<html>\
  <head>\
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8; width=device-width, initial-scale=1\">\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #afc8cc; font-family: Arial, Helvetica, Sans-Serif; Color: #1360AC; }\
      .container {position: relative;padding-left: 70px;padding-right: 40px;margin-bottom: 24px;cursor: pointer;font-size: 50px;-webkit-user-select: none;-moz-user-select: none;-ms-user-select: none;user-select: none;}\
      .container input { position: absolute;opacity: 0;cursor: pointer;}\
      .checkmark { position: absolute;top: 0;left: 0;height: 50px;width: 50px;background-color: #eee;border-radius: 50%%;}\
      .container:hover input ~ .checkmark {background-color: #ccc;}\
      .container input:checked ~ .checkmark {background-color: #2196F3;}\
      .checkmark:after { content: \"\";position: absolute;display: none;}\
      .container input:checked ~ .checkmark:after {display: block;}\
      .container .checkmark:after {top: 18px;left: 18px;width: 16px;height: 16px;border-radius: 50%%;background: white;}\
      .switch {position: relative;display: inline-block;width: 120px;height: 68px;}\
      .switch input {opacity: 0;width: 0;height: 0;}\
      .slider {position: absolute;cursor: pointer;top: 0;left: 0;right: 0;bottom: 0;background-color: #ccc;-webkit-transition: .4s;transition: .4s;border-radius: 68px;}\
      .slider:before {position: absolute;content: \"\";height: 52px;width: 52px;left: 8px;bottom: 8px;background-color: white;-webkit-transition: .4s;transition: .4s;border-radius: 50%%;}\
      input:checked + .slider { background-color: #2196F3;}\
      input:focus + .slider {box-shadow: 0 0 1px #2196F3;}\
      input:checked + .slider:before {-webkit-transform: translateX(52px);-ms-transform: translateX(52px);transform: translateX(52px);}\
      .slidecontainer {width: 80%%;}\
      .slider1 {-webkit-appearance: none;width: 100%%;height: 40px;border-radius: 12px;background: #d3d3d3;outline: none;opacity: 0.7;-webkit-transition: .2s;transition: opacity .2s;}\
      .slider1:hover {opacity: 1;}\
      .slider1::-webkit-slider-thumb {-webkit-appearance: none;appearance: none;width: 75px;height: 75px;border-radius: 50%%;background: #1360AC;cursor: pointer;}\
      .slider1::-moz-range-thumb {width: 65px;height: 65px;border-radius: 50%%;background: #1360AC;cursor: pointer;}\
      h1{font-size: 70px;}\
      h2{font-size: 60px;}\
      p{font-size: 40px;}\
      button{height: 100px;width: 80%%; font-size: 60px;background-color: #1360AC;border: none;color: white;padding: 30px 64px;text-align: center;text-decoration: none;display: inline-block;font-size: 32px;border-start-end-radius: 20%%;}\
      .alert {padding-top: 50px;padding-left: 50px;padding-bottom: 50px;background-color: #f5Ad0A;color: white;font-size: 60px;font-family: Arial, Helvetica, Sans-Serif; width: 74%%;}\
      .closebtn {padding-right: 15px; padding-left: 15px;color: #6C3902;font-weight: bold;float: right;font-size: 60px;cursor: pointer;transition: 0.3s;line-height:250%%}\
      .closebtn:hover {color: #3C2101;}\
    </style>\
  </head>\
  <body>\
    <img src=\"A.png\" alt=\"Flowers in Chania\" style=\"width: 100%%; position: fixed;top: 0; margin-left: -10px;\">\
    <img src=\"B.png\" alt=\"Flowers in Chania\" style=\"width: 50%%; position: fixed; bottom: -90px;right: -120px;\">\
    <h1 style=\"margin-top: 100px;\">&nbsp;</h1>\
    <h2>模式<br><svg height=\"10\" width=100%%><line x1=\"0\" y1=\"0\" x2=80%% y2=\"0\" style=\"stroke:#1360AC;stroke-width:30;margin-top:-10px\"  /></svg></h2>\
    <form id=\"modeForm\">\
    <label class=\"container\">常亮<input type=\"radio\" %s name=\"radioG0\" value=\"0\"><span class=\"checkmark\"></span></label>\
    <label class=\"container\">呼吸<input type=\"radio\" %s name=\"radioG0\" value=\"1\"><span class=\"checkmark\"></span></label>\
    <label class=\"container\">聲感<input type=\"radio\" %s name=\"radioG0\" value=\"2\"><span class=\"checkmark\"></span></label><br><br>\
    <label class=\"container\">燭光<input type=\"radio\" %s name=\"radioG0\" value=\"3\"><span class=\"checkmark\" ></span></label>\
    <label class=\"container\">螢火蟲<input type=\"radio\" %s name=\"radioG0\" value=\"4\"><span class=\"checkmark\"></span></label></form>\
    <br><br><h2>亮度<br><svg height=\"10\" width=100%%><line x1=\"0\" y1=\"0\" x2=80%% y2=\"0\" style=\"stroke:#1360AC;stroke-width:30;margin-top:-10px\"  /></svg></h2>\
    <div class=\"slidecontainer\"><input type=\"range\" min=\"1\" max=\"100\" value=\"%d\" class=\"slider1\" id=\"brightnessBar\"><p>Value: <span id=\"demo\">%d</span>%%</p></div>\
    <br><h2>同步效果<br><svg height=\"10\" width=100%%><line x1=\"0\" y1=\"0\" x2=80%% y2=\"0\" style=\"stroke:#1360AC;stroke-width:30;margin-top:-10px\"  /></svg></h2>\
    <form id=\"syncForm\">\
    <label class=\"switch\"><input type=\"checkbox\" id=\"syncEnableSwitch\" %s onclick=\"disablesyncForm()\" ><span class=\"slider round\"></span></label><br><p>         </p>\
    <label class=\"container\">主控<input type=\"radio\"  name=\"radioG1\" value=\"0\" %s %s><span class=\"checkmark\"></span></label>\
    <label class=\"container\">從端<input type=\"radio\"  name=\"radioG1\" value=\"1\" %s %s><span class=\"checkmark\"></span></label></form>\
    <br>\
    <div id=\"alertArea\">\
    </div>\
    <button type=\"button\" onclick=\"checkSyncAlert()\">儲存</button>\
    <script>\
     var slider = document.getElementById(\"brightnessBar\");var output = document.getElementById(\"demo\");output.innerHTML = slider.value;\
     slider.oninput = function() {output.innerHTML = this.value;};\
     function requestA() {\
         let modeF =  document.getElementById(\"modeForm\"); let syncRoleF = document.getElementById(\"syncForm\");let modeAns, syncEnableAns, syncRoleAns;\
         for (var i = 0; i < modeF.radioG0.length; i++) { if (modeF.radioG0[i].checked) {modeAns = modeF.radioG0[i].value;break;}};\
         for (var i = 0; i < syncRoleF.radioG1.length; i++) { if (syncRoleF.radioG1[i].checked) {syncRoleAns = syncRoleF.radioG1[i].value;break;}};\
         if(document.getElementById(\"syncEnableSwitch\").checked){ syncEnableAns = '1'} else{ syncEnableAns= '0'};\
         var url = '/onA?'+'brightness='+slider.value.toString()+'&mode='+modeAns+'&syncEnable='+syncEnableAns;\
         if(document.getElementById(\"syncEnableSwitch\").checked) url+= '&syncRole='+syncRoleAns;\
         location.href = url;\
    }\
    function disablesyncForm(){\
        if(document.getElementById(\"syncEnableSwitch\").checked){\
            document.getElementById(\"syncForm\").radioG1[0].disabled = false;\
            document.getElementById(\"syncForm\").radioG1[1].disabled= false;\
            document.getElementById(\"syncForm\").radioG1[1].checked = true;\
        }\
        else{\
            document.getElementById(\"syncForm\").radioG1[0].disabled= true;\
            document.getElementById(\"syncForm\").radioG1[0].checked = false;\
            document.getElementById(\"syncForm\").radioG1[1].disabled= true;\
            document.getElementById(\"syncForm\").radioG1[1].checked= false;\
        }\
    }\
    function checkSyncAlert(){\
            var insertDiv = document.getElementById(\"alertArea\");\
            insertDiv.innerHTML = '<div class=\"alert\">'+'<span class=\"closebtn\">'+'知道了 ✔'+'</span>'+'<strong>'+'完成！'+'</strong>'+' 所選的設置已經保存.'+'</div>';\
            setTimeout(function() {requestA();}, 1500);\
    }\
    </script>\
  </body>\
</html>",

           (modee==0)?"checked=\"checked\"":"",(modee==1)?"checked=\"checked\"":"",(modee==2)?"checked=\"checked\"":"",(modee==3)?"checked=\"checked\"":"",(modee==4)?"checked=\"checked\"":"",Tbrightness,Tbrightness,(TsyncE==0)?"value=\"0\"":"checked=\"true\"",(TsyncE==0)?"disabled=true":"", (syncRole==0)?"checked=\"checked\"":"",(TsyncE==0)?"disabled= true":"", (syncRole==1)?"checked=\"checked\"":""
          );
  server.send(200, "text/html", temp);
  memset(temp, 0, sizeof temp);
}

void handleNotFound() {
    if(loadFromSpiffs(server.uri())) return;

  /*String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";*/
  for (uint8_t i = 0; i < server.args(); i++) {
    //message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    String t = server.argName(i);
    if(t=="brightness") brightness = server.arg(i).toInt();
    else if(t=="mode") modee = server.arg(i).toInt();
    else if(t=="syncRole") syncRole = server.arg(i).toInt();
    else if(t=="syncEnable")syncEnable = server.arg(i).toInt();
               
  }
  writeEEPROM(modee,brightness,syncEnable,syncRole);

  Serial.print("Mode:");Serial.print(modee);Serial.print(", Brightness:");Serial.print(brightness);
  Serial.print(", SyncEnable:");Serial.print(syncEnable);Serial.print(", SyncRole:");Serial.print(syncRole);
  Serial.println(" READ");
  //server.send(404, "text/plain", message);

  char redirectBack[150];
  snprintf(redirectBack, 150,

"<html>\
  <head>\
  </head>\
  <body>\
    <script>\
      javascript:location.href='/'\
    </script>\
  </body>\
</html>"
          );
    server.send(200, "text/html", redirectBack);
    if(syncEnable==1){
        ZB.begin(38400);
        Serial.println("Zigbee ON");
      
    }
    else{
      if(ZB) ZB.end();
    }
}

void readEEPROM(){
 modee = EEPROM.read(0); 
 brightness = EEPROM.read(1);
 syncEnable = EEPROM.read(2);
 syncRole = EEPROM.read(3);
 Serial.print("Mode:");Serial.print(modee);Serial.print(", Brightness:");Serial.print(brightness);
 Serial.print(", SyncEnable:");Serial.print(syncEnable);Serial.print(", SyncRole:");Serial.print(syncRole);
 Serial.println(" READ");
 
}
void writeEEPROM(int m, int b, int E, int R){
  EEPROM.write(0, m);
  EEPROM.write(1, b);
  EEPROM.write(2, E);
  EEPROM.write(3, R);
  if (EEPROM.commit()) Serial.println("EEPROM successfully committed");
  else Serial.println("ERROR! EEPROM commit failed");
}

bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith(".png")) dataType = "image/png";
  else return false;
  
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {}
 
  dataFile.close();
  return true;
}




void Chang() { //長亮int brightnessbrightness
  bbrightness = 255;
  analogWrite(LED, bbrightness*brightness/100.0);
  //Serial.println(bbrightness*brightness/100.0);
  bbrightness = 250;

}

void Breathe() { //呼吸燈
  analogWrite(LED, bbrightness*brightness/100.0);
  bbrightness = bbrightness + fadeAmount;
  if (bbrightness <= 0 || bbrightness >= 255) {
    fadeAmount = -fadeAmount ;
  }
  delay(delayDuration);
  //Serial.print(bbrightness*brightness/100.0);                           
}

void Sound() { //聲控燈
  micVal=analogRead(MIC);  //讀取感測器輸出
  Serial.println(micVal);
  bbrightness = map(micVal,SoundMIN,SoundMAX, 0, 255);
  analogWrite(LED, bbrightness*brightness/100.0);
  if(micVal>SoundMAX)SoundMAX=micVal;
  if(micVal<SoundMIN)SoundMIN=micVal;
  /*if (micVal-background > 10) {  //若超過背景音量 10
    Serial.println(micVal);
    toggle = !toggle;  //反轉 LED 狀態
    if (toggle) {digitalWrite(LED, HIGH);}  //狀態為 ON 就點亮 LED
    else {digitalWrite(LED, LOW);}  //狀態為 OFF 就點亮 LED
    }*/
  delay(50);
}

void Firefly(){
  analogWrite(LED, bbrightness*brightness/100.0);
  int RandFadeAmount = random(1, 50);
  bbrightness = bbrightness + RandFadeAmount * sign;
  if (bbrightness <= 0) {
    sign = sign * (-1);
    bbrightness = -bbrightness;
  }
  else if(bbrightness >= 255){
    sign = sign * (-1);
    bbrightness = 255-(bbrightness-255);
  }
  delay(45);
  Serial.print(bbrightness*brightness/100.0);   
}

void Candle(){
  analogWrite(LED, bbrightness*brightness/100.0);
  int RandFadeAmount = random(1, 60);
  bbrightness = 255 - RandFadeAmount*1.5;
  
  delay(60);
  Serial.print(bbrightness*brightness/100.0);   
}
