// Import necessary libraries to be used in this sketch. These libraries must be
// downloaded already.
#include <SPI.h>
#include <SD.h> // library to write and read to SD card
#include <Arduino_MKRENV.h> // library to use the MKR ENV shield
#include "ArduinoLowPower.h" // library to use Low Power modes
// initialize variables to store measurements
float time; // time since beginning of launch (in seconds)
float timeStamp; // time since arduino turns on (in seconds)
float startTime;
float pressure;
float currentElevation;
float altitude;
float voltage; // voltage of battery (in volts)
int arrayLength = 25;
float timeStampArray[25]; // used to store the last 25 time stamps (different from
// time!) in seconds
float pressureArray[25]; // used to store the last 25 pressure readings
Ascent
Apogee
Descent
Dip in pressure
(Most likely from
rocket separation)
// altimeter parameters DO NOT EDIT THESE UNLESS YOU KNOW EXACTLY WHAT YOU ARE DOING!
File myFile;
String fileNameHeader = "File_"; // All launch data files will start with theses
// words. KEEP THE HEADER SHORT! There is a limit to overall file name length.
String fileType = ".csv"; // Launch data files will be written in .csv files
String fileName = "";
String dataFileHeader = "Time(s),Altitude(m),Pressure(kPa),Voltage(V)";
// First row of launch data file
const int chipSelect = 4; //SDCARD_SS_PIN – Change to “28” if using an MKR Zero Board;
bool write2SDCard = true; // Will write to SD card if true.
float triggerAltitude = 10.0; // Arduino will only start recording once it crosses
// this altitudethreshold (in m) -- nominal value: 10.0
float launchPadElevation = -1; // Elevation of the launch pad (in m). Rocket's
// altitude is measured relative to this elevation.
bool launchDetected = false; // False until a launch is detected.
bool launchEnded = false; // False until the end of the launch is detected.
const int padSetUpTime = 60000; // Length of time between sensor set up and launch pad
// elevation calibration (in ms) -- nominal value: 60000 (1 min)
const int padElevationSampleNumber = 25; // number of averaged readings to determine
// pad elevation -- nominal value: 25
int arrayPointer = 0; // cycles between 0 and arrayLength
float batteryMaxVoltage = 4.30; // Voltage of a fully charged battery -- nominal value
// 4.30
int deepSleepDuration = 10000; // Go to deep sleep after launch
void setup() {
// Open serial communications and wait for port to open:
Serial.begin(9600);
// Check if the ENV shield is responsive
if (!ENV.begin()) {
Serial.println("Failed to initialize MKR ENV shield!");
while (1);
}
// Will only run if write2SDCard is true
if (write2SDCard) {
Serial.print("Initializing SD card...");
// see if the card is present and can be initialized:
if (!SD.begin(chipSelect)) {
Serial.println("Card failed, or not present");
// don't do anything more:
while (1);
}
Serial.println("initialization done.");
// Find a file name that is not taken already. Use while loop to increment launch
// number.
int launchNumber = 1;
fileName = fileNameHeader + String(launchNumber) + fileType;
while (SD.exists(fileName)) {
// Check if this file contains launch data, if not, stop incrementing the launch
// number.
File file = SD.open(fileName);
int fileOutput = file.read();
file.close();
if (fileOutput == -1){
break;
}
launchNumber++;
fileName = fileNameHeader + String(launchNumber) + fileType;
}
// Open that file. If the file opened okay, write to it:
myFile = SD.open(fileName, FILE_WRITE);
if (!myFile) {
// if the file didn't open, print an error:
Serial.println("error opening " + fileName);
}
}
// Blink the indicator LED to give visual indication that all checks have been passed.
// While the LED is still blinking, the students should put the altimeter inside the
// rocket
int onTime = 1000; // time the LED is on (in ms)
int offTime = 1000; // time the LED is off (in ms)
int repeat = padSetUpTime / (onTime + offTime); // number of times the LED should
// blink
blink(onTime, offTime, repeat);
// At this point the altimeter sits inside the rocket on the pad.
// Calculate elevation of launch pad by averaging the pressure readings taken.
float pressure_sum = 0;
for (int i = 0; i < padElevationSampleNumber; i++) {
// Take measurements and store them in the time and pressure arrays
for (int j = 0; j < arrayLength; j++) {
timeStampArray[j] = millis() / 1000.0; // convert to seconds
pressureArray[j] = ENV.readPressure(KILOPASCAL);
}
// take the average of this array
pressure = average(pressureArray);
pressure_sum += pressure;
}
// Determine launch pad elevation from the averaged pressure
float pressureAtPad = pressure_sum / padElevationSampleNumber;
launchPadElevation = calculateAltitude(pressureAtPad);
// Print out pad elevation to Serial monitor
String messageHeader = "Pad Elevation (m) is: ";
String elevationMessage = messageHeader + launchPadElevation;
Serial.println(elevationMessage); // for debugging only
delay(500);
Serial.println(fileName);
delay(500);
}
void loop() {
// Obtain measurements
getMeasurements(); // obtains time and pressure measurements
currentElevation = calculateAltitude(pressure); // calculates elevation relative to
// sea level based on pressure reading
altitude = currentElevation - launchPadElevation; // calculates elevation relative to
// launch pad
String dataString = "";
dataString = dataString + String(time) + "," + String(altitude) + "," +
String(pressure) + "," +
String(voltage);
Serial.println(dataString);
// Run if launch has not been detected
if (!launchDetected){
// Check if the trigger altitude has been crossed. This block of code ends up only
// running once.
if (altitude > triggerAltitude) {
Serial.println("Launch detected!");
launchDetected = true;
startTime = timeStamp; // set startTime to current timeStamp
// Write the file headers
myFile = SD.open(fileName, FILE_WRITE);
myFile.println(dataFileHeader);
myFile.close();
}
}
// Runs if launch has been detected and has not ended.
else if (!launchEnded) {
// Check that the rocket is still in flight
if (altitude < triggerAltitude){
launchEnded = true;
}
Serial.println("Altimeter is in flight");
if (write2SDCard) {
// Start writing to SD Card
Serial.println("writing to SD card");
myFile = SD.open(fileName, FILE_WRITE);
myFile.println(dataString);
myFile.close();
}
}
// run if launch has been detected but it has ended
else {
Serial.println("Post launch!");
// Go to deep sleep to conserve battery until the rocket is retrieved.
LowPower.deepSleep(deepSleepDuration);
}
}
float average(float array[]) {
/*
Returns the average of an array of floats
*/
float arraySum = 0;
for (int i = 0; i < arrayLength; i++) {
arraySum += array[i];
}
return arraySum/arrayLength;
}
void blink(int onTime, int offTime, int repeat) {
for (int i = 0; i < repeat; i++) {
digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
delay(onTime); // wait
digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
delay(offTime); // wait
}
}
float getBatteryVoltage() {
/*
Function to read voltage of the external Lipo Battery
*/
int sensorValue = analogRead(ADC_BATTERY); // read the input on analog pin 0:
// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V):
voltage = sensorValue * (batteryMaxVoltage / 1023.0);
}
void getMeasurements() {
/*
Function to return averaged values of sensor readings (raw sensor readings show too
much noise) AND
time
*/
// Update array with newest measurements
timeStampArray[arrayPointer] = millis() / 1000.0 ;
pressureArray[arrayPointer] = ENV.readPressure(KILOPASCAL);
arrayPointer = (arrayPointer + 1) % arrayLength;
// calculate average values in time and pressure array
timeStamp = average(timeStampArray);
pressure = average(pressureArray);
time = timeStamp - startTime;
if (!launchDetected) {
time = -1.0;
}
}
float calculateAltitude(float pressure){
/*
Function for converting pressure readings into elevation.
Returns elevation in meters.
*/
return 145366.45*(1-pow(pressure*10/1013.25,(0.190284)))*0.3048;
}










