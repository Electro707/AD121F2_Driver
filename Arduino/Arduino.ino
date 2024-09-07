/**
 * AD121F2 Adapter Board driver
 * by Jamal Bouajjaj
 *
 * This firmware controls the IS31FL3239 controller, which is connected to the AD121F2 RGB 7-Segment Display
 *
 */
#include <Wire.h>

//#define ADDR 0b01101000
#define ADDR 0x34
#define SHUTDOWN_PIN 4
#define BRIGHTNESS 10

enum colorEnum_e{
    RED = 0x0000FF,
    GREEN = 0x00FF00,
    BLUE = 0xFF0000,
    YELLOW = 0x00FFFF,
    MAGENTA = 0xFF00FF,
    CYAN = 0xFFFF00,
    WHITE = 0xFFFFFF,
    BLACK = 0x000000,
};

// struct for individual RGB values
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b; 
}rgb_struct_t;

// A mapping between PWM registers and the segment's RGB LEDs
const rgb_struct_t segmentsRegs[7] = {
  {.r = 0x3D, .g = 0x37, .b = 0x35},
  {.r = 0x43, .g = 0x41, .b = 0x3F},
  {.r = 0x13, .g = 0x11, .b = 0x0B},
  {.r = 0x1D, .g = 0x17, .b = 0x15},
  {.r = 0x23, .g = 0x21, .b = 0x1F},
  {.r = 0x29, .g = 0x27, .b = 0x25},
  {.r = 0x33, .g = 0x31, .b = 0x2B}
};

const rgb_struct_t segmentSlRegs[7] = {
    {.r = 0x68, .g = 0x65, .b = 0x64},
    {.r = 0x6B, .g = 0x6A, .b = 0x69},
    {.r = 0x53, .g = 0x52, .b = 0x4F},
    {.r = 0x58, .g = 0x55, .b = 0x54},
    {.r = 0x5B, .g = 0x5A, .b = 0x59},
    {.r = 0x5E, .g = 0x5D, .b = 0x5C},
    {.r = 0x63, .g = 0x62, .b = 0x5F}
};

const uint32_t colorsList[] = {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE};


// An LUT between a PWM value, and the log of it. Should a more natural brightness increase
const uint8_t pwm_log_mapping[256] = {0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 36, 36, 37, 37, 38, 38, 39, 40, 40, 41, 42, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62, 62, 63, 64, 65, 66, 67, 68, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 79, 80, 81, 82, 83, 84, 85, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 99, 100, 101, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 116, 117, 118, 120, 121, 122, 124, 125, 126, 128, 129, 131, 132, 134, 135, 136, 138, 139, 141, 143, 144, 146, 147, 149, 150, 152, 154, 155, 157, 159, 160, 162, 164, 166, 167, 169, 171, 173, 175, 176, 178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 215, 217, 219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 242, 245, 247, 250, 252, 255};

// The current colors per segment
uint32_t current_color[7] = {0};


void setup() {
  // init periferlas
  pinMode(SHUTDOWN_PIN, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  randomSeed(analogRead(0));            // setup entropy
  
  // reset device, and pull driver out of shutdown
  writeRegister(0x7F, 0);
  digitalWrite(SHUTDOWN_PIN, HIGH);
  
  // Normal mode operation, 16Mhz clock, 8-bit PWM
  writeRegister(0x00, 0b00000001);
  
  // Set all LED's scaling register (DC current) to 0xFF
  for(int i=0x4C;i<=0x4F;i++){
    writeRegister(i, 0xFF);
  }
  for(int i=0x52;i<=0x55;i++){
    writeRegister(i, 0xFF);
  }
  for(int i=0x58;i<=0x5F;i++){
    writeRegister(i, 0xFF);
  }
  for(int i=0x62;i<=0x65;i++){
    writeRegister(i, 0xFF);
  }
  for(int i=0x68;i<=0x6B;i++){
    writeRegister(i, 0xFF);
  }

  // change global DC current
  writeRegister(0x6E, BRIGHTNESS);

  // short detection, not used?
//  writeRegister(0x71, 0b11);
//  readRegister(0x72);
//  readRegister(0x73);
//  readRegister(0x74);
}

void loop() {
    uint32_t color;
    // for(int i=0;i<7;i++){setSegmentColor(i, RED);}
    // fadeUpDown();
    // for(int i=0;i<7;i++){setSegmentColor(i, GREEN);}
    // fadeUpDown();
    // for(int i=0;i<7;i++){setSegmentColor(i, BLUE);}
    // fadeUpDown();
    // for(int i=0;i<7;i++){setSegmentColor(i, YELLOW);}
    // fadeUpDown();
    // for(int i=0;i<7;i++){setSegmentColor(i, MAGENTA);}
    // fadeUpDown();
    // for(int i=0;i<7;i++){setSegmentColor(i, CYAN);}
    // fadeUpDown();
    // for(int i=0;i<7;i++){setSegmentColor(i, WHITE);}
    // fadeUpDown();

    for(int i=0;i<8;i++){
        // get random color from pre-defined list
        // color = colorsList[random(sizeof(colorsList)/sizeof(uint32_t))];

        // random color
        // color = random(255);
        // color |= (random(255) << 8);
        // color |= (random(255) << 16);

        // color = HsvToRgb(random(360), 1.0, 0.8);
        // setSegment(i, color);

        setSegmentRandomColor(i);
        fadeUpDown();
    }

//    for(int i=0;i<7;i++){
//      writeRegister((uint8_t)*((uint8_t *)&segments[i]+0), 0);
//      writeRegister((uint8_t)*((uint8_t *)&segments[i]+1), 0);
//      writeRegister((uint8_t)*((uint8_t *)&segments[i]+2), 0);
//    }
//
//    for(int i=0;i<7;i++){
//      writeRegister((uint8_t)*((uint8_t *)&segments[i]+random(3)), 255);
//    }
//    writeRegister(0x49, 0);
//    delay(1000);

    // for(int i=0;i<7;i++){
    //   writeRegister(segments[i].r, pwm_log_mapping[random(255)]);
    //   writeRegister(segments[i].g, pwm_log_mapping[random(255)]);
    //   writeRegister(segments[i].b, pwm_log_mapping[random(255)]);
    // }
    // writeRegister(0x49, 0);
    // delay(1000);
}

// set segment colors to display a number
void setSegmentRandomColor(uint8_t n){
    // LUT between segments to color (as bits)
    uint32_t currentC;
    static uint8_t nToSegments[9] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111};
    
    for(int i=0;i<7;i++){
        if((nToSegments[n] >> i) & 1){
            currentC = HsvToRgb(random(360), 1.0, 0.8);;
        } else{
            currentC = 0;
        }
        setSegmentColor(i, currentC);
    }   
}

