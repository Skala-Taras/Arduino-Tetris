
#include <LedControl.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>       
#include <Adafruit_SSD1306.h>  

#define DIN 12
#define CLK 11
#define CS 10
#define HIGH_SCORE_ADDR 0 
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  

LedControl lc = LedControl(DIN, CLK, CS, 2);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

short currentPieceType ;
short rotation = 0;
int positionX = 3;
int positionY = -4;  // Start above the board
unsigned long lastFall = 0;
unsigned long lastInput = 0;



bool gameIsOver;

int points = 0;
int highScore = 0;

byte gameBoard[16] = {0};
byte piece[4][4];

// Type falling piece "I" (line)
byte I_BLOCK[4][4] = {
    // Rotation 0:
    {0b10000000, 0b10000000, 0b10000000, 0b10000000},
    // Rotation 1: 
    {0b00000000, 0b00000000, 0b11110000, 0b00000000},
    // Rotation 2: 
    {0b10000000, 0b10000000, 0b10000000, 0b10000000},
    // Rotation 3: 
    {0b00000000, 0b00000000, 0b11110000, 0b00000000}
};

// Type falling piece "O" (square)
byte O_BLOCK[4][4] = {
    // All rotations are the same
    {0b00000000, 0b11000000, 0b11000000, 0b00000000},

    {0b00000000, 0b11000000, 0b11000000, 0b00000000},

    {0b00000000, 0b11000000, 0b11000000, 0b00000000},

    {0b00000000, 0b11000000, 0b11000000, 0b00000000},
};

// Type falling piece "T"
byte T_BLOCK[4][4] = {
    // Rotation 0: 
    {0b00000000, 0b11100000, 0b01000000, 0b00000000},
    // Rotation 1: 
    {0b01000000, 0b11000000, 0b01000000, 0b00000000},
    // Rotation 2: 
    {0b01000000, 0b11100000, 0b00000000, 0b00000000},
    // Rotation 3:
    {0b10000000, 0b11000000, 0b10000000, 0b00000000}
};

// Type falling piece "L"
byte  L_BLOCK[4][4] = {
    // Rotation 0: 
    {0b00000000, 0b11100000, 0b10000000, 0b00000000},
    // Rotation 1:
    {0b11000000, 0b01000000, 0b01000000, 0b00000000},
    // Rotation 2: 
    {0b00100000, 0b11100000, 0b00000000, 0b00000000},
    // Rotation 3: 
    {0b10000000, 0b10000000, 0b11000000, 0b00000000}
};

// Type falling piece "J"
byte J_BLOCK[4][4] = {
    // Rotation 0:
    {0b00000000, 0b11100000, 0b00100000, 0b00000000},
    // Rotation 1: 
    {0b01000000, 0b01000000, 0b11000000, 0b00000000},
    // Rotation 2: 
    {0b10000000, 0b11100000, 0b00000000, 0b00000000},
    // Rotation 3: 
    {0b11000000, 0b10000000, 0b10000000, 0b00000000}
};

// Type falling piece "S"
byte S_BLOCK[4][4] = {
    // Rotation 0:
    {0b00000000, 0b01100000, 0b11000000, 0b00000000},
    // Rotation 1: 
    {0b01000000, 0b01100000, 0b00100000, 0b00000000},
    // Rotation 2:
    {0b00000000, 0b01100000, 0b11000000, 0b00000000},
    // Rotation 3: 
    {0b01000000, 0b01100000, 0b00100000, 0b00000000}
};

// Type falling piece "Z"
byte Z_BLOCK[4][4] = {
    // Rotation 0: 
    {0b00000000, 0b11000000, 0b01100000, 0b00000000},
    // Rotation 1: 
    {0b00100000, 0b01100000, 0b01000000, 0b00000000},
    // Rotation 2: 
    {0b00000000, 0b11000000, 0b01100000, 0b00000000},
    // Rotation 3:
    {0b00100000, 0b01100000, 0b01000000, 0b00000000}
};

byte TEST[4][4] = {
    {0b10000000, 0b10000000, 0b10000000, 0b10000000},
    // Rotation 1: 
    {0b00000000, 0b00000000, 0b11110000, 0b00000000},
    // Rotation 2: 
    {0b10000000, 0b10000000, 0b10000000, 0b10000000},
    // Rotation 3: 
    {0b00000000, 0b00000000, 0b11110000, 0b00000000}
};



// Merges landed piece with game board
//> fallingPiece - 4-byte array representing piece rows
//> posX/Y - board coordinates of piece's top-left corner
void fixingFigureOnBoard(byte fallingPiece[4], short posX, short posY) {
  for (short row = 0; row < 4; row++) {

    short boardRow = posY + row;

    if (boardRow < 0 || boardRow >= 16) continue;

    if (fallingPiece[row] == 0) continue;

    gameBoard[boardRow] |= fallingPiece[row] >> posX;
  }
}

