#include <Adafruit_NeoPixel.h>

//******piezo stuff
int piezoThreshold1 = 500;
int piezoThreshold2 = 100;
int input1state = 0;
int input2state = 0;
int input1states[] = {-1,-1,-1,-1,-1};
int input2states[] = {-1,-1,-1,-1,-1};
int input1average = -1;
int input2average = -1;
//******

const float Pi = 3.14159;
const int ledAmount = 144;
const float speedUpValue = 0.9f;

int curLedIndex = 0;
int delayAmount = 1;
int delayDefault = 700;
int ballSpeed = 700;

bool indexUp = true;
bool GameOver = false;
bool whoWon = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledAmount, 7, NEO_GRB + NEO_KHZ800);

int shotCounter = 1;
int shotType = 2;
int timer = 0;
int hitIndex = 0;
int hitThreshold = 0;
int zenThreshold = 5;
boolean zenMode = false;
int shotColors[] = {0,100,0, /**/ 0,100,25, /**/ 100,25,50};

int threshold[] = {4,8,12};
int thesholdColors[] = {100,0,0, /**/ 100,35,0, /**/ 100,90,0};
int tail = ledAmount - 10;

float zenColor[] = {50,50,50};
float zenTargetColor[] = {50,50,50};
int fadeSpeed = 10;
int fadeIndex = 0;

int endTime;
int endTimeDelay=500;

//score
int playerScores[] = {0,0};

void setup() {

  Serial.begin(9600);
  strip.begin();
  
  strip.setPixelColor(2, 0, 100, 0);
  strip.show();
  
  delay(delayAmount);
  
}

void restart(){
  input1state = 0;
  input2state = 0;
  for (int i=0; i<5; i++){
    input1states[i]=-1;
    input2states[i]=-1;
  }
  input1average = -1;
  input2average = -1;

  curLedIndex = 0;

  shotCounter = 1;
  shotType = 2;
  timer = 0;
  hitThreshold = 0;
  zenThreshold = 5;

  fadeSpeed = 10;
  fadeIndex = 0;

  if(whoWon){
    indexUp = true;
    curLedIndex =threshold[2];
  }else{
    indexUp = false;
    curLedIndex = ledAmount - threshold[2];
  }
  hitIndex = curLedIndex; 
  GameOver = false;
  zenMode = false;
  whoWon = false;
  ballSpeed = delayDefault;
  endTime = 0;
}

void loop() {
  timer++;
  if (GameOver){
    endTime++;
  }
  
  input1state = analogRead(A0);
  input2state = analogRead(A1);

  //add current value to list to average
  for (int i=1; i<5; i++){
    input1states[i-1]=input1states[i];
    input2states[i-1]=input2states[i];
  }
  input1states[4] = input1state;
  input2states[4] = input2state;

//  Serial.print(input1average);
//  Serial.print("\t");
//  Serial.print(input2average);
//  Serial.print("\t");

  //check if it's a hit
  int old1avg = input1average;
  input1average = averageOfValues(input1states);
  if (input1average >= 0){
    if (abs(old1avg-input1state)>piezoThreshold1){
      if (GameOver && whoWon && endTime > endTimeDelay){
        restart();
      }else{
        hit(0,input1state,piezoThreshold1);
      }
//      Serial.print("input 1");
//      Serial.print("\t");
//      Serial.println(input1average);
    }else{
//      Serial.print(0);
//      Serial.print("\t");
    }
  }
  int old2avg = input2average;
  input2average = averageOfValues(input2states);
  if (input2average >= 0){
    if (abs(old2avg-input2state)>piezoThreshold2){
      if (GameOver && !whoWon && endTime > endTimeDelay){
        restart();
      }else{
        hit(1,input2state,piezoThreshold2);
      }
//      Serial.print("input 2");
//      Serial.print("\t");
//      Serial.println(input2average);
    }else{
//      Serial.print(0);
    }
  }

  //draw all the lights
  //reset the board
  float brightness = 0;
  for (int i=0; i<ledAmount; i++){
    strip.setPixelColor(i, brightness,brightness,brightness);
  }

//show hit areas
  for (int i=0; i<3; i++){
    strip.setPixelColor(threshold[i], shotColors[0+i*3],shotColors[1+i*3],shotColors[2+i*3]);
    strip.setPixelColor(ledAmount-1-threshold[i], shotColors[0+i*3],shotColors[1+i*3],shotColors[2+i*3]);
  }
  
  
  //determine which color based on what direction its going
  float color[] = {brightness,brightness,brightness};
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
      if(curLedIndex - i > hitIndex){
        strip.setPixelColor(max(0,curLedIndex-i),f*color[0],f*color[1],f*color[2]);
      }
    }else{
      if(curLedIndex + i < hitIndex){
        strip.setPixelColor(min(ledAmount-1,curLedIndex+i),f*color[0],f*color[1],f*color[2]);
      }
    }
  }

  strip.setPixelColor(0, 100,0,0);
  strip.setPixelColor(ledAmount-1, 100,0,0);

  if (GameOver){
    for (int i=0; i<ledAmount; i++){
      strip.setPixelColor(i,0,0,0);
    }
    for (int i=0; i<playerScores[0]+1; i++){
      if (whoWon){
        strip.setPixelColor(i+1,0,100,0);
      }else{
        strip.setPixelColor(i+1,100,0,0);
      }
    }
    for (int i=0; i<playerScores[1]+1; i++){
      if (!whoWon){
        strip.setPixelColor(ledAmount-2-i,0,100,0);
      }else{
        strip.setPixelColor(ledAmount-2-i,100,0,0);
      }
    }
    
    if (endTime > endTimeDelay){
        strip.setPixelColor(0,50,50,50);
        strip.setPixelColor(ledAmount-1,50,50,50);
    }
  }
  
  strip.show();

  if (!GameOver){
    moveball();
    if (curLedIndex > ledAmount-1 && indexUp){
      Serial.println("gameover");
      GameOver = true;
      hitIndex = curLedIndex;
      whoWon = true;
      playerScores[0]++;
      endTime = timer;
    }
    if (curLedIndex < 0 && !indexUp){
      Serial.println("gameover");
      GameOver = true;
      hitIndex = curLedIndex;
      whoWon = false;
      playerScores[1]++;
      endTime = timer;
    }
  }
  
  delay(delayAmount);
    
}