// set segment colors to display a number
void setSegment(uint8_t n, uint32_t color){
    // LUT between segments to color (as bits)
    uint32_t currentC;
    static uint8_t nToSegments[9] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111};
    
    for(int i=0;i<7;i++){
        if((nToSegments[n] >> i) & 1){
            currentC = color;
        } else{
            currentC = 0;
        }
        setSegmentColor(i, currentC);
    }   
}

// uses the scaling registers to fade, rather than modify PWM (color) values
void fadeUpDown(){
  for(int k=0;k<256;k++){
    for(int i=0;i<7;i++){
      writeRegister(segmentSlRegs[i].r, k);
      writeRegister(segmentSlRegs[i].g, k);
      writeRegister(segmentSlRegs[i].b, k);
    }
    update();
    delay(2);
  }

  for(int k=255;k>=0;k--){
    for(int i=0;i<7;i++){
      writeRegister(segmentSlRegs[i].r, k);
      writeRegister(segmentSlRegs[i].g, k);
      writeRegister(segmentSlRegs[i].b, k);
    }
    update();
    delay(2);
  }
}

void update(void){
  writeRegister(0x49, 0);
}

void setSegmentColor(uint8_t seg, uint32_t color){
  writeRegister(segmentsRegs[seg].r, color & 0xFF);
  writeRegister(segmentsRegs[seg].g, (color >> 8) & 0xFF);
  writeRegister(segmentsRegs[seg].b, (color >> 16) & 0xFF);
}

// Writes to the IS chip's register
void writeRegister(uint8_t reg, uint8_t data){
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

// Reads from the IS chip's register
void readRegister(uint8_t reg){
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, 1);
  Serial.print(reg, HEX); Serial.print(": ");
  while(Wire.available()){
    Serial.print(Wire.read(), HEX);
  }
  Serial.println("");
}

//copied from https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB
uint32_t HsvToRgb(uint32_t hue, float sat, float val){
    float r, g, b;
    uint32_t res;

    float chroma = val * sat;
    float h_s = hue / 60.0;
    float x = chroma * (1-fabs(fmod(h_s, 2) - 1));
    switch((uint8_t)h_s){
        case 0:
            r = chroma;
            g = x;
            b = 0;
            break;
        case 1:
            r = x;
            g = chroma;
            b = 0;
            break;
        case 2:
            r = 0;
            g = chroma;
            b = x;
            break;
        case 3:
            r = 0;
            g = x;
            b = chroma;
            break;
        case 4:
            r = x;
            g = 0;
            b = chroma;
            break;
        case 5:
            r = chroma;
            g = 0;
            b = x;
            break;
        default:
            return 0;
            break;
    }

    float m = val - chroma;
    r += m; g += m; b += m;
    r *= 255; g *= 255; b *= 255;

    Serial.println(hue);
    Serial.println(val);
    Serial.println(sat);
    Serial.println("-");
    Serial.println(h_s);
    Serial.println((uint8_t)h_s);
    Serial.println(chroma);
    Serial.println(x);
    Serial.println(m);
    Serial.println("-");
    Serial.println(r);
    Serial.println(g);
    Serial.println(b);
    Serial.println("---");

    res = (uint32_t)r;
    res |= ((uint32_t)g << 8);
    res |= ((uint32_t)b << 16);
    return res;
}