void displayGameOnLeds(byte fallingPiece[4], short posX, short posY) {
  byte tempBoard[16];
  memcpy(tempBoard, gameBoard, sizeof(tempBoard));
  
  for (short row = 0; row < 4; row++) {
    short boardRow = posY + row;
    if (boardRow < 0 || boardRow >= 16) continue;
    tempBoard[boardRow] |= fallingPiece[row] >> posX;
  }
  
  displayBoard(tempBoard);
}

void displayBoard(byte arr[16]) {
  for (short row = 0; row < 16; row++) {
    if (row < 8) {
      lc.setRow(0, row, arr[row]);
    } else {
      lc.setRow(1, row - 8, arr[row]);
    }
  }
}



// Collision detection system
//> Returns true if piece would collide with board boundaries or existing blocks
bool checkCollision(byte piece[4], short posX, short posY) {
  
  for (short row = 0; row < 4; row++) {

    byte pieceRow = piece[row];

    if (pieceRow == 0) continue;

    short boardRow = posY + row;

    if (boardRow >= 16) return true;
    if (boardRow < 0) continue;

    byte shifted = pieceRow >> posX;

    if (posX < 0 || (shifted << posX) != pieceRow) return true;
    if (gameBoard[boardRow] & shifted) return true;
  }
  return false;
}



// Row clearing with animation and score update
//> Returns number of cleared rows for scoring
int clearAndShiftRows() {
    int clearedRows = 0;
    bool rowsToClear[16] = {false}; // Tablica przechowująca informację, które wiersze mają być wyczyszczone

    // Oznaczamy wiersze do wyczyszczenia
    for (int row = 0; row < 16; row++) {
        if (gameBoard[row] == 0xFF) {
            rowsToClear[row] = true;
            clearedRows++;
        }
    }

    // Animacja gaszenia LED-ów
    for (int i = 0; i < 8; i++) {
        for (int row = 0; row < 16; row++) {
            if (rowsToClear[row]) {
                gameBoard[row] = gameBoard[row] >> 1;
            }
        }
        displayBoard(gameBoard);
        delay(100);
    }

    // Spadanie klocków - przesuwanie wierszy w dół
    int dstRow = 15; // Indeks, gdzie będziemy kopiować wiersze od dołu
    for (int srcRow = 15; srcRow >= 0; srcRow--) {
        if (!rowsToClear[srcRow]) {
            gameBoard[dstRow] = gameBoard[srcRow];
            dstRow--; 
        }
    }

    // Zerowanie górnych wierszy (które były przesunięte)
    for (int row = dstRow; row >= 0; row--) {
        gameBoard[row] = 0x00;
    }

    return clearedRows;
}



// Tetromino rotation handler with wall kick adjustments
//> positions array will store corrected [X,Y] values after rotation
//> tempRotation - proposed new rotation state to check
void correctionPositionPiece(
  short positionX, 
  short positionY, 
  short tempRotation,
  short* positions
  ) {

  switch (currentPieceType) {
    
    case 0:

      switch (tempRotation) {

        case 0: 
        case 2:
          // 0, 180 degrees
          positionX = positionX + 2;
          
          break;   

        case 1:
        case 3:
          // 90, 270 degrees
          positionX = positionX - 2;
          positionY = positionY - 1; 
          break;   
      }
      break;

    case 2:
      switch (tempRotation) {
        
        case 0:
          positionX = positionX - 1;
          break;
        
        case 3:
          // 270 degrees
          positionX++;
          break;
      }
      break;

    case 3:
    case 4:
      switch (tempRotation) {
        case 0:
          positionX--;
          break;
        
        case 3:
          // 270 degrees                     
          positionY--;
          positionX++;
          break;
      }
      break;
    
    break;
  
  }
  positions[0] = positionX;
  positions[1] = positionY;
}


// Handles piece rotation with wall kick adjustments
void handleRotation() {

  if (millis() - lastInput < 200) return;
  lastInput = millis();

  byte newPiece[4];
  short newRotation = rotation;
  if (newRotation >= 3) newRotation = 0;
  else newRotation++;

  memcpy(newPiece, piece[newRotation], 4);

  short positions[2];

  correctionPositionPiece(positionX, positionY, newRotation, positions);

  if (!checkCollision(newPiece, positions[0], positions[1])) {
    rotation = newRotation;
    positionX = positions[0];
    positionY = positions[1];
  }
}

void handleMovement(int dir) {
  if (millis() - lastInput < 150) return;
  lastInput = millis();
  
  int newX = positionX + dir;
  if (!checkCollision(piece[rotation], newX, positionY)) {
    positionX = newX;
  }
}



