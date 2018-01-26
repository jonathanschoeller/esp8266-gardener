# ESP8266 Gardener
An ESP8266 based project for remotely watering plants on battery power.

## Compiling the ESP8266 Gardener
The ESP8266 Gardener connects to your Wi-Fi network and communicates with AWS IoT in the cloud. As a prerequisite, you will need to have an AWS account.

First, create a file in the root folder named keys.cpp. This file will contain your passwords and other details that are unique to your project.
Next, use the included [AWS CloudFormation](esp8266-gardener.cform) template to create a new stack.
Once the CloudFormation stack is created, look at the outputs from the stack. You should see `AccessKeyID`, `SecretAccessKey`, `AWSRegion`, and `AWSPath`.
In the keys.cpp file you created, add the following:
```cpp
#include "keys.h"

const char* awsRegion = // The CloudFormation output value for AWSRegion;
const char* awsDomain = // Get this from https://console.aws.amazon.com/iot/home, Manage, Thing, Interact, Rest API Endpoint. or from https://docs.aws.amazon.com/iot/latest/developerguide/thing-shadow-rest-api.html;
const char* awsPath = // The CloudFormation output value for AWSPath;

const char* awsKeyID = // The CloudFormation output value for AWSKeyID;
const char* awsSecKey = // The CloudFormation output value for SecretAccessKey;
const char* ssid = // The SSID for your wi-fi network;
const char* password = // The password for your wifi network;
```

The ESP8266 Gardener uses the [Arduino core for ESP8266 library](https://github.com/esp8266/Arduino). To compile and install it on your ESP8266 device, open `esp8266-gardener.ino` in the Arduino IDE.