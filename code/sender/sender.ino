#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <ArxContainer.h>


RF24 radio(7, 8);// ce & csn

String abc = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,[]{}-=*/";
//addresses//
byte addresses[][6] = {"1Node","2Node"};

//encrypt////
long rsa_encrypt(long m, long e, long n) {
    long c = 1;
    while (e > 0) {
        if (e & 1) {
            c = (c * m) % n;
        }
        m = (m * m) % n;
        e >>= 1;
    }
    return c;
}

void encrypt_result(String w, arx::vector<int>* ans)
{
    ans->push_back(0);
    int c = 0;
    while (c < w.length() - 1)
    {
        int m = abc.indexOf(w[c]);
        int x = rsa_encrypt(m, 13, 2537);
        //int Second  = rsa_encrypt(first, 13, 2537);
        ans->push_back(x);
        c++;
    }
}

void sendData(const void* data, size_t dataSize) {
    const uint8_t* byteData = reinterpret_cast<const uint8_t*>(data);
    size_t remainingData = dataSize;
    size_t offset = 0;
    const size_t maxPayloadSize = radio.getPayloadSize();
    

    
        radio.write(&dataSize, sizeof(dataSize));

        
        offset += sizeof(dataSize);
        delay(10); // Delay between sending packets
    
    
    while (remainingData > 0) {
        size_t chunkSize = min(remainingData, maxPayloadSize);
        radio.write(byteData + offset, chunkSize);
        offset += chunkSize;
        remainingData -= chunkSize;
        delay(10); // Delay between sending packets
    }
}

// -----------------------------------------------------------------------------
// SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP
// -----------------------------------------------------------------------------
int dataToSend[64];
void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("The Arduino Is Ready");
  Serial.println("Write the message you want to sent ");

  radio.begin();

  radio.setDataRate(RF24_2MBPS);
  radio.enableDynamicPayloads();


  radio.setChannel(124);
  
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);


  radio.startListening();
    for (int i = 0; i < 64; i++) {
    dataToSend[i] = i;
  }

}

// -----------------------------------------------------------------------------
// We are LISTENING on this device only (although we do transmit a response)
// -----------------------------------------------------------------------------
void loop() {

  radio.stopListening();
  char incomingChar; 
  String inputString = "";
  arx::vector<int> ans;
  
  if (Serial.available()) 
  {
    inputString = Serial.readString();
    Serial.print("The message: ");
    Serial.print(inputString);
    encrypt_result(inputString, &ans);
    Serial.print("encrypted: ");
    for (int i = 1; i < ans.size(); i++)
    {
      Serial.print(abc[ans[i]%26]);
      
    }
    Serial.println();
    sendData(ans.data(), ans.size() * sizeof(int));
    
    Serial.println("The Message Sent ");
    Serial.println("write The Message You Want To Send: ");
  }
}
