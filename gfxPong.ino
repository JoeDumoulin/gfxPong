#include <stdlib.h>

#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_STMPE610.h"

// This is calibration data for the raw touch data to the screen coordinates
// Note that the screen is turned for the game 270 degrees.
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// touch screen object
// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// where is the pong?
int x = 0;
int y = 0;

int xDir = 1;
int yDir = 1;

int pongW = tft.width()/25;
int pongH = tft.height()/25;

int lastX = x;
int lastY = y;

// Where is the paddle?
int paddleH;
int paddleX;
int paddleY;
int paddleW;

bool gameOn = false;

int score = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("PongTest"); 
 
  // start the tft screen
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  
  // Start the touch screen
  ts.begin();
  
  // print the startup message
  drawText("Start Game", tft.width()/2 - 70, tft.height()/2 - 10, ILI9341_BLUE);
  
  delay(100);
}

void loop() {
  if (gameOn)
  {
    drawScore();
    drawPaddle();  
    drawPong();  
  }
  
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();

    // Scale from ~0->4000 to tft.width using the calibration #'s
    int pX = map(p.y, TS_MAXY, TS_MINY, 0, tft.width());
    int pY = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
    
    if (isin(pX, pY, paddleX, paddleY, paddleW, paddleH))
    {
      // here we want to move the paddle up and down the Y-direction
      //Serial.println("hit!");
      
      // move the paddle center to the point's y value if in bounds
      if (pY - paddleH/2 >= 0 && pY + paddleH/2 <= tft.height())
      {
        int lastPaddleY = paddleY;
        tft.fillRect(paddleX, paddleY, paddleW, paddleH, ILI9341_BLACK);
        
        paddleY = pY - paddleH/2;
        drawPaddle();
      }
    }
    
    if (!gameOn)
      resetGame();
      gameOn = true;

    //Serial.print("X = "); Serial.print(pX);
    //Serial.print("\tY = "); Serial.println(pY);
  }
  //delay(2);
}

bool isin(int pX, int pY, int padX, int padY, int padW, int padH)
{
  return pX >= padX && pX <= padX + padW
      && pY >= padY && pY <= padY + padH;
}

void resetGame()
{
  score = 0;
  resetPong();
  resetPaddle();
  tft.fillScreen(ILI9341_BLACK);
}

void resetPong()
{
  x = 0;
  y = 0;

  xDir = 1;
  yDir = 1;

  pongW = tft.width()/25;
  pongH = tft.height()/25;

  lastX = x;
  lastY = y;
}

void resetPaddle()
{
  paddleH = pongH*4;
  paddleX = tft.width() - pongW*3;
  paddleY = tft.height()/2 - paddleH/2;
  paddleW = pongW;
}

void drawPaddle()
{

  tft.fillRect(paddleX, paddleY, paddleW, paddleH, ILI9341_MAGENTA);
//  tft.fillRect(0, 0, pongW, paddleH, ILI9341_MAGENTA);
}

void drawPong(){
  // clearlast
  tft.fillRect(lastX, lastY, pongW, pongH, ILI9341_BLACK);
    
  // draw the pong
  tft.fillRect(x, y, pongW, pongH, ILI9341_MAGENTA);
  
  lastX = x;
  lastY = y;
  
  // calculate new position
  int xTry = x + xDir;
  int yTry = y + yDir;
  
  // See if we need to change directions or stop the game
//  if (xTry + pongW < tft.width() && xTry > 0)
  if (xTry + pongW < paddleX && xTry > 0)
    x = xTry;
  else
  {
    // should we turn around or stop?
    if (xTry <= 0)
    {
      xDir *= -1;
      x += xDir;
    }
    else if ((yTry + pongH >= paddleY - 2
        && yTry <= paddleY + paddleH + 2))
    {
      Serial.print(yTry); Serial.println(paddleY);
      ++score;
      drawScore();
      xDir *= -1;
      x += xDir;
    }
    else
    {
      gameOn = false;
      //resetGame();
      drawText("Game Over", tft.width()/2 - 70, tft.height()/2 - 10, ILI9341_BLUE);
    }
  }
  if (yTry + pongH < tft.height() && yTry > 0)
    y = yTry;
  else
  {
    yDir *= -1;
    y += yDir;
  }  
}

void drawScore()
{
  char buff[7];
  if (score >0)
  {
    itoa(score-1, buff, 10);
    drawText(buff, 0, 0, ILI9341_BLACK);
  }
  itoa(score, buff, 10);
  drawText(buff, 0, 0, ILI9341_BLUE);
}

void drawText(char* text, int posX, int posY, unsigned color) {
  unsigned long start = micros();
  tft.setCursor(posX, posY);
  tft.setTextColor(color);  tft.setTextSize(2);
  tft.println(text);  
}

void drawPongOld(){
  int w = tft.width();
  int h = tft.height();
  
  int cx = w/2-1;
  int cy = h/2-1;
  
  tft.fillRect(cx, cy, w/25, w/25, ILI9341_MAGENTA);

}