//called when a player hits a button/ball/whatever
void hit(int player, int power, int piezoThreshold){
  //for each individual threshold, check if that's where the player hit
  //and send that info to a switch statement that will determine which type of hit it is
  //but also just changes direction of the ball
  for (int i=0; i<3; i++){
    if (player == 0 && !indexUp){
      if (curLedIndex <= threshold[i]){
        indexUp = true;
        hitIndex = curLedIndex;
        hitThreshold = curLedIndex;
        HitCounter(i,power, piezoThreshold);
        break;
      }
    }else if (player==1 && indexUp){
      if (curLedIndex >= ledAmount-threshold[i]){
//        direction = -1;
        indexUp = false;
        hitIndex = curLedIndex;
        hitThreshold = ledAmount - curLedIndex;
        HitCounter(i,power, piezoThreshold);
        
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

void HitCounter(int i, int power, int piezoThreshold){
  timer = 0;
  shotType = i;
  float change = max(float(power - piezoThreshold), 800) / 800;
  change *= 50;

  Serial.print("CHANGE COEFFICIENT!!!\t");
  Serial.println(change);
  if((i == 1 || i == 0 || i == 2)){
    if (!zenMode){
      shotCounter ++;
      ballSpeed -= change;
      if(ballSpeed < 150){
        ballSpeed = 150;
      }
    }
    if (zenMode){
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //this speeds up the game indefinitely
      ballSpeed *= 0.95;
    }
  }else{
    zenMode = false;
    shotCounter = 1;
    ballSpeed = delayDefault;
  }
}


void moveball(){
  
  int endPoint;
  if(indexUp){
    endPoint = ledAmount - hitThreshold + 1;
//    if(shotType == 2){
//      endPoint -= 2;
//    }
  }else{
    endPoint = hitThreshold - ledAmount - 1;
//    if(shotType == 2){
//      endPoint += 2;
//    }
  }
 
  
  switch(shotType){
    case 0:
      if (zenMode){
        curLedIndex = (int)Circ_easeOut(timer, hitIndex, endPoint, (ballSpeed/2)/delayAmount);
      }else{
        curLedIndex = (int)Circ_easeOut(timer, hitIndex, endPoint, (ballSpeed/2)/delayAmount);
      }
    break;
    case 1:
      if (zenMode){
        curLedIndex = (int)Linear_easeNone(timer, hitIndex, endPoint, (ballSpeed/2)/delayAmount);
      }else{
        curLedIndex = (int)Linear_easeNone(timer, hitIndex, endPoint, (ballSpeed/2)/delayAmount);
      }
      
    break;
    case 2:
      curLedIndex = (int)Expo_easeOut(timer, hitIndex, endPoint, ballSpeed/delayAmount);
    break;
  }

}

int averageOfValues(int vals[]){
  int total = -1;
  int count = 0;
  for (int i=0; i<5; i++){
    if (vals[i] >= 0){
      count++;
      total += vals[i];
    }
  }
  if (total<0 && count>0){
    total = 0;
  }
  if (total<0 && count<1){
    return -1;
  }
  return total/count;
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
