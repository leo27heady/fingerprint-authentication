#include <SPI.h>
#include "header.h"
#include <math.h>
#include <avr/pgmspace.h>

int slaveSelect = 2;
int delayTime = 50;

String inString = ""; 
int input_counter = 0;
float Accum;

byte Input[1225];

const int InputNodes = 1225;
const int HiddenNodes_1 = 5;
const int HiddenNodes_2 = 10;
const int OutputNodes = 1;

float Hidden_1[HiddenNodes_1];
float Hidden_2[HiddenNodes_2];
float Output[OutputNodes];

int output_num;

void forward_prop(){
  /******************************************************************
  * Compute input layer activations 
  ******************************************************************/

    Serial.println("\n\n1st Hidden Layer Values:");
    for(int i = 0 ; i < HiddenNodes_1; i++ ) {    
      Accum = pgm_read_float_near( &(HiddenBias_1[i]) );
      for(int j = 0 ; j < InputNodes; j++ ) { 
        Accum += float(Input[j]) * pgm_read_float_near( &(HiddenWeights_1[j][i]) );
      } 
      Hidden_1[i] = max(Accum, 0);
      Serial.println(Hidden_1[i], 8);
    }

    
  /******************************************************************
  * Compute hidden layer activations 
  ******************************************************************/
  
    Serial.println("\n\n2nd Hidden Layer Values:");
    for(int i = 0 ; i < HiddenNodes_2; i++ ) {    
      Accum = pgm_read_float_near( &(HiddenBias_2[i]) );
      for(int j = 0 ; j < HiddenNodes_1; j++ ) {
        Accum += Hidden_1[j] * pgm_read_float_near( &(HiddenWeights_2[j][i]) );
      }
      Hidden_2[i] = max(Accum, 0);
      Serial.println(Hidden_2[i], 8);
    }

  /******************************************************************
  * Compute output layer activations and calculate errors
  ******************************************************************/

    for(int i = 0 ; i < OutputNodes ; i++ ) {    
      Accum = pgm_read_float_near( &(OutputBias[i]) );
      for(int j = 0 ; j < HiddenNodes_2; j++ ) {
        Accum += Hidden_2[j] * pgm_read_float_near( &(OutputWeights[j][i]) );
      }
      Output[i] = 1.0/(1.0 + exp(-Accum));   
    }
    Serial.print("\n\nResult: ");
    Serial.print(Output[0]);
    output_num = int(round(Output[0]));
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(slaveSelect, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);  
}

void loop() {

 while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      inString += (char)inChar; // convert the incoming byte to a char and add it to the string
    }

    if (inChar == ' ') {
      Input[input_counter] = inString.toInt();
      inString = "";
      input_counter++;
    }

    
    // if you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      Serial.print("EndVal:");
      Input[input_counter] = inString.toInt();
      Serial.println(Input[input_counter]);
      Serial.println("\nSequence len: ");
      Serial.println(input_counter+1);

      Serial.println("\nSequence: ");
      for(int i = 0 ; i < 10 ; i++ ) { 
        Serial.print(float(Input[i]));
        Serial.print(", ");
      }
      forward_prop();

      digitalWrite(slaveSelect, LOW);            //Write our Slave select low to enable the SHift register to begin listening for data
      SPI.transfer(output_num+1);                     //Transfer the 8-bit value of data to shift register, remembering that the least significant bit goes first
      digitalWrite(slaveSelect, HIGH);           //Once the transfer is complete, set the latch back to high to stop the shift register listening for data
      Serial.print("\n<------Binary output------>");
      Serial.print("\nSPI output: ");
      Serial.print(output_num+1, BIN);

      
      inString = "";
      input_counter = 0;

      
    }
  }

}
