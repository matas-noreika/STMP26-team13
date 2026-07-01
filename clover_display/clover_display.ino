#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <math.h>

// --- PIN DEFINITIONS FOR TENSTAR TS-ESP32-S3 (ADAFRUIT FEATHER CLONE) ---
#define TFT_POWER_EN  21  
#define TFT_MOSI      35  
#define TFT_SCLK      36  
#define TFT_CS        7   
#define TFT_DC        39  
#define TFT_RST       40  
#define TFT_BL        45  

// Standard resolution for 1.14" ST7789 TFTs
#define TFT_WIDTH  135
#define TFT_HEIGHT 240

// Instantiate the display
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);

// --- GEOFENCING PARAMETERS ---
int fenceCenterX;
int fenceCenterY;
float maxFenceRadius = 55.0; // The target maximum radius of the 4-leaf clover

// --- USER SIMULATION PARAMETERS ---
float userX;
float userY;
float userPrevX;
float userPrevY;
float angle = 0.0;    
float userRadius = 5; 
float moveRadius;     

// Track previous state to optimize screen updates
bool wasInside = true; 

// --- CUSTOM FUNCTION TO DRAW A 4-LEAF HEART CLOVER ---
void drawLuckyHeartClover(int cx, int cy, float maxR, uint16_t color) {
  float step = PI / 90.0; // High resolution for smooth, rounded heart curves
  
  // Scale adjustment so the mathematical shape visually matches maxFenceRadius
  // The max value of our polar function is roughly 0.72, so we scale by ~1.35
  float scale = maxR * 1.35; 
  
  int firstX = -1, firstY = -1;
  int prevX = -1, prevY = -1;

  for (float a = 0; a <= 2 * PI + step; a += step) {
    // Rotate the shape by 45 degrees (PI/4) for the classic "X" orientation
    float theta = a - PI / 4.0;
    
    // The magic math for a 4-leaf heart clover:
    // 1. abs(cos(2 * theta)) creates 4 rounded petals.
    // 2. (1.0 - 0.4 * cos(4 * theta)) carves a smooth "heart dip" into each petal.
    float baseLeaf = abs(cos(2.0 * theta));
    float heartDip = 1.0 - 0.4 * cos(4.0 * theta);
    
    float r = scale * baseLeaf * heartDip;

    // Convert polar to cartesian coordinates
    int px = cx + r * cos(a);
    int py = cy + r * sin(a);

    if (prevX == -1) {
      firstX = px; 
      firstY = py;
    } else {
      tft.drawLine(prevX, prevY, px, py, color);
    }
    
    prevX = px; 
    prevY = py;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting 4-Leaf Lucky Heart Clover Geofencing..."));

  // 1. Turn on physical power to the TFT
  pinMode(TFT_POWER_EN, OUTPUT);
  digitalWrite(TFT_POWER_EN, HIGH);
  delay(10); 

  // 2. Enable the Backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // 3. Initialize Custom Hardware SPI
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // 4. Initialize the TFT panel (Landscape)
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(3);

  // Clear the screen
  tft.fillScreen(ST77XX_BLACK);

  // Calculate center
  fenceCenterX = tft.width() / 2;
  fenceCenterY = tft.height() / 2;

  // Initialize previous user position
  userPrevX = fenceCenterX;
  userPrevY = fenceCenterY;
}

void loop() {
  // --- USER MOVEMENT SIMULATION ---
  angle += 0.05;
  
  // Calculate a distance that oscillates so the user traces in and out
  float distanceMultiplier = 0.6 + 0.6 * sin(angle * 1.3); 
  moveRadius = maxFenceRadius * distanceMultiplier;

  // Calculate new target coordinates
  float nextX = fenceCenterX + cos(angle) * moveRadius;
  float nextY = fenceCenterY + sin(angle) * moveRadius;

  // Clamp user position to screen boundaries
  userX = constrain(nextX, userRadius, tft.width() - userRadius);
  userY = constrain(nextY, userRadius, tft.height() - userRadius);

  // --- LUCKY CLOVER GEOFENCE COLLISION DETECTION ---
  float dy = userY - fenceCenterY;
  float dx = userX - fenceCenterX;
  
  // Find the exact angle of the user relative to the center
  float userAngle = atan2(dy, dx); 
  
  // Apply the same 45-degree rotation offset used when drawing
  float theta = userAngle - PI / 4.0;
  
  // Calculate the exact allowed radius at this angle using the heart formula
  float scale = maxFenceRadius * 1.35;
  float baseLeaf = abs(cos(2.0 * theta));
  float heartDip = 1.0 - 0.4 * cos(4.0 * theta);
  
  float allowedRadius = scale * baseLeaf * heartDip;
  float distToCenter = sqrt(dx*dx + dy*dy);
  
  // Check if the user is inside the heart leaf
  bool isInside = (distToCenter <= allowedRadius);

  // --- DYNAMIC VISUAL UPDATE ---

  // Erase the previous user position
  tft.fillCircle(userPrevX, userPrevY, userRadius + 1, ST77XX_BLACK);
  
  // Redraw the Lucky Clover (BLUE if inside, RED if outside)
  uint16_t fenceColor = isInside ? ST77XX_BLUE : ST77XX_RED;
  drawLuckyHeartClover(fenceCenterX, fenceCenterY, maxFenceRadius, fenceColor);
  
  // Redraw Center Marker (Magenta)
  tft.fillCircle(fenceCenterX, fenceCenterY, 3, ST77XX_MAGENTA);

  // Redraw Warning Border if outside
  if (!isInside) {
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_RED);
    tft.drawRect(1, 1, tft.width()-2, tft.height()-2, ST77XX_RED);
  }

  // Draw the new user position
  uint16_t userColor = isInside ? ST77XX_GREEN : ST77XX_YELLOW;
  tft.fillCircle(userX, userY, userRadius, userColor);

  // --- ALERT SYSTEM & TEXT UPDATE ---
  if (isInside != wasInside || angle == 0.05) { 
    
    tft.fillRect(0, tft.height() - 20, 120, 20, ST77XX_BLACK);
    tft.setCursor(5, tft.height() - 15);
    tft.setTextSize(1);
    
    if (isInside) {
      // Erase red warning border
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

  // Update previous position for the next frame
  userPrevX = userX;
  userPrevY = userY;

  delay(40);
}