#include <Adafruit_NeoPixel.h>

//******piezo stuff
int piezoThreshold = 50;

//******

const float Pi = 3.14159;
const int ledAmount = 61;
const float speedUpValue = 0.9f;

int curLedIndex = 0;
int delayAmount = 150;
int delayDefault = 150;
bool indexUp = true;
bool GameOver = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledAmount, 7, NEO_GRB + NEO_KHZ800);

int input1State = 0;       
int input2State = 0;

int shotCounter = 1;
int shotType = 0;
int timer = 0;
int hitIndex = 0;
int hitThreshold = 0;
int zenThreshold = 5;
boolean zenMode = false;
int shotColors[] = {0,100,0, /**/ 0,100,25, /**/ 100,25,50};

int threshold[] = {1,3,5};
int thesholdColors[] = {100,0,0, /**/ 100,35,0, /**/ 100,90,0};
int tail = 5;

float zenColor[] = {50,50,50};
float zenTargetColor[] = {50,50,50};
int fadeSpeed = 10;
int fadeIndex = 0;

void setup() {

  Serial.begin(9600);
  strip.begin();
  
  strip.setPixelColor(2, 0, 100, 0);
  strip.show();
  
  delay(delayAmount);
  
}

void loop() {
  timer++;
  
  input1State = analogRead(A0);
  input2State = analogRead(A1);
  
  if (input1State > piezoThreshold || input2State > piezoThreshold){
    Serial.print(input1State);
    Serial.print("\t");
    Serial.println(input2State);
  }
  
  if (input1State > piezoThreshold){
    hit(0,input1State);
  }
  
  if (input2State > piezoThreshold){
    hit(1,input2State);
  }


  //--------for zen mode
  if (zenMode){
    fadeIndex++;
    zenColor[0] = zenColor[0] + (zenTargetColor[0]-zenColor[0])*(float(fadeIndex)/float(fadeSpeed));
    zenColor[1] = zenColor[1] + (zenTargetColor[1]-zenColor[1])*(float(fadeIndex)/float(fadeSpeed));
    zenColor[2] = zenColor[2] + (zenTargetColor[2]-zenColor[2])*(float(fadeIndex)/float(fadeSpeed));
    if (fadeIndex > fadeSpeed){
      generateRandomColor();
      fadeIndex = 0;
    }
  }

  //draw all the lights
  float brightness = (float(shotCounter)/float(zenThreshold))*50;
  for (int i=0; i<ledAmount; i++){
//    brightness = 0;
    if (GameOver){
      strip.setPixelColor(i, 100,0,0);
    }else if(zenMode){
      strip.setPixelColor(i, zenColor[0],zenColor[1],zenColor[2]);
    }else{
      strip.setPixelColor(i, brightness,brightness,brightness);
    }
  }

  for (int i=0; i<3; i++){
    strip.setPixelColor(threshold[i], thesholdColors[0+i*3],thesholdColors[1+i*3],thesholdColors[2+i*3]);
    strip.setPixelColor(ledAmount-1-threshold[i], thesholdColors[0+i*3],thesholdColors[1+i*3],thesholdColors[2+i*3]);
  }
  
  
  //determine which color based on what direction its going
  float color[] = {brightness,brightness,brightness};
//  if(indexUp){
//    color[1] = 100;
//  }else{
//    color[2] = 100;
//  }
  color[0] = shotColors[3*shotType];
  color[1] = shotColors[1+3*shotType];
  color[2] = shotColors[2+3*shotType];
  
  //draw the ball
  strip.setPixelColor(curLedIndex, color[0],color[1],color[2]);
  
  //tail drawing
  for (int i=shotCounter; i>0; i--){
    float f = 1 - float(i)/float(shotCounter) + 0.1;
    f = f * 0.5;
    if (indexUp){
      strip.setPixelColor(max(0,curLedIndex-i),f*color[0],f*color[1],f*color[2]);
    }else{
      strip.setPixelColor(min(ledAmount-1,curLedIndex+i),f*color[0],f*color[1],f*color[2]);
    }
  }
  
  strip.show();

  if (!GameOver){
    moveball();
    if (curLedIndex > ledAmount-1 && indexUp){
      GameOver = true;
    }
    if (curLedIndex < 0 && !indexUp){
      GameOver = true;
    }
  }
  
  delay(50);//delayAmount);
    
}


//called when a player hits a button/ball/whatever
void hit(int player, int power){
  //for each individual threshold, check if that's where the player hit
  //and send that info to a switch statement that will determine which type of hit it is
  //but also just changes direction of the ball
  for (int i=0; i<3; i++){
    if (player == 0 && !indexUp){
      if (curLedIndex <= threshold[i]){
        indexUp = true;
        hitIndex = curLedIndex;
        hitThreshold = curLedIndex;
        HitCounter(i,power);
        break;
      }
    }else if (player==1 && indexUp){
      if (curLedIndex >= ledAmount-threshold[i]){
//        direction = -1;
        indexUp = false;
        hitIndex = curLedIndex;
        hitThreshold = ledAmount - curLedIndex;
        HitCounter(i,power);
        
        break;
      }
    }
  }
}

