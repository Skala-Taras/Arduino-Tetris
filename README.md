Overview

    This code implements a Tetris game using an 8x16 LED matrix for gameplay visualization and an SSD1306 OLED display for score tracking. It supports piece rotation, movement, scoring, and high score persistence     via EEPROM.

![photo_5253834361789345647_y](https://github.com/user-attachments/assets/aad1ccfc-239b-43e1-b1ee-3c95156c076b)

Hardware Setup

    LED Matrix: Uses 2 MAX72xx panels (8x8 each) connected via SPI (DIN 12, CLK 11, CS 10)

    OLED Display: 128x64 SSD1306 via I2C

Controls:
    
    PIN 3: Right
    
    PIN 5: Left
    
    PIN 4: Rotate
    
    PIN 6: Fast Drop
    
    PIN 7: End Game

Global Variables

    gameBoard[16]: 16x8 grid representing game state
    
    currentPieceType: Active tetromino type (0-6)
    
    positionX/Y: Current piece position
    
    rotation: Current rotation state (0-3)
    
    points/highScore: Game scoring system

Key Functions

    1. Core Game Functions
    fixingFigureOnBoard(): Merges landed piece with game board
    
        checkCollision(): Detects piece collisions with board boundaries/other pieces
        
        clearAndShiftRows(): Clears completed rows and shifts remaining blocks down with animation
        
        correctionPositionPiece(): Adjusts piece position after rotation (wall kick logic)
        
    2. Movement & Rotation
        handleRotation(): Handles piece rotation with collision checking
        
        handleMovement(): Processes left/right movement inputs
        
        definingNewPiece(): Initializes new tetromino shapes from predefined templates
    
    3. Display Functions
        displayGameOnLeds(): Combines board state with falling piece for LED output
        
        updateScoreDisplay(): Shows current score and high score on OLED
        
        gameOver(): Displays end-game screen with animations
    
    4. Game Flow
        gameLoop(): Main game loop handling timing, inputs, and game logic
        
        resetGame(): Resets game state and starts new session
        
        setup(): Initializes hardware and shows startup sequence
        
        loop(): Primary Arduino loop running game logic

Tetromino Definitions

    7 classic pieces stored as 4x4 bit patterns with rotation variants:
    
    I, O, T, L, J, S, Z
    Each piece has 4 rotation states (0-3)

Features

    Progressive difficulty (speed increases with score)
    
    EEPROM high score storage
    
    Input debouncing (200ms cooldown)
    
    Smooth falling animation
    
    Row clear animation effect
