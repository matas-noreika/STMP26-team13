// Include the TinyGPS++ library to parse NMEA data sentences from the Waveshare module
#include <TinyGPS++.h>

// Include the SoftwareSerial library to allow serial communication on digital pins
#include <SoftwareSerial.h>

// Define the Arduino pin connected to the Waveshare module's Transmit (TX) pin
const int Waveshare_TX_Pin = 2; 

// Define the Arduino pin connected to the Waveshare module's Receive (RX) pin
const int Waveshare_RX_Pin = 3;

// Set the communication speed for the Waveshare GPS chip (9600 is standard for L76X/MAX-M8Q)
const uint32_t GPSBaud = 9600; 

// Define the exact Latitude coordinate of the center of your geofence target area
const double TARGET_LAT = 53.349805;       

// Define the exact Longitude coordinate of the center of your geofence target area
const double TARGET_LON = -6.260310;        

// Set the radius limit of your virtual boundary fence measured in meters
const double GEOFENCE_RADIUS_METERS = 100.0; 

// Create an instance of the TinyGPSPlus object to access GPS parsing functions
TinyGPSPlus gps;

// Initialize a SoftwareSerial object named 'ss' using our defined RX and TX pins
SoftwareSerial ss(Waveshare_TX_Pin, Waveshare_RX_Pin);

// The setup function runs once when the Arduino boots up or resets
void setup() {
  // Initialize the main hardware serial port at 115200 baud for the PC Serial Monitor
  Serial.begin(115200); 
  
  // Initialize the software serial port to start listening to the Waveshare module
  ss.begin(GPSBaud);    
  
  // Configure the Arduino's built-in LED pin as an output component
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Turn off the built-in LED immediately on startup
  digitalWrite(LED_BUILTIN, LOW);

  // Print an initialization message to the PC Serial Monitor
  Serial.println("--- Waveshare GNSS Geofence Active ---");
}

// The loop function runs continuously in a cycle after setup finishes
void loop() {
  // Check if there are raw bytes of data waiting in the software serial buffer from the GPS
  while (ss.available() > 0) {
    // Read a byte of data and feed it into the TinyGPS++ parser; returns true if a full phrase is ready
    if (gps.encode(ss.read())) {
      // Call our custom geofence processing function once a valid data packet is parsed
      processGeofence();
    }
  }

  // Error check: If 5 seconds have passed and the GPS object hasn't processed any character data...
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    // Print a warning indicating a physical wiring error or incorrect baud rate
    Serial.println("Warning: No data received from Waveshare module. Check TX/RX jumpers.");
    // Pause execution for 5 seconds to prevent spamming the serial monitor
    delay(5000);
  }
}

// Custom function containing the core logic for checking the geofence
void processGeofence() {
  // Verify if the Waveshare module has established a lock and generated valid coordinate data
  if (gps.location.isValid()) {
    // Extract the current latitude value from the parsed GPS data structure
    double currentLat = gps.location.lat();
    
    // Extract the current longitude value from the parsed GPS data structure
    double currentLon = gps.location.lng();

    // Use TinyGPS++ math to calculate the distance between your location and target in meters
    double distanceMeters = gps.distanceBetween(currentLat, currentLon, TARGET_LAT, TARGET_LON);

    // Print the string label for tracked satellites to the screen
    Serial.print("Satellites: ");
    // Fetch and print the total number of connected satellites tracked by the Waveshare board
    Serial.print(gps.satellites.value());
    // Print a divider string to organize the console output readable format
    Serial.print(" | Distance: ");
    // Print the calculated distance value, rounded to 1 decimal place
    Serial.print(distanceMeters, 1);
    // Print the unit measurement label and move cursor to a new line
    Serial.println(" meters");

    // Check if the current calculated distance is less than or equal to the defined radius limit
    if (distanceMeters <= GEOFENCE_RADIUS_METERS) {
      // Print an alert message indicating the device is inside the geofence perimeter
      Serial.println("[STATUS] INSIDE Geofence boundary.");
      // Send power to the built-in LED pin to turn it on
      digitalWrite(LED_BUILTIN, HIGH); 
    } 
    // Execute this alternative block if the calculated distance is greater than the radius limit
    else {
      // Print a message indicating the device is outside the geofence perimeter
      Serial.println("[STATUS] OUTSIDE Geofence boundary.");
      // Cut power to the built-in LED pin to turn it off
      digitalWrite(LED_BUILTIN, LOW);  
    }
  } 
  // Execute this block if the GPS is active but has not resolved its location yet
  else {
    // Print a status message indicating it is searching for satellite signals
    Serial.print("Searching for Satellites... Current Tracked: ");
    // Print the number of satellites it can currently see while trying to lock
    Serial.println(gps.satellites.value());
    // Wait half a second before trying again to keep the serial stream readable
    delay(500);
  }
}