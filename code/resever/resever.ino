#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <ArxContainer.h>

RF24 radio(7, 8);//ce & csn

String abc = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,[]{}-=*/";
//addresses//
byte addresses[][6] = {"1Node", "2Node"};


int modInverse(int a, int m) {
    int m0 = m, t, q;
    int x0 = 0, x1 = 1;

    if (m == 1) {
        return 0;
    }

    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) {
        x1 += m0;
    }

    return x1;
}

long rsa_decrypt(long c, long d, long n) {
    long m = 1;
    while (d > 0) {
        if (d & 1) {
            m = (m * c) % n;
        }
        c = (c * c) % n;
        d >>= 1;
    }
    return m;
}

String dencrypt_result(arx::vector<int> v) {
    int d = modInverse(13, 2436);
    int c = 0;
    String ans = "";
    while (c < v.size()) {
        long x = rsa_decrypt(v[c], d, 2537);
        //lon
        ans += abc[x];
        c++;
    }
    return ans;
}

// -----------------------------------------------------------------------------
// SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP
// -----------------------------------------------------------------------------
int receivedData[16];

void receiveData(void** data, size_t* dataSize) {
    const size_t maxPayloadSize = radio.getPayloadSize();
    int* receivedData = nullptr;
    size_t receivedSize = 0;
    size_t remainingData = 0;
    
    while (radio.available()) {
        if (receivedData == nullptr) {
            // First iteration, read the data size
            radio.read(&receivedSize, sizeof(receivedSize));
            remainingData = receivedSize;
            receivedData = new int[(receivedSize/2)-1];
            //delay(30);
        }
        else {
            // Subsequent iterations, read the data in chunks
            size_t chunkSize = min(remainingData, maxPayloadSize);

            radio.read(receivedData + (receivedSize/2 - remainingData/2), chunkSize);
            remainingData -= chunkSize;

        }
        delay(30);
    }
    
    *data = reinterpret_cast<void*>(receivedData);
    *dataSize = receivedSize;
}




void setup() {
    Serial.begin(9600);
    Serial.println("The Arduino Is Ready To Receive ");

    radio.begin();


    radio.enableDynamicPayloads();
    radio.setDataRate(RF24_2MBPS);

    radio.setChannel(124);

    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);

    randomSeed(analogRead(A0));
}

// -----------------------------------------------------------------------------
// LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// -----------------------------------------------------------------------------
void loop() {
    unsigned char data = random(0, 4);

    radio.startListening();

    unsigned long started_waiting_at = millis();

    while (!radio.available()) {
        if (millis() - started_waiting_at > 200) {
//            Serial.println("No response received - timeout!");
            return;
        }
    }
    void* receivedData;
    size_t receivedSize;

    
    arx::vector<int> dataRx;


    receiveData(&receivedData, &receivedSize); // Read the byte array

        

        if (receivedData != nullptr) {
            int* data = static_cast<int*>(receivedData);

            // Process the received data as needed
            Serial.print("Received: ");
            for (size_t i = 0; i < receivedSize / sizeof(int)-1; i++) {
               dataRx.push_back(data[i]);
                Serial.print(abc[data[i]%26]);
                //Serial.print(" ");
            }
            Serial.println();

            delete[] static_cast<uint8_t*>(receivedData);
        }
    Serial.print("The Message:");
    Serial.println(dencrypt_result(dataRx));
    dataRx.clear();


    delay(1000);
}