// Main game sequence manager
void gameLoop() {
  

  while (true) {
    positionX = 3;  
    positionY = -2;  
    rotation = 0;
    bool fastDrop = false;
    long unsigned timeWait;
    bool finishGame = false;

    currentPieceType = random(0, 7);
    definingNewPiece();

    if (checkCollision(piece[rotation], positionX, positionY)) {
      gameOver();
      break;
    }

    while (true) {
      if (digitalRead(7) == LOW) {  // END-GAME
        finishGame = true;
      }

      if (digitalRead(5) == LOW) handleMovement(-1); //LEFT
      if (digitalRead(3) == LOW) handleMovement(1);  //RIGHT
      if (digitalRead(4) == LOW) handleRotation();  //ROTATE

      if (digitalRead(6) == LOW) {  //DOWN
        fastDrop = true;
      } else {
        fastDrop = false;
      }

      //LVL GAME

      if (points <= 3 && points >= 0) timeWait = 500;

      else if (points > 3 && points <= 7) timeWait = 350;

      else if (points > 7 && points <= 12) timeWait = 200;
    
      else timeWait = 100;

      unsigned long currentWait = fastDrop ? timeWait / 7 : timeWait;

      if (millis() - lastFall > currentWait) {
        lastFall = millis();
        if (checkCollision(piece[rotation], positionX, positionY + 1)) {
          fixingFigureOnBoard(piece[rotation], positionX, positionY);

          int rowsCleared = clearAndShiftRows();
          points += rowsCleared;

          if (points > highScore) {
            highScore = points;
            EEPROM.put(HIGH_SCORE_ADDR, highScore);
          }
          updateScoreDisplay();

          break; // Przechodzimy do nowego klocka
        } else {
          positionY++;
        }
      }

      displayGameOnLeds(piece[rotation], positionX, positionY);
      updateScoreDisplay();
      delay(50);
    }

    if (finishGame) {
      gameOver();
      break;
    }
  }
}



// Defind new piece into piece[4][4] arr with rotation
void definingNewPiece() {
  switch (currentPieceType) {

    case 0: // "I" pieceTest
        memcpy(piece, I_BLOCK, sizeof(I_BLOCK));
        
        break;

    case 1: // "O" piece
        memcpy(piece, O_BLOCK, sizeof(O_BLOCK));
        break;

    case 2: // "T" piece
        memcpy(piece, T_BLOCK, sizeof(T_BLOCK));
        break;

    case 3: // "L" piece
        memcpy(piece, L_BLOCK, sizeof(L_BLOCK));
        break;

    case 4: // "J" piece
        memcpy(piece, J_BLOCK, sizeof(J_BLOCK));
        break;

    case 5: // "S" piece
        memcpy(piece, S_BLOCK, sizeof(S_BLOCK));
        break;

    case 6: // "Z" piece
        memcpy(piece, Z_BLOCK, sizeof(Z_BLOCK));
        break;
  }

}


void resetGame() {
    points = 0;
    memset(gameBoard, 0, sizeof(gameBoard));
    lc.clearDisplay(0);
    lc.clearDisplay(1);
    gameLoop();
}

void gameOver() {

    if (points > highScore) {
        highScore = points;
        EEPROM.put(HIGH_SCORE_ADDR, highScore);
    }

    // Czyszczenie ekranu przed wyświetleniem napisu GAME OVER
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE, BLACK); // Czyszczenie tła
    display.setCursor(20, 10);
    display.println("GAME");
    display.setCursor(20, 35);
    display.println("OVER");
    display.display();

    delay(2000);

    // Czyszczenie ekranu przed wyświetleniem wyników
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print("Final Score: ");
    display.println(points);
    
    display.setCursor(10, 35);
    display.print("High Score: ");
    display.println(highScore);
    
    display.display();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 2; j++) {
            lc.setColumn(j, i, B11111111);
        }
        delay(150);
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 2; j++) {
            lc.setColumn(j, i, B00000000);
        }
        delay(150);
    }
    
    while (true) {
      if (digitalRead(6) == LOW || digitalRead(5) == LOW || digitalRead(4) == LOW || digitalRead(3) == LOW) {
        break;
      }
        delay(100);
    }



    display.clearDisplay(); 
    resetGame();
}

void updateScoreDisplay() {

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE, BLACK);
    
    display.setCursor(10,10);
    display.print("Score: ");
    display.println(points);
    
    display.setCursor(10, 30);
    display.print("Record: ");
    display.println(highScore);
    
    display.display();
}


void setup() {
  Serial.begin(115200);
  lc.shutdown(0, false);
  lc.setIntensity(0, 1);
  lc.clearDisplay(0);
  lc.shutdown(1, false);
  lc.setIntensity(1, 1);
  lc.clearDisplay(1);

  EEPROM.get(HIGH_SCORE_ADDR, highScore);

  pinMode(7, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);


   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

  randomSeed(analogRead(A0));

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.println("Welcome");

  display.setCursor(15, 20);
  display.println("to TETRIS");

  display.setTextSize(2);
  display.setCursor(15, 40);
  display.println("from T. S.");
  display.display();
  delay(2000); 

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("Press");
  display.setCursor(10, 30);
  display.println("button");
  display.display();

  
  for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 2; j++) {
          lc.setColumn(j, i, B11111111);
      }
      delay(150);
  }
  for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 2; j++) {
          lc.setColumn(j, i, B00000000);
      }
      delay(150);
  }
  while (true) {
      if (digitalRead(6) == LOW || digitalRead(5) == LOW || digitalRead(4) == LOW || digitalRead(3) == LOW) {
        break;
      }
        delay(100);
    }
  
  display.display();  
  display.clearDisplay();
}

void loop() {

  gameLoop();

}

