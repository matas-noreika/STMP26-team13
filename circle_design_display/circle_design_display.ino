#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <math.h> // For movement and distance calculations

// --- PIN DEFINITIONS FOR TENSTAR TS-ESP32-S3 (ADAFRUIT FEATHER CLONE) ---
#define TFT_POWER_EN  21  // CRITICAL: Enables physical power to the screen and sensors
#define TFT_MOSI      35  // SPI Data Out
#define TFT_SCLK      36  // SPI Clock
#define TFT_CS        7   // Chip Select
#define TFT_DC        39  // Data/Command
#define TFT_RST       40  // Reset
#define TFT_BL        45  // Backlight PWM

// Standard resolution for 1.14" ST7789 TFT screens
#define TFT_WIDTH  135
#define TFT_HEIGHT 240

// Instantiate the screen referencing the default hardware SPI bus
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);

// --- GEOFENCING PARAMETERS ---
int fenceCenterX;
int fenceCenterY;
int fenceRadius = 55; // Radius of the geofencing zone in pixels

// --- USER SIMULATION PARAMETERS ---
float userX;
float userY;
float userPrevX;
float userPrevY;
float angle = 0.0;    // Angle for movement simulation
float userRadius = 5; // Radius of the dot representing the user
float moveRadius;     // User movement radius (can exceed fenceRadius)

// Track previous state to optimize screen updates
bool wasInside = true; 

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting Tenstar Geofencing Visual Test with Clamped Bounds..."));

  // 1. CRITICAL: Enable physical power to the screen and sensors
  pinMode(TFT_POWER_EN, OUTPUT);
  digitalWrite(TFT_POWER_EN, HIGH);
  delay(10); // Allow power to stabilize

  // 2. Enable Backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // 3. Initialize Custom Hardware SPI for this board
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // 4. Initialize the TFT panel
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  
  // Set orientation (0, 1, 2, or 3) - landscape orientation works well for this test
  tft.setRotation(3);

  // --- INITIAL VISUAL SETUP ---
  
  // Clear the screen
  tft.fillScreen(ST77XX_BLACK);

  // Calculate the center based on landscape orientation
  fenceCenterX = tft.width() / 2;
  fenceCenterY = tft.height() / 2;

  // Initialize the previous user position so it can be erased efficiently
  userPrevX = fenceCenterX;
  userPrevY = fenceCenterY;
}

void loop() {
  // --- USER MOVEMENT SIMULATION ---
  
  // Increase the angle to create a circular trajectory
  angle += 0.06;
  
  // Calculate an oscillating distance from the center to move in and out of the zone
  float distanceMultiplier = 0.8 + 0.5 * sin(angle * 1.2); 
  moveRadius = fenceRadius * distanceMultiplier;

  // Calculate new user coordinates before clamping them
  float nextX = fenceCenterX + cos(angle) * moveRadius;
  float nextY = fenceCenterY + sin(angle) * moveRadius;

  // --- FIX: Boundary Clamping ---
  // This line forces userX and userY to stay inside the screen boundaries.
  // constrain(value, minimum, maximum)
  userX = constrain(nextX, userRadius, tft.width() - userRadius);
  userY = constrain(nextY, userRadius, tft.height() - userRadius);


  // Calculate distance from the center to the new user position (using clamped coordinates)
  float distToCenter = sqrt(pow(userX - fenceCenterX, 2) + pow(userY - fenceCenterY, 2));
  bool isInside = (distToCenter <= fenceRadius);

  // --- DYNAMIC VISUAL UPDATE ---

  // 1. Erase the previous user position by drawing a black circle over it
  tft.fillCircle(userPrevX, userPrevY, userRadius + 1, ST77XX_BLACK);
  
  // 2. REDRAW BACKGROUND ELEMENTS (Prevents them from disappearing)
  
  // Redraw Geofence
  uint16_t fenceColor = isInside ? ST77XX_BLUE : ST77XX_RED;
  tft.drawCircle(fenceCenterX, fenceCenterY, fenceRadius, fenceColor);
  
  // Redraw Center Marker
  tft.fillCircle(fenceCenterX, fenceCenterY, 2, ST77XX_WHITE);

  // Redraw Warning Border
  if (!isInside) {
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_RED);
    tft.drawRect(1, 1, tft.width()-2, tft.height()-2, ST77XX_RED);
  }

  // 3. Draw the new user position (now strictly inside the screen)
  uint16_t userColor = isInside ? ST77XX_GREEN : ST77XX_YELLOW;
  tft.fillCircle(userX, userY, userRadius, userColor);

  // --- STATUS TEXT UPDATE ---
  
  // Clear the text area (small black rectangle in the corner) only if the state changed
  if (isInside != wasInside || angle == 0.06) { 
    
    // Clear text area
    tft.fillRect(0, tft.height() - 20, 120, 20, ST77XX_BLACK);
    
    // Set cursor and text color
    tft.setCursor(5, tft.height() - 15);
    tft.setTextSize(1);
    
    if (isInside) {
      // If the user moves back inside, erase the red warning border
      tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_BLACK);
      tft.drawRect(1, 1, tft.width()-2, tft.height()-2, ST77XX_BLACK);
      
      tft.setTextColor(ST77XX_GREEN);
      tft.print("STATUS: INSIDE");
    } else {
      tft.setTextColor(ST77XX_RED);
      tft.print("STATUS: OUTSIDE!");
    }
    
    wasInside = isInside;
  }

  // Update previous position for the next round
  userPrevX = userX;
  userPrevY = userY;

  // Simulation speed control
  delay(40);
}