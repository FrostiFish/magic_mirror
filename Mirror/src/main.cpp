#include <Arduino.h>
#include <arduinoFFT.h>      //https://github.com/kosme/arduinoFFT
#include <OOCSI.h>

//FFT
#define NUM_BANDS  8
#define READ_DELAY 50
#define USE_RANDOM_DATA false

arduinoFFT FFT = arduinoFFT();

#define SAMPLES 512              //Must be a power of 2
#define SAMPLING_FREQUENCY 40000 //Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT.
#define BINFREQUENCY (SAMPLING_FREQUENCY/SAMPLES)

const int dcCompensation = 2;
const int bassBins =  3;
const int midBins = 13;
const int trebleBins = 127;

// Use ADC1 so that WiFi stuff doesnt interfere with ADC measurements
#define ADC_PIN 34

unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];
int bass[bassBins];
int mid[midBins];
int treble[trebleBins];
unsigned long newTime, oldTime;

//OOCSI
OOCSI oocsi = OOCSI();
uint32_t oocsiSendingRate = 1000; //In milliseconds
uint32_t lastSendMillis = 0;

//HY-SRF05
const uint8_t distanceArraySamples = 5;
int distanceSamplePeriod = 500000; //In microseconds

//Left unit
#define LEFT_TRIG_PIN 32
#define LEFT_ECHO_PIN 33

uint32_t leftTriggerMicros;
uint32_t newLeftDistance = 0;
uint32_t leftDistanceArray[distanceArraySamples];

//Right unit
#define RIGHT_TRIG_PIN 26
#define RIGHT_ECHO_PIN 27

uint32_t rightTriggerMicros;
uint32_t newRightDistance = 0;
uint32_t rightDistanceArray[distanceArraySamples];

void sendTrigger(int8_t pin, uint32_t &triggerMicros) {
  digitalWrite(pin, HIGH);
  triggerMicros = micros();
  delayMicroseconds(10);
  digitalWrite(pin, LOW);
}

//Functions
void pushIntoArray(uint32_t *array, int numOfElements, int newElement) {
  for (int i = numOfElements; i > 0; i--) {
    array[i];
  }
  array[0] = newElement;
}

int arrayAverage(uint32_t *array, int numOfElements) {
  long sum = 0;

  for (int i = 0; i < numOfElements; i++) {
    sum += array[i];
  }
  return sum/numOfElements;
}

//Interrupts
void processOOCSI() {

}

/*
void leftEcho() {
  newLeftDistance = (micros() - leftTriggerMicros);
}
*/

void setup() {
  oocsi.connect("group1_mirror", "oocsi.id.tue.nl", "Habbo Hotel", "ABCikbengay69", processOOCSI);

  Serial.begin(115200);
  analogReadResolution(12);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  pinMode(ADC_PIN, INPUT);

  pinMode(LEFT_TRIG_PIN, OUTPUT);
  pinMode(LEFT_ECHO_PIN, INPUT);
  pinMode(RIGHT_TRIG_PIN, OUTPUT);
  pinMode(RIGHT_ECHO_PIN, INPUT);
  //attachInterrupt(digitalPinToInterrupt(LEFT_ECHO_PIN), leftEcho, RISING);
}

void loop() {

  //Take samples from microphone pin for FFT calculation
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros() - oldTime;
    oldTime = newTime;
    vReal[i] = analogRead(ADC_PIN);
    vImag[i] = 0;
    while (micros() < (newTime + sampling_period_us))
    {
      delay(0);
    }
  }
  //Calculate FFT
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  for (int i = 0; i < bassBins; i++) {
    bass[i] = vReal[i+dcCompensation];
  }
  for (int i = 0; i < midBins; i++) {
    mid[i] = vReal[i+bassBins+dcCompensation];
  }
  for (int i = 0; i < trebleBins; i++) {
    treble[i] = vReal[i+midBins+bassBins+dcCompensation];
  }

  //Put old distance measurement into array
  if ((micros() - leftTriggerMicros) >= distanceSamplePeriod) {
    pushIntoArray(leftDistanceArray, distanceArraySamples, newLeftDistance);
    sendTrigger(LEFT_TRIG_PIN, leftTriggerMicros);
    newLeftDistance = pulseIn(LEFT_ECHO_PIN, HIGH, distanceSamplePeriod)/29/2;
  }

  if ((micros() - rightTriggerMicros) >= distanceSamplePeriod) {
    pushIntoArray(rightDistanceArray, distanceArraySamples, newRightDistance);
    sendTrigger(RIGHT_TRIG_PIN, rightTriggerMicros);
    newRightDistance = pulseIn(RIGHT_ECHO_PIN, HIGH, distanceSamplePeriod)/29/2;
  }

  if ((millis() - lastSendMillis) >= oocsiSendingRate) {
    lastSendMillis = millis();

    oocsi.newMessage("magic_mirror");
    oocsi.addInt("distL", leftDistanceArray[0]);
    oocsi.addInt("distR", rightDistanceArray[0]);
    oocsi.addInt("bassBins", bassBins);
    oocsi.addIntArray("bass", bass, bassBins);
    oocsi.addInt("midBins", midBins);
    oocsi.addIntArray("mid", mid, midBins);
    oocsi.addInt("trebleBins", trebleBins);
    oocsi.sendMessage();

    oocsi.newMessage("magic_mirror");
    oocsi.addIntArray("treble", treble, trebleBins);
    oocsi.sendMessage();
  }
}