void generateRandomColor(){
  //generate random colors
  zenTargetColor[0] = random(20);
  zenTargetColor[1] = random(20-zenTargetColor[0]);
  zenTargetColor[2] = (20-zenTargetColor[0]-zenTargetColor[1]);
  //make them more pastel
  zenTargetColor[0] += 15;
  zenTargetColor[1] += 15;
  zenTargetColor[2] += 15;
}

void HitCounter(int i, int power){
  timer = 0;
  shotType = i;
  float change = max(float(power - piezoThreshold),800) / 800;
  change *= 0.25;
  change = 0.95 - change;
  Serial.print("CHANGE COEFFICIENT!!!\t");
  Serial.println(change);
  if((i == 1 || i == 0) && !zenMode){
    shotCounter ++;
    delayAmount *= change;
    if(shotCounter >= zenThreshold){
      zenMode = true;
      delayAmount = delayDefault * 0.9;
      fadeIndex = 0;
      generateRandomColor();
      zenColor[0]=50; zenColor[1]=50; zenColor[2]=50;
    }
  }else{
    zenMode = false;
    shotCounter = 1;
    delayAmount = delayDefault;
  }
}


void moveball(){
  
  int endPoint;
  if(indexUp){
    endPoint = ledAmount - hitThreshold;
    if(shotType == 2){
      endPoint -= 2;
    }
  }else{
    endPoint = hitThreshold - ledAmount;
    if(shotType == 2){
      endPoint += 2;
    }
  }
 
  
  switch(shotType){
    case 0:
      curLedIndex = (int)Circ_easeInOut(timer, hitIndex, endPoint, 0.01*delayAmount * ledAmount);
    break;
    case 1:
      curLedIndex = (int)Linear_easeNone(timer, hitIndex, endPoint, 0.01*delayAmount * ledAmount);
    break;
    case 2:
      curLedIndex = (int)Bounce_easeOut(timer, hitIndex, endPoint, 0.01*delayAmount * ledAmount);
    break;
  }

}

//***********EASING FUNCTIONS************************************************

//class Circ {
  
  float  Circ_easeIn(float t,float b , float c, float d) {
    return -c * ((float)sqrt(1 - (t/=d)*t) - 1) + b;
  }
  
  float  Circ_easeOut(float t,float b , float c, float d) {
    return c * (float)sqrt(1 - (t=t/d-1)*t) + b;
  }
  
  float  Circ_easeInOut(float t,float b , float c, float d) {
    if ((t/=d/2) < 1) return -c/2 * ((float)sqrt(1 - t*t) - 1) + b;
    return c/2 * ((float)sqrt(1 - (t-=2)*t) + 1) + b;
  }

//};

//class Bounce {
  
  float  Bounce_easeIn(float t,float b , float c, float d) {
    return c - Bounce_easeOut (d-t, 0, c, d) + b;
  }
  
  float  Bounce_easeOut(float t,float b , float c, float d) {
    if ((t/=d) < (1/2.75f)) {
      return c*(7.5625f*t*t) + b;
    } else if (t < (2/2.75f)) {
      return c*(7.5625f*(t-=(1.5f/2.75f))*t + .75f) + b;
    } else if (t < (2.5/2.75)) {
      return c*(7.5625f*(t-=(2.25f/2.75f))*t + .9375f) + b;
    } else {
      return c*(7.5625f*(t-=(2.625f/2.75f))*t + .984375f) + b;
    }
  }
//};
  
//class Linear {
  
  float Linear_easeNone (float t,float b , float c, float d) {
    return c*t/d + b;
  }
  
//};
  
//class Sine {
  
  float  Sine_easeIn(float t,float b , float c, float d) {
    return -c * (float)cos(t/d * (Pi/2)) + c + b;
  }
  
  float  Sine_easeOut(float t,float b , float c, float d) {
    return c * (float)sin(t/d * (Pi/2)) + b;  
  }
  
  float  Sine_easeInOut(float t,float b , float c, float d) {
    return -c/2 * ((float)cos(Pi*t/d) - 1) + b;
  }
  
//};

//class Expo {
  
  float  Expo_easeIn(float t,float b , float c, float d) {
    return (t==0) ? b : c * (float)pow(2, 10 * (t/d - 1)) + b;
  }
  
  float  Expo_easeOut(float t,float b , float c, float d) {
    return (t==d) ? b+c : c * (-(float)pow(2, -10 * t/d) + 1) + b;  
  }
  
  float  Expo_easeInOut(float t,float b , float c, float d) {
    if (t==0) return b;
    if (t==d) return b+c;
    if ((t/=d/2) < 1) return c/2 * (float)pow(2, 10 * (t - 1)) + b;
    return c/2 * (-(float)pow(2, -10 * --t) + 2) + b;
  }

//};


//class Cubic {
  
  float Cubic_easeIn (float t,float b , float c, float d) {
    return c*(t/=d)*t*t + b;
  }
  
  float Cubic_easeOut (float t,float b , float c, float d) {
    return c*((t=t/d-1)*t*t + 1) + b;
  }
  
  float Cubic_easeInOut (float t,float b , float c, float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t + b;
    return c/2*((t-=2)*t*t + 2) + b;
  }

//};
