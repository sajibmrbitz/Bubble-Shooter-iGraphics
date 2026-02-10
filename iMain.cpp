#include "iGraphics.h"
#include "iSound.h"
#include <math.h>

#define POWER_BALLB_INDEX 6

// Tracks screen mode:
// 0 = Menu, 1 = new game, 2 = saved game, 3 = high score, 4 = setting, 5 = about us 6 = difficulty 7=end 8=name enter.
int mode = 0;
int selectedLevel = 0;
int currentLevel = 1;
int bgSoundIdx = -1;
int easy=0;
int medium=0;
int hard=0;
int randomnumber=0;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int BUBBLE_RADIUS = 20;
const int BUBBLE_DIAMETER = BUBBLE_RADIUS * 2;
const double BUBBLE_VERTICAL_SPACING = BUBBLE_DIAMETER * 0.8667; // For hexagonal grid
const int BOARD_ROWS = 12+1;//only for gamve over mecha.
const int BOARD_COLS = 20;
const int BOARD_START_X = BUBBLE_RADIUS;
const int BOARD_START_Y = 600-3*BUBBLE_RADIUS; // grid high position theke start hbe
const double LAUNCHER_SPEED = 15.0; //bubble spped 
const int bubblechance=60;
const int sbubblecolor=98; 
const int maxcolor=6;
const int MAX_LEVELS = 5;
const int gameoverline=BUBBLE_RADIUS; // Y = 20
bool showLevelText = false; // Optional
int levelTextTimerID = -1; 
int launchcount=1;
int pop_r;
int pop_c;
int scorecount=0;
char scoretext[20];                                 //red       //dark green   //blue
unsigned char bubbleColors[maxcolor][3] = {{255, 0, 0}, {1, 50, 32}, {0, 0, 255}, {255, 255, 0}, {150, 75, 0},{128,0,128}};
int counter_pop=0;
int newgame=0;
int savedgame=0;
int highscore=0;
int aboutus=0;
int settings=0;
int nclick=0;
int sclick=0;
int hclick=0;
int seclick=0;
int aclick=0;
int soundLevel = 50;
bool music_on_off=true;
bool isPaused = false; 
int nopopcount=0;
bool gameoverpoint=true;
//swapbubbles
bool isSwapping = false;      
int swapAnimationTimer = 0;  
const int SWAP_ANIMATION_DURATION = 15; 
double swapLauncherStartX, swapLauncherStartY;
double swapNextStartX, swapNextStartY;
//thundereffect
int shakeTimer = 0;  
int shakeMagnitude = 5; 
struct Bubble {
    int x, y;
    int colorIndex;
    bool visible;
    double velX,velY;
    bool checked;
    bool checked2;
    bool isPopping;
    double scale;
};


// --- Global Variables ---
Bubble gameBubbles[BOARD_ROWS][BOARD_COLS]; 
Bubble launcherBubble; 
Bubble nextBubble; 

int currentScore = 0;
bool gameOver = false;
bool sbubble=false;
bool savailable=false;
bool sused=false;
double launcherAngle = 90.0; 
struct Player {
    char name[50];
    int score;
};

Player highScores[10];
char currentPlayerName[50] = "";
int nameCharIndex = 0;
struct GameState {
    Bubble gameBubbles[BOARD_ROWS][BOARD_COLS];
    Bubble launcherBubble;
    Bubble nextBubble;

    int currentScore;
    int currentLevel;
    int launchcount;
    double launcherAngle;
    int easy, medium, hard;
    int randomnumber;

    bool sbubble;
    bool sused;

    int saveSignature; 
};
#define MAX_WIN_BALLS 50

typedef struct {
    float x, y;
    float vy;
    int colorIndex;
    bool active;
} WinBall;

WinBall winBalls[MAX_WIN_BALLS];


// --- Function Prototypes ---
void gameInit();
void drawbubbles(int x, int y, int colorIndex, double scale, int offsetX, int offsetY);
void drawgridbubbles(int offsetX, int offsetY);
void drawlauncherball(int offsetX, int offsetY);
void drawCannonHead(double cx, double cy, double angleDeg, double length, double width, int offsetX, int offsetY);
void moveLauncherBubble();
void snapBubbleToGrid();
void checkCollision();
void gameLogicUpdate();
void loadNextBubble();
void popbubble();
void check_near(int a, int b, int color);
void exchange();
void removeextra(int a,int b);
void checkfloat(int r,int c);
void popfloat();
void nopopshiftrow();
void gameendcheck();
void drawpowerballB(int x, int y, int offsetX, int offsetY);
void powerballBmecha();
void drawAimingLine(int offsetX, int offsetY);
void updatePoppingAnimation();
//score relate
void loadHighScores();
void saveHighScores();
void checkAndAddHighScore();
//saving
void saveGame();
bool loadGame() ;
void initWinBalls();

void initWinBalls() {
    for (int i = 0; i < MAX_WIN_BALLS; i++) {
        winBalls[i].x = rand() % 800; // screen width
        winBalls[i].y = 700 + rand() % 300; // start above screen
        winBalls[i].vy = (rand() % 3 + 2); // initial fall speed
        winBalls[i].colorIndex = rand() % randomnumber;
        winBalls[i].active = true;
    }
}


void updatePoppingAnimation() {             ///---animated pop
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (gameBubbles[r][c].isPopping) {
                gameBubbles[r][c].scale -= 0.1; //10% kore invisible hobe

                if (gameBubbles[r][c].scale <= 0) {
                    gameBubbles[r][c].scale = 0;
                    gameBubbles[r][c].visible = false; 
                    gameBubbles[r][c].isPopping = false; 
                    

                }
            }
        }
    }
gameendcheck();
}

void drawAimingLine(int offsetX, int offsetY) {
    if (launcherBubble.velX != 0 || launcherBubble.velY != 0 || isSwapping) {
        return;
    }

    int colorIdx = launcherBubble.colorIndex;
    if (colorIdx >= 0 && colorIdx < maxcolor) {
        iSetColor(bubbleColors[colorIdx][0], bubbleColors[colorIdx][1], bubbleColors[colorIdx][2]);
    } else {
        iSetColor(255, 255, 255); // white for special bubbles
    }

    double ghostBubbleX = launcherBubble.x;
    double ghostBubbleY = launcherBubble.y;

    double angleRad = launcherAngle * M_PI / 180.0;
    double ghostVelX = LAUNCHER_SPEED * cos(angleRad);
    double ghostVelY = LAUNCHER_SPEED * sin(angleRad);


    for (int i = 0; i < 300; ++i) { // random 300 length of aim line
        ghostBubbleX += ghostVelX * 0.5;
        ghostBubbleY += ghostVelY * 0.5;

        // side walls
        if (ghostBubbleX - BUBBLE_RADIUS <= 0 || ghostBubbleX + BUBBLE_RADIUS >= SCREEN_WIDTH) {
            ghostVelX *= -1; 
        }

        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                if (gameBubbles[r][c].visible) {
                    double dx = gameBubbles[r][c].x - ghostBubbleX;
                    double dy = gameBubbles[r][c].y - ghostBubbleY;
                    if (sqrt(dx * dx + dy * dy) < BUBBLE_DIAMETER) {//found a bubble
                        return;
                    }
                }
            }
        }
        
        if (ghostBubbleY + BUBBLE_RADIUS >= SCREEN_HEIGHT-40) {
            return;
        }

        if (i % 5 == 0) {       //i out of five dots
            iFilledCircle(ghostBubbleX + offsetX, ghostBubbleY + offsetY, 3);
        }
    }
}

void saveGame(){
    GameState currentState;

    for (int r=0;r<BOARD_ROWS;r++){
        for (int c=0;c<BOARD_COLS;c++) {
            currentState.gameBubbles[r][c]=gameBubbles[r][c];
        }
    }
    currentState.launcherBubble = launcherBubble;
    currentState.nextBubble = nextBubble;

    currentState.currentScore = scorecount;
    currentState.currentLevel = currentLevel;
    currentState.launchcount = launchcount;
    currentState.launcherAngle = launcherAngle;

    currentState.easy = easy;
    currentState.medium = medium;
    currentState.hard = hard;
    currentState.randomnumber = randomnumber;

    currentState.sbubble = sbubble;
    currentState.sused = sused;

    currentState.saveSignature = 12345; //accessing index

    FILE *fp = fopen("savegame.dat", "wb"); //write binary
    if (fp == NULL) {
        printf("Error: Could not open save file for writing.\n");
        return;
    }

    fwrite(&currentState, sizeof(GameState), 1, fp);
    fclose(fp);

    printf("Game saved successfully!\n");//in terminal
}

bool loadGame() {//true or false
    GameState savedState;

    FILE *fp = fopen("savegame.dat", "rb"); //read binary
    if (fp == NULL) {
        printf("No saved game file found.\n");
        return false;
    }

    fread(&savedState, sizeof(GameState), 1, fp);
    fclose(fp);

    // just checking
    if (savedState.saveSignature != 12345) {
        printf("Invalid save file.\n");
        return false;
    }

    // copy saved struct
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            gameBubbles[r][c] = savedState.gameBubbles[r][c];
        }
    }
    launcherBubble = savedState.launcherBubble;
    nextBubble = savedState.nextBubble;

    scorecount = savedState.currentScore;
    currentLevel = savedState.currentLevel;
    launchcount = savedState.launchcount;
    launcherAngle = savedState.launcherAngle;

    easy = savedState.easy;
    medium = savedState.medium;
    hard = savedState.hard;
    randomnumber = savedState.randomnumber;

    sbubble = savedState.sbubble;
    sused = savedState.sused;

    printf("Game loaded successfully!\n");
    return true;
}


void checkAndAddHighScore() {
    int newScore = scorecount;
    int scoreIndex = -1;

    // Find where the new score fits in the list
    for (int i = 0; i < 10; i++) {
        if (newScore > highScores[i].score) {
            scoreIndex = i;
            break;
        }
    }

    if (scoreIndex != -1) {
        // Shift lower scores down
        for (int i = 8; i >= scoreIndex; i--) {
            highScores[i + 1] = highScores[i];
        }

        // new score add korbe
        strcpy(highScores[scoreIndex].name, currentPlayerName);
        highScores[scoreIndex].score = newScore;

        saveHighScores(); // update file save hbe
    }
}
void saveHighScores() {
    FILE *fp = fopen("highscores.txt", "w");
    if (fp == NULL) {
        
        return;
    }

    for (int i = 0; i < 10; i++) {
        fprintf(fp, "%s %d\n", highScores[i].name, highScores[i].score);
    }
    fclose(fp);
}
void loadHighScores() {
    FILE *fp = fopen("highscores.txt", "r");
    if (fp == NULL) {
        // file e kisu na thakle intitially sob 0
        for (int i = 0; i < 10; i++) {
            strcpy(highScores[i].name, "---");
            highScores[i].score = 0;
        }
        return;
    }

    for (int i = 0; i < 10; i++) {
        fscanf(fp, "%s %d", highScores[i].name, &highScores[i].score);
    }
    fclose(fp);
}


void drawbubbles(int x, int y, int colorIndex, double scale, int offsetX, int offsetY) 
{
    if(colorIndex<0 || colorIndex>=maxcolor){
        iSetColor(0,0,0);
        iFilledCircle(x + offsetX, y + offsetY, BUBBLE_RADIUS * scale);
        return;
    }

    double currentRadius = BUBBLE_RADIUS * scale; 
    
    iSetColor(bubbleColors[colorIndex][0],bubbleColors[colorIndex][1],bubbleColors[colorIndex][2]);
    iFilledCircle(x + offsetX, y + offsetY, currentRadius - (0.5 * scale)); 

    if (scale > 0.5) {
        iSetTransparentColor(255,255,255,0.8); 
        iFilledCircle(x - currentRadius/3 + offsetX, y + currentRadius/3 + offsetY, currentRadius/3);
    }
}

void drawgridbubbles(int offsetX, int offsetY)
{
    for(int r=0; r<BOARD_ROWS; r++){
        for(int c=0; c<BOARD_COLS; c++) {
            if(gameBubbles[r][c].visible){
                drawbubbles(gameBubbles[r][c].x, gameBubbles[r][c].y, gameBubbles[r][c].colorIndex, gameBubbles[r][c].scale, offsetX, offsetY);
            }
        }
    }
}

void powerballBmecha(int centerR, int centerC){
      const int dr[] = {-2,-2,-2,-1,-1,-1, 0, 0, 0, 1, 1, 1, 2, 2, 2};
    const int dc_even[] = {-1, 0, 1,-2,-1, 0,-2,-1, 1,-2,-1, 0,-1, 0, 1};
    const int dc_odd[]  = {-1, 0, 1,-1, 0, 1,-2,-1, 1,-1, 0, 1,-1, 0, 1};

    for (int i = 0; i < 15; i++) {
        int nr = centerR + dr[i];
        int nc = centerC + (centerR % 2 == 0 ? dc_even[i] : dc_odd[i]);

        if (nr >= 0 && nr < BOARD_ROWS && nc >= 0 && nc < BOARD_COLS) {
            if (gameBubbles[nr][nc].visible) {
                gameBubbles[nr][nc].visible = false;
                if(easy)scorecount += 10;
                if(medium)scorecount += 12;
                if(hard)scorecount += 15;
                iPlaySound("assets/sounds/bomb.wav", false, 100);
                iPlaySound("assets/sounds/pop.wav", false, 80);
            }
        }
    }

    // Also destroy the power ball itself
    gameBubbles[centerR][centerC].visible = false;
    
}

void drawpowerballB(int x, int y, int offsetX, int offsetY)
{
    int radius = 20;
    iSetColor(50, 50, 50);
    iFilledCircle(x + offsetX, y + offsetY, radius);
    iSetColor(200, 200, 200);
    iFilledCircle(x - 5 + offsetX, y + 5 + offsetY, 5);
    iSetColor(80, 80, 80);
    iFilledRectangle(x - 3 + offsetX, y + radius - 2 + offsetY, 6, 6);
    iSetColor(255, 200, 0);
    iLine(x + offsetX, y + radius + 4 + offsetY, x + offsetX, y + radius + 10 + offsetY);
    iSetColor(255, 100, 0);
    iFilledCircle(x + offsetX, y + radius + 12 + offsetY, 3);
    iSetColor(255, 0, 0);
    iText(x - 3 + offsetX, y - 5 + offsetY, "!", GLUT_BITMAP_HELVETICA_18);
}

void iDraw() 
{
    iClear();
    if(mode == 0) {
        iShowImage(0, 0, "assets/images/Game_bg.png");  // Menu background

        if(newgame){iSetColor(255,255,255);iRectangle(255,390,300,40);}
        if(savedgame){iSetColor(255,255,255);iRectangle(230,340,340,40);}
        if(highscore){iSetColor(255,255,255);iRectangle(240,280,320,40);}
        if(settings){iSetColor(255,255,255);iRectangle(260,225,300,40);}
        if(aboutus){iSetColor(255,255,255);iRectangle(270,155,270,40);}
    }
    else if (mode == 1) 
    {
    iSetColor(13, 3, 36);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); 

    iSetColor(80, 80, 80);
    iFilledRectangle(0, 560, SCREEN_WIDTH, 40);
    sprintf(scoretext, "YOUR SCORE: %d", scorecount);
    iSetColor(255, 255, 128);
    iText(20, 575, scoretext, GLUT_BITMAP_HELVETICA_18);
    iText(750, 575, "Back", GLUT_BITMAP_HELVETICA_18);




    int offsetX = 0;
    int offsetY = 0;
    if (shakeTimer > 0) {
        offsetX = rand() % (2 * shakeMagnitude + 1) - shakeMagnitude;
        offsetY = rand() % (2 * shakeMagnitude + 1) - shakeMagnitude;
        shakeTimer--;
    }

    drawAimingLine(offsetX, offsetY);
    drawgridbubbles(offsetX, offsetY);
    drawCannonHead(SCREEN_WIDTH/2, 0, launcherAngle, 80, 40, offsetX, offsetY);
    drawlauncherball(offsetX, offsetY);

        // shake canon
    iSetColor(80, 80, 80); iFilledCircle(SCREEN_WIDTH/2 + offsetX, 0 + offsetY, 20);
    iSetColor(80, 80, 80); iFilledCircle(SCREEN_WIDTH/2 + offsetX, 0 + offsetY, 10);
    iSetColor(80, 80, 80); iFilledCircle(SCREEN_WIDTH/2 + offsetX, 0 + offsetY, 5);


    if(showLevelText) {
        char levelStr[20];
        sprintf(levelStr, "LEVEL %d", currentLevel);
        iSetColor(255, 255, 0);
        iText(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2, levelStr, GLUT_BITMAP_TIMES_ROMAN_24);
    }
    
    if (isPaused) {
        iSetTransparentColor(0, 0, 0, 0.7);
        iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        iSetColor(255, 255, 0);
        iText(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 + 20, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

        int buttonX = SCREEN_WIDTH / 2 - 100;
        int buttonY = SCREEN_HEIGHT / 2 - 30;
        int buttonW = 200;
        int buttonH = 40;
        iSetColor(200, 200, 255);
        iRectangle(buttonX, buttonY-200, buttonW, buttonH);
        iSetColor(255, 55, 25);
        iText(buttonX + 40, buttonY + 12-200, "SAVE AND EXIT", GLUT_BITMAP_HELVETICA_18);
        iSetColor(255, 255, 255); 
        iText(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 20, "Press 'r' to resume", GLUT_BITMAP_HELVETICA_18);
    }
    }   
    else if(mode == 2) {
        iSetColor(255, 0, 0);
        iText(30, 200, "Saved Game", GLUT_BITMAP_HELVETICA_18);
    }
    else if (mode == 3) {
    iClear();

    // Draw dark background panel
    int panelX = SCREEN_WIDTH / 2 - 220;
    int panelY = SCREEN_HEIGHT / 2 + 200;
    int panelWidth = 440;
    int panelHeight = 400;

    iSetColor(20, 20, 40); // Background panel color
    iFilledRectangle(panelX, panelY - panelHeight, panelWidth, panelHeight);

    iSetColor(255, 255, 255); 
    iRectangle(panelX, panelY - panelHeight, panelWidth, panelHeight);

    
    const char *title = " HIGH SCORES ";
    iSetColor(255, 215, 0); // Gold
    iText(SCREEN_WIDTH / 2 - 80, panelY - 40, title, GLUT_BITMAP_TIMES_ROMAN_24);

    
    iSetColor(200, 200, 255);
    iText(panelX + 50, panelY - 70, "Name", GLUT_BITMAP_HELVETICA_18);
    iText(panelX + panelWidth - 100, panelY - 70, "Score", GLUT_BITMAP_HELVETICA_18);

    
    for (int i = 0; i < 10; i++) {
        char entry[100];
        sprintf(entry, "%2d. %-15s %5d", i + 1, highScores[i].name, highScores[i].score);

        int y = panelY - 100 - i * 30;

        // Top 3 get special colors
        if (i == 0) iSetColor(255, 215, 0);     // Gold
        else if (i == 1) iSetColor(192, 192, 192); // Silver
        else if (i == 2) iSetColor(205, 127, 50);  // Bronze
        else iSetColor(230, 130, 255);         // Others

        // Display entry (manually aligned text columns)
        char rankStr[5], nameStr[20], scoreStr[10];
        sprintf(rankStr, "%2d.", i + 1);
        sprintf(nameStr, "%-15s", highScores[i].name);
        sprintf(scoreStr, "%5d", highScores[i].score);

        iText(panelX + 10, y, rankStr, GLUT_BITMAP_HELVETICA_18);
        iText(panelX + 50, y, nameStr, GLUT_BITMAP_HELVETICA_18);
        iText(panelX + panelWidth - 100, y, scoreStr, GLUT_BITMAP_HELVETICA_18);
    }
    iSetColor(255, 255, 255);
    iText(30, 20, "Press 'b' to go back", GLUT_BITMAP_HELVETICA_12);
}



    else if(mode == 4) {
        if(music_on_off){
        iShowImage(0, 0, "assets/images/bg_setting_on.png");}
        else
        iShowImage(0, 0, "assets/images/bg_setting_off.png");
        int barX = 420, barY = 240;
    int barWidth = 150, barHeight = 15;

    iSetColor(200, 200, 200); // Bar outline
    iRectangle(barX, barY, barWidth, barHeight);

    if (soundLevel > 100) soundLevel = 100;
    if (soundLevel < 0) soundLevel = 0;

    int fillWidth = (soundLevel * barWidth) / 100;

int r = (soundLevel * 255) / 100;             
int g = 255 - abs(soundLevel - 50) * 5;           
int b = 0;

if (g < 0) g = 0;    // Clamp
if (g > 255) g = 255;

iSetColor(r, g, b);
iFilledRectangle(barX, barY, fillWidth, barHeight);



        iSetColor(100, 100, 100);
        iText(10, 10, "Press 'b' to go back", GLUT_BITMAP_HELVETICA_12);
    }
    else if(mode == 5) {
        iShowImage(0, 0, "assets/images/about_us.png");
        iText(30, 20, "Press 'b' to go back", GLUT_BITMAP_HELVETICA_12);
        
       
    }
    else if(mode==6){
        iShowImage(0,0,"assets/images/difficulty.png");
    }
     else if (mode == 7) {
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    iSetColor(255, 255, 255);

    if (launchcount >= bubblechance) {
        iShowImage(150,300,"assets/images/game_over.png");
    } else {
        iShowImage(0,0,"assets/images/BUETC.png");
        for (int i = 0; i < MAX_WIN_BALLS; i++) {
        if (winBalls[i].active) {
            // Draw ball
            iSetColor(bubbleColors[winBalls[i].colorIndex][0],
                      bubbleColors[winBalls[i].colorIndex][1],
                      bubbleColors[winBalls[i].colorIndex][2]);
            iFilledCircle(winBalls[i].x, winBalls[i].y, BUBBLE_RADIUS);

            // Animate
            winBalls[i].y += winBalls[i].vy;
            winBalls[i].vy -= 0.055; // gravity

            // Bounce if hit ground
            if (winBalls[i].y <= BUBBLE_RADIUS + 40) {
                winBalls[i].vy *= -0.6; // bounce back
                if (fabs(winBalls[i].vy) < 1.0) {
                    winBalls[i].active = false; // settle
                }
            }
        }
    }
        
        iSetColor(255,255,255);
        iText(300, 500, "YOU WIN !!!!!!!!!!!", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Show the score
    char scoreText[50];
    sprintf(scoreText, "YOUR SCORE: %d", scorecount);
     iSetColor(255,255,255);
    iText(300, 300, scoreText, GLUT_BITMAP_HELVETICA_18);

    iText(30, 20, "Press 'b' to go back to Main Menu", GLUT_BITMAP_HELVETICA_18);
}
 else if (mode == 8) { // Enter Name Screen
        iSetColor(13, 3, 36);
        iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        iSetColor(255, 255, 255);
        iText(250, 400, "ENTER YOUR NAME:", GLUT_BITMAP_TIMES_ROMAN_24);

        
        iSetColor(200, 200, 200);
        iRectangle(250, 340, 300, 40);
        iText(260, 350, currentPlayerName, GLUT_BITMAP_HELVETICA_18);

        iText(260, 250, "Press ENTER to continue", GLUT_BITMAP_HELVETICA_18);
    }
    else if(mode == 9) {
        iShowImage(0,0,"assets/images/instruction.png");
    }

}



void gameendcheck(){
bool anyBubbleLeft = false;
for (int r = 0; r < BOARD_ROWS; r++) {
    for (int c = 0; c < BOARD_COLS; c++) {
        if (gameBubbles[r][c].visible) {
            anyBubbleLeft = true;
            break;
        }
    }
    if (anyBubbleLeft) break;
}

    if (!anyBubbleLeft){
        if (currentLevel < MAX_LEVELS) {
            currentLevel++; 
            launchcount = 1;
            sused=false;
            sbubble=false;
            gameInit(); 
            iPlaySound("assets/sounds/win.wav", false, 50);
        } 
        else {
            checkAndAddHighScore();
            mode = 7; // YOU WIN

            initWinBalls();
            gameOver = true;
remove("savegame.dat");
            iPlaySound("assets/sounds/win.wav", false, 50);
            iPlaySound("assets/sounds/yeah_boy.wav", false, 1000);
        }
        return; 
    }
if (launchcount >= bubblechance) {
    checkAndAddHighScore();
    mode = 7;
    gameOver = true;
   
remove("savegame.dat");
    iPlaySound("assets/sounds/gameover.wav", false, 100); 
    return;
}

}
void gameovercheats(){
   if(gameoverpoint){ checkAndAddHighScore();mode = 7;
        launchcount=bubblechance;
        iPlaySound("assets/sounds/gameover.wav", false, 100); 
       gameoverpoint=false;
   }
}

void nopopshiftrow() {

    for (int r = BOARD_ROWS - 1; r >= 2; r--) {
        for (int c = 0; c < BOARD_COLS; c++) {
            gameBubbles[r][c].visible = gameBubbles[r - 2][c].visible;
            gameBubbles[r][c].colorIndex = gameBubbles[r - 2][c].colorIndex;//copy visibility and color,not entire struct including ispopping etc
        }
    }


    for (int r = 0; r < 2; r++) {
        int offsetX = (r % 2 == 1) ? BUBBLE_RADIUS : 0;
        for (int c = 0; c < BOARD_COLS; c++) {
            if (BOARD_START_X + offsetX + c * BUBBLE_DIAMETER + BUBBLE_RADIUS <= SCREEN_WIDTH) {
                gameBubbles[r][c].visible = true;
                gameBubbles[r][c].colorIndex = rand() % randomnumber;
            } else {
                gameBubbles[r][c].visible = false;
                gameBubbles[r][c].colorIndex = -1; 
            }
        }
    }


    for (int r = 0; r < BOARD_ROWS; r++) {                 
        int offsetX = (r % 2 == 1) ? BUBBLE_RADIUS : 0;
        for (int c = 0; c < BOARD_COLS; c++) {
            gameBubbles[r][c].x = BOARD_START_X + offsetX + c * BUBBLE_DIAMETER;
            gameBubbles[r][c].y = BOARD_START_Y - r * BUBBLE_VERTICAL_SPACING;

            gameBubbles[r][c].isPopping = false; 
            gameBubbles[r][c].scale = 1.0;      
            gameBubbles[r][c].checked = false;
            gameBubbles[r][c].checked2 = false;
            gameBubbles[r][c].velX = 0;
            gameBubbles[r][c].velY = 0;
        }
    }

    
   bool isGameOver = false;
    for (int c = 0; c < BOARD_COLS; c++) {
        if (gameBubbles[BOARD_ROWS - 2][c].visible) {
            isGameOver = true;
          
remove("savegame.dat");
            break;
        }
    }

    if (isGameOver) {
        iSetTimer(100, gameovercheats);
    }
}

void exchange(int *a, int *b){
int t;
t=*a;
*a=*b;
*b=t;
}

void checkfloat(int r, int c)
{
    if (r < 0 || r >= BOARD_ROWS || c < 0 || c >= BOARD_COLS) return;
    if (!gameBubbles[r][c].visible || gameBubbles[r][c].checked2 || gameBubbles[r][c].isPopping) return;

    gameBubbles[r][c].checked2 = true;

    if (r % 2 == 0){
        checkfloat(r-1,c-1);
        checkfloat(r-1,c);
        checkfloat(r,c-1);
        checkfloat(r,c+1);
        checkfloat(r+1,c-1);
        checkfloat(r+1,c);
    }
    else
    {
        checkfloat(r-1,c);
        checkfloat(r-1,c+1);
        checkfloat(r,c-1);
        checkfloat(r,c+1);
        checkfloat(r+1,c);
        checkfloat(r+1,c+1);
    }
}

void popfloat()
{

    for (int r=0;r<BOARD_ROWS;r++)
    {
        for (int c=0;c<BOARD_COLS;c++)
        {
            gameBubbles[r][c].checked2=false;
        }
    }
    for (int c=0;c<BOARD_COLS;c++)
    {
        if (gameBubbles[0][c].visible)
        {
            checkfloat(0,c);
        }
    }
    for (int r=0;r<BOARD_ROWS;r++)
    {
        for (int c=0;c<BOARD_COLS;c++)
        {
            if (gameBubbles[r][c].visible && !gameBubbles[r][c].checked2)
            {
                if (!gameBubbles[r][c].isPopping)
                {
                gameBubbles[r][c].visible=false;
                gameBubbles[r][c].velX = 0;
                gameBubbles[r][c].velY = 0;
                if(easy)scorecount+=10;
                if(medium)scorecount+=12;
                if(hard)scorecount+=15; 
                }
            }

        }
    }
}
void removeextra(int a,int b){
if(a==0)return;
if (a%2==0){
if(gameBubbles[a-1][b-1].visible==false && gameBubbles[a-1][b].visible==false
&&gameBubbles[a][b-1].visible==false && gameBubbles[a][b+1].visible==false&&
gameBubbles[a+1][b].visible==false && gameBubbles[a+1][b].visible==false){gameBubbles[a][b].visible=false;scorecount++ ;}}
if(a%2!=0){
if(gameBubbles[a-1][b].visible==false && gameBubbles[a-1][b+1].visible==false
&&gameBubbles[a][b-1].visible==false && gameBubbles[a][b+1].visible==false&&
gameBubbles[a+1][b+1].visible==false && gameBubbles[a+1][b].visible==false){gameBubbles[a][b].visible=false;scorecount++;}}

}


void check_near(int a, int b, int color){
    if(a<0 || a>=BOARD_ROWS || b<0 || b>=BOARD_COLS) return;
    if (!gameBubbles[a][b].visible || gameBubbles[a][b].checked || gameBubbles[a][b].colorIndex!=color) return;

    gameBubbles[a][b].checked=true;
    counter_pop++;

    if(a%2!=0) {
       check_near(a-1,b,color);
       check_near(a-1,b+1,color);
       check_near(a,b-1,color);
       check_near(a,b+1,color);
       check_near(a+1,b+1,color);
       check_near(a+1,b,color);
    } else {
       check_near(a-1,b-1,color);
       check_near(a-1,b,color);
       check_near(a,b-1,color);
       check_near(a,b+1,color);
       check_near(a+1,b-1,color);
       check_near(a+1,b,color);
    }
}

void popbubble(int a, int b){
    if(a<0 || a>= BOARD_ROWS || b<0 || b>=BOARD_COLS) return;
    if(!gameBubbles[a][b].visible) return;

    // Step 1: Reset all checked to false before new pop attempt
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            gameBubbles[i][j].checked =false;
        }
    }

    counter_pop=0; // Reset global count
    int targetC=gameBubbles[a][b].colorIndex;
    //  Mark all connected same-color bubbles
   check_near(a,b,targetC);
    // If 3 or more connected, pop them
    if(counter_pop>= 3){
        for(int i=0;i<BOARD_ROWS;i++){
            for(int j=0;j<BOARD_COLS;j++){
                if(gameBubbles[i][j].checked){
                   if (!gameBubbles[i][j].isPopping && gameBubbles[i][j].visible) {
                   gameBubbles[i][j].isPopping = true; //animation start
                   iPlaySound("assets/sounds/pop.wav", false, 100);
                   }
                }
            }
        }

        if(easy)scorecount+=counter_pop*10;
        if(medium)scorecount+=counter_pop*12;
        if(hard)scorecount+=counter_pop*15;
    }
    
}

void gameInit() {
    int initialRows = 2 + currentLevel; 
    if (initialRows > BOARD_ROWS) initialRows = BOARD_ROWS; // not to exceed max

    for (int r = 0; r < BOARD_ROWS; r++) {
        int offsetX = (r % 2 == 1) ? BUBBLE_RADIUS : 0; // Odd rows are shifted
        for (int c = 0; c < BOARD_COLS; ++c) {
            gameBubbles[r][c].x = BOARD_START_X + offsetX + c * BUBBLE_DIAMETER;
            gameBubbles[r][c].y = BOARD_START_Y - r * BUBBLE_VERTICAL_SPACING;
            
            if (r < initialRows) {
                gameBubbles[r][c].colorIndex = rand() % randomnumber;
                gameBubbles[r][c].visible = true; 
                gameBubbles[r][c].checked = false;
                gameBubbles[r][c].isPopping = false;
                //gameBubbles[r][c].isFalling = false;
                gameBubbles[r][c].scale = 1.0;
            } else {
                gameBubbles[r][c].colorIndex = -1;
                gameBubbles[r][c].visible = false; 
            }
            
            if(gameBubbles[r][c].x + BUBBLE_RADIUS > SCREEN_WIDTH) gameBubbles[r][c].visible = false;

            gameBubbles[r][c].velX = 0;
            gameBubbles[r][c].velY = 0;
        }
    }
    
    launcherBubble.colorIndex = rand() % randomnumber;
    launcherBubble.visible = true;
    launcherBubble.velX = 0;
    launcherBubble.velY = 0;

    nextBubble.colorIndex = rand() % randomnumber;
    nextBubble.visible = true;
    
    showLevelText = true;
    if(levelTextTimerID != -1) iPauseTimer(levelTextTimerID);
    
    levelTextTimerID = iSetTimer(1500, [](){ showLevelText = false; }); 
    
    gameOver = false;
    sbubble=false;
}

void loadNextBubble() {
    if(launchcount<bubblechance){ 
    launcherBubble.colorIndex = nextBubble.colorIndex;
    launcherBubble.visible = true;
    launcherBubble.velX = 0;
    launcherBubble.velY = 0;

    int chance = rand() % 100; // 15% chance
    if (chance < 15) {
    nextBubble.colorIndex = POWER_BALLB_INDEX; 
    } 

    else {
    nextBubble.colorIndex = rand()%randomnumber; // only uses 0 to randomnumber-1
   }
    launchcount++;
}
    else if(launchcount>=bubblechance){
        nextBubble.visible = false;
    }
}


void snapBubbleToGrid() {
    float minDistance = 999;
    int bestRow = -1, bestCol = -1;
    const float searchRadius = BUBBLE_DIAMETER * 2.5;///search within 100 radius

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            if (!gameBubbles[r][c].visible) {
                float dx = gameBubbles[r][c].x - launcherBubble.x;
                float dy = gameBubbles[r][c].y - launcherBubble.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < searchRadius && distance < minDistance) {
                    minDistance = distance;
                    bestRow = r;
                    bestCol = c;
                }
            }
        }
    }

    if (bestRow != -1) {
        if (bestRow == BOARD_ROWS - 1) {
            gameOver = true;
remove("savegame.dat");
            launchcount = bubblechance;
            mode = 7;
            iPlaySound("assets/sounds/gameover.wav", false, 100);
            return;
        }
        gameBubbles[bestRow][bestCol].visible = true;
        gameBubbles[bestRow][bestCol].scale = 1.0;
        gameBubbles[bestRow][bestCol].colorIndex = launcherBubble.colorIndex;
        pop_r = bestRow;
        pop_c = bestCol;




        if (gameBubbles[bestRow][bestCol].colorIndex == sbubblecolor) {
            gameBubbles[bestRow][bestCol].visible = false;
            scorecount += 20;
            iPlaySound("assets/sounds/pop.wav", false, 100);
            shakeTimer = 20;            //shaking timer
            for (int c = 0; c < BOARD_COLS; c++) {
                if (gameBubbles[bestRow][c].visible) {
                    gameBubbles[bestRow][c].visible = false;
                    if (easy) scorecount += 10;
                    else if (medium) scorecount += 12;
                    else if (hard) scorecount += 15;
                }
            }
            for (int r = 0; r < BOARD_ROWS; r++) {
                if (gameBubbles[r][bestCol].visible) {
                    gameBubbles[r][bestCol].visible = false;
                    if (easy) scorecount += 10;
                    else if (medium) scorecount += 12;
                    else if (hard) scorecount += 15;
                }
            }
        }
        else if (launcherBubble.colorIndex == POWER_BALLB_INDEX) {
            powerballBmecha(bestRow, bestCol);
        }
        else {
            popbubble(bestRow, bestCol);
        }
        popfloat();
        launcherBubble.visible = false;
        launcherBubble.velX = 0;
        launcherBubble.velY = 0;
        loadNextBubble();
        }
        else
        {
        gameOver = true;
        
remove("savegame.dat");
        launcherBubble.visible = false;
    }
}

void checkCollision() {
    
    if (launcherBubble.velX == 0 && launcherBubble.velY == 0) return;

    // collision jodi top e hy
    if (launcherBubble.y + BUBBLE_RADIUS >= SCREEN_HEIGHT) {
        launcherBubble.velX = 0;
        launcherBubble.velY = 0;
        snapBubbleToGrid();
        if (gameBubbles[pop_r][pop_c].colorIndex == POWER_BALLB_INDEX) {
    powerballBmecha(pop_r, pop_c); // call custom pop
} else {
    popbubble(pop_r, pop_c); // normal match logic
}
popfloat();
        //if(easy){if((launchcount+1)%15==0){nopopshiftrow();iPlaySound("assets/sounds/shiftrow.wav",false,100);}}
    if(medium){if((launchcount+1)%15==0){nopopshiftrow();iPlaySound("assets/sounds/shiftrow.wav",false,100);}}
    if(hard){if((launchcount+1)%10==0){nopopshiftrow();iPlaySound("assets/sounds/shiftrow.wav",false,100);}}
    gameendcheck();
        return;
    }

    // Check collision with grid bubbles
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            if (gameBubbles[r][c].visible) {
                float dx = gameBubbles[r][c].x - launcherBubble.x;
                float dy = gameBubbles[r][c].y - launcherBubble.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance <= BUBBLE_DIAMETER) {
                    launcherBubble.velX = 0;
                    launcherBubble.velY = 0;
                    snapBubbleToGrid();
                    if (gameBubbles[pop_r][pop_c].colorIndex == POWER_BALLB_INDEX) {
    powerballBmecha(pop_r, pop_c); // call custom pop
} 
else {
    popbubble(pop_r, pop_c); // normal match logic
}
popfloat();
                    ;
                  // if(easy){if((launchcount+1)%15==0){nopopshiftrow();iPlaySound("assets/sounds/shiftrow.wav",false,100);}}
    if(medium){if((launchcount+1)%15==0){nopopshiftrow();iPlaySound("assets/sounds/shiftrow.wav",false,100);}}
    if(hard){if((launchcount+1)%10==0){nopopshiftrow();iPlaySound("assets/sounds/shiftrow.wav",false,100);}}
                    gameendcheck();
                    return; // Exit after first collision
                }
            }
        }
    }
}


void moveLauncherBubble() {
    if (!launcherBubble.visible || (launcherBubble.velX == 0 && launcherBubble.velY == 0)) return;

    launcherBubble.x += launcherBubble.velX;
    launcherBubble.y += launcherBubble.velY;

    if (launcherBubble.x-BUBBLE_RADIUS<=0 || launcherBubble.x + BUBBLE_RADIUS >= SCREEN_WIDTH) {
        launcherBubble.velX *= -1;
    }
    
}


void drawlauncherball(int offsetX, int offsetY)
{
    if (launcherBubble.velX == 0 && launcherBubble.velY == 0 && launcherBubble.visible && !isSwapping) {
        double angleRad = launcherAngle * M_PI / 180.0;
        launcherBubble.x = SCREEN_WIDTH/2 + 80 * cos(angleRad);
        launcherBubble.y = 0 + 80 * sin(angleRad);
    }
    
    if (launcherBubble.visible) {
        int x = launcherBubble.x;
        int y = launcherBubble.y;
        int color = launcherBubble.colorIndex;

        if(color == POWER_BALLB_INDEX){
            drawpowerballB(x, y, offsetX, offsetY);
        }
        else if (color == sbubblecolor) {
        double segmentAngle = 360.0 / maxcolor; 
        for(int i = 0; i < maxcolor; ++i) { 
        iSetColor(bubbleColors[i][0], bubbleColors[i][1], bubbleColors[i][2]);
        double angle = i * segmentAngle * M_PI / 180.0;
        double offset_radius = BUBBLE_RADIUS / 2.0;
        iFilledCircle(x + offset_radius * cos(angle) + offsetX, y + offset_radius * sin(angle) + offsetY, BUBBLE_RADIUS / 2.5);
        }
        iSetTransparentColor(255, 255, 255, 0.7);
        iFilledCircle(x + offsetX, y + offsetY, BUBBLE_RADIUS / 3.0);
        } 
        else {
            drawbubbles(x, y, color, 1.0, offsetX, offsetY);
        }
    }
}

void drawCannonHead(double cx, double cy, double angleDeg, double length, double width, int offsetX, int offsetY)
{
    // Shaking center
    double shake_cx = cx + offsetX;
    double shake_cy = cy + offsetY;

    double angleRad = angleDeg * M_PI / 180.0;
    double c = cos(angleRad);
    double s = sin(angleRad);

    // === Main Cannon Barrel ===
    double barrelLength = length * 0.77;
    double barrelWidth = width * 0.6;
    double barrel_dx = (barrelWidth / 2.0) * sin(angleRad);
    double barrel_dy = (barrelWidth / 2.0) * cos(angleRad);
    double tipX = shake_cx + barrelLength * c;
    double tipY = shake_cy + barrelLength * s;
    double barrelX[] = { shake_cx - barrel_dx, shake_cx + barrel_dx, tipX + barrel_dx, tipX - barrel_dx };
    double barrelY[] = { shake_cy + barrel_dy, shake_cy - barrel_dy, tipY - barrel_dy, tipY + barrel_dy };
    iSetColor(40, 40, 90); // Sleek base barrel color
    iFilledPolygon(barrelX, barrelY, 4);

    
    double glowWidth = barrelWidth * 0.4;
    double glow_dx = (glowWidth / 2.0) * sin(angleRad);
    double glow_dy = (glowWidth / 2.0) * cos(angleRad);
    double glowX[] = { shake_cx - glow_dx, shake_cx + glow_dx, tipX + glow_dx, tipX - glow_dx };
    double glowY[] = { shake_cy + glow_dy, shake_cy - glow_dy, tipY - glow_dy, tipY + glow_dy };
    iSetColor(0, 255, 255); 
    iFilledPolygon(glowX, glowY, 4);

    
    double baseRadius = width * 1.2;
    iSetColor(30, 30, 50); // Metallic gray
    iFilledCircle(shake_cx, shake_cy, baseRadius);

    int ringCount = 3;
    for (int i = 0; i < ringCount; i++) {
        double ringRadius = width * (1.0 + i * 0.15);
        int shade = 30 + i * 10;
        iSetColor(shade, shade, 50 + i * 10); 
        iCircle(shake_cx, shake_cy, ringRadius);
    }

    for (int i = 0; i < 360; i += 45) {
        double rad = i * M_PI / 180.0;
        double innerR = width * 0.8;
        double outerR = width * 1.3;
        double x1 = shake_cx + cos(rad) * innerR;
        double y1 = shake_cy + sin(rad) * innerR;
        double x2 = shake_cx + cos(rad) * outerR;
        double y2 = shake_cy + sin(rad) * outerR;
        iSetColor(80, 80, 130);
        iLine(x1, y1, x2, y2);
    }

    
    iSetColor(0, 255, 255);
    iFilledCircle(shake_cx, shake_cy, width * 0.4);

   
    double supportHeight = width * 0.55;
    double supportWidth = width * 0.6;
    double supportTopY = shake_cy;
    double supportBottomY = shake_cy - supportHeight;
    double supportLeftX = shake_cx - supportWidth / 2.0;
    double supportRightX = shake_cx + supportWidth / 2.0;
    double supportX[] = { supportLeftX, supportRightX, supportRightX, supportLeftX };
    double supportY[] = { supportTopY, supportTopY, supportBottomY, supportBottomY };
    iSetColor(60, 0, 100); 
    iFilledPolygon(supportX, supportY, 4);

   
    iSetColor(15, 15, 15);
    iFilledEllipse(shake_cx, shake_cy - baseRadius * 0.8, width * 2.2, width * 0.5);

    
    for (int i = 0; i < 3; i++) {
        double pulseRadius = width * 0.75 - i * 5;
        iSetColor(255 - i * 30, 255 - i * 30, 255);
        iCircle(shake_cx, shake_cy, pulseRadius);
    }

    
    iSetColor(255, 255, 255); 
    iCircle(shake_cx, shake_cy, width * 0.75);

    
    for (int i = 0; i < 360; i += 60) {
        double rad = i * M_PI / 180.0;
        double boltX = shake_cx + cos(rad) * width * 1.1;
        double boltY = shake_cy + sin(rad) * width * 1.1;
        iSetColor(0, 200, 255);
        iFilledCircle(boltX, boltY, 3);
        iSetColor(255, 255, 255);
        iCircle(boltX, boltY, 3.5);
    }

    if (nextBubble.visible) {
        int x, y;
        int color = nextBubble.colorIndex;

        if (isSwapping) {
            x = nextBubble.x;
            y = nextBubble.y;
        } 
        else {
            x = cx + 150; 
            y = 20;      
            nextBubble.x = x; 
            nextBubble.y = y;
        }

        if (color == POWER_BALLB_INDEX) {
            drawpowerballB(x, y, 0, 0); 
        } 
        else if (color == sbubblecolor) {
            double segmentAngle = 360.0 / maxcolor;
            for(int i = 0; i < maxcolor; ++i) {
                iSetColor(bubbleColors[i][0], bubbleColors[i][1], bubbleColors[i][2]);
                double angle = i * segmentAngle * M_PI / 180.0;
                double offset_radius = BUBBLE_RADIUS / 2.0;
                iFilledCircle(x + offset_radius * cos(angle), y + offset_radius * sin(angle), BUBBLE_RADIUS / 2.5);
            }
            iSetTransparentColor(255, 255, 255, 0.7);
            iFilledCircle(x, y, BUBBLE_RADIUS / 3.0);
        }
        else {
            drawbubbles(x, y, color, 1.0, 0, 0);
        }
    }
}

void gameLogicUpdate(){
    if (isPaused) return; 
    else if (mode == 1 && !gameOver) {
        moveLauncherBubble();
        updatePoppingAnimation();
        checkCollision();
        
    }
    if (isSwapping) 
    {
        if (swapAnimationTimer>0) {
            swapAnimationTimer--;
            double progress = 1.0 - ((double)swapAnimationTimer / SWAP_ANIMATION_DURATION);

            launcherBubble.x = swapLauncherStartX + (swapNextStartX - swapLauncherStartX) * progress;   //move both
            launcherBubble.y = swapLauncherStartY + (swapNextStartY - swapLauncherStartY) * progress;

            nextBubble.x = swapNextStartX + (swapLauncherStartX - swapNextStartX) * progress;
            nextBubble.y = swapNextStartY + (swapLauncherStartY - swapNextStartY) * progress;

        }
        else
        {
            isSwapping = false;
            exchange(&launcherBubble.colorIndex, &nextBubble.colorIndex);//simple exchange
            double angleRad = launcherAngle * M_PI / 180.0;
            launcherBubble.x = SCREEN_WIDTH/2 + 80 * cos(angleRad);
            launcherBubble.y = 0 + 80 * sin(angleRad);
            nextBubble.x = SCREEN_WIDTH/2 + 150;
            nextBubble.y = 20;
        }
    }
}

/*
    iMouse() is triggered on mouse button press/release.
*/
void iMouse(int button, int state, int mx, int my){
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (mode == 0) {
            if (mx >= 255 && mx <= 555 && my >= 390 && my <= 430) {
                mode = 8;
                strcpy(currentPlayerName, "");
                nameCharIndex = 0;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            } else if (mx >= 230 && mx <= 570 && my >= 340 && my <= 380) {
                if (loadGame()) {
                    mode = 1;
                    isPaused = false;
                }
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            } else if (mx >= 240 && mx <= 560 && my >= 280 && my <= 320) {
                mode = 3;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            } else if (mx >= 290 && mx <= 520 && my >= 225 && my <= 265) {
                mode = 4;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            } else if (mx >= 270 && mx <= 540 && my >= 155 && my <= 195) {
                mode = 5;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            }
            else if (mx >= 694 && mx <= 777 && my >= 26 && my <= 100) {
                mode = 9;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            }
            else if (mx >= 10 && mx <= 100 && my >= 0 && my <= 75) {
                
                
                iPlaySound("assets/sounds/button_click.wav", false, 100);
                exit(0);
            }
        } else if (mode == 4) {
            if (mx >= 530 && mx <= 590 && my >= 365 && my <= 400) {
                music_on_off = !music_on_off;
                if (music_on_off)
                    iResumeSound(bgSoundIdx);
                else
                    iPauseSound(bgSoundIdx);
            } else if (mx >= 395 && mx <= 430 && my >= 235 && my <= 255) {
                
                soundLevel -= 10;
                if (soundLevel < 0) soundLevel = 0;

                iDecreaseVolume(bgSoundIdx, 10);
            } else if (mx >= 550 && mx <= 595 && my >= 235 && my <= 255) {
               if(soundLevel<=90)soundLevel+=10;
                else soundLevel=100;
                iIncreaseVolume(bgSoundIdx, 10);
            }
            else if (mx >= 300 && mx <= 500 && my >= 130 && my <= 180){mode=0;}
        } else if (mode == 1 && !gameOver){
            if (isPaused) {
                int buttonX = SCREEN_WIDTH / 2 - 100;
                int buttonY = SCREEN_HEIGHT / 2 - 30;
                int buttonW = 200;
                int buttonH = 40;
                if (mx >= buttonX && mx <= buttonX + buttonW && my >= buttonY-200 && my <= buttonY-200 + buttonH) {
                    saveGame();
                    mode = 0;
                    isPaused = false;
                }
                return;
            }
            if (mx >=750  && mx <= 800 && my >= 565 && my <= 600){isPaused=true;}

            else if (launcherBubble.visible && launcherBubble.velX == 0 && launcherBubble.velY == 0){
                double angleRad = launcherAngle * M_PI / 180.0;
                launcherBubble.velX = LAUNCHER_SPEED * cos(angleRad);
                launcherBubble.velY = LAUNCHER_SPEED * sin(angleRad);
            }
        } else if (mode == 6) {
            bool selected = false;
            if (mx >= 300 && mx <= 500 && my >= 392 && my <= 460) {
                medium = 0;
                hard = 0;
                selected = true;
                randomnumber = 4;
                easy = 1;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            } else if (mx >= 240 && mx <= 560 && my >= 300 && my <= 365) {
                easy = 0;
                hard = 0;
                selected = true;
                randomnumber = 5;
                medium = 1;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            } else if (mx >= 290 && mx <= 505 && my >= 220 && my <= 280) {
                easy = 0;
                medium = 0;
                selected = true;
                randomnumber = 6;
                hard = 1;
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            }

            if (selected) {
                mode = 1;
                scorecount = 0;
                currentLevel = 1;
                launchcount = 1;
                gameInit();
                iPlaySound("assets/sounds/button_click.wav", false, 100);
            }
        }
    }
}

/*
    iKeyboard() is called when a key is pressed.
*/
void iKeyboard(unsigned char key) 
{   
    if (mode == 1){
    if (key == 'p' || key == 'P') {
        isPaused = true;
    } 
    else if (key == 'r' || key == 'R') {
        isPaused = false;
    }
    }
    if (mode == 8) { // Name Input Screen
        if (key == '\r') { // Enter key
            if (nameCharIndex > 0) {
                mode = 6; // Go to difficulty selection
            }
        } else if (key == '\b') { // Backspace key
            if (nameCharIndex > 0) {
                nameCharIndex--;
                currentPlayerName[nameCharIndex] = '\0';
            }
        } else {
            if (nameCharIndex < 49) { // Prevent buffer overflow
                currentPlayerName[nameCharIndex] = key;
                nameCharIndex++;
                currentPlayerName[nameCharIndex] = '\0';
            }
        }
    } 
    if(key == 'b') {if(mode!=8){
        mode = 0;
        launchcount=1;
        scorecount=0;
        sbubble=false;
            ///savailable=true;
            sused=false; 
    }}
    else if(key == 'n' && mode == 1) {
        gameInit();
        scorecount=0;
    }
    else if ((key == 'e' || key == 'E') && mode == 1) {
    if (launcherBubble.velX == 0 && launcherBubble.velY == 0 && !isSwapping) {
        isSwapping = true; 
        swapAnimationTimer =SWAP_ANIMATION_DURATION; 

        double angleRad = launcherAngle * M_PI / 180.0;
        swapLauncherStartX = SCREEN_WIDTH/2 + 80 * cos(angleRad);
        swapLauncherStartY = 0 + 80 * sin(angleRad);

        swapNextStartX = SCREEN_WIDTH/2 + 150;
        swapNextStartY = 20;
    }
    }
    else if((key=='h'||key=='H') && mode==1 && !gameOver && launcherBubble.velX==0 && launcherBubble.velY==0 && !sused){
        if(!sbubble && launcherBubble.colorIndex!=POWER_BALLB_INDEX&&launcherBubble.colorIndex!=sbubblecolor){
            launcherBubble.colorIndex=sbubblecolor;
            sbubble=true;
            ///savailable=true;
            sused=true; 
        }
    }
}


void iMouseMove(int mx, int my) 
{
    if (mode == 1) {
        if (isPaused) return;
        double dx = mx - (SCREEN_WIDTH / 2.0);
        double dy = my - 0;
        launcherAngle = atan2(dy, dx) * 180.0 / M_PI;

        if (launcherAngle < 15) launcherAngle = 15;
        if (launcherAngle > 165) launcherAngle = 165;
    }
    else if(mode==0){

    if(mx >= 255 && mx <= 555 && my >= 390 && my <= 430 && nclick==0){ 

    nclick=!nclick;hclick=0;seclick=0;aclick=0;sclick=0;
    newgame=!newgame; savedgame=0;highscore=0;settings=0;aboutus=0;
    iPlaySound("assets/sounds/button_sound.wav", false,80);
    }
    else if(mx >= 230 && mx <= 570 && my >= 340 && my <= 380 && sclick==0 ) {

sclick=!sclick; nclick=0;seclick=0;aclick=0;hclick=0;
    savedgame=!savedgame;newgame=0;highscore=0;settings=0;aboutus=0;
    iPlaySound("assets/sounds/button_sound.wav", false,80); } 

    else if(mx >= 240 && mx <= 560 && my >= 280 && my <= 320 && hclick==0) {

    hclick=!hclick; nclick=0;seclick=0;aclick=0;sclick=0;
    highscore=!highscore; savedgame=0;newgame=0;settings=0;aboutus=0;
    iPlaySound("assets/sounds/button_sound.wav", false,80);}

    else if(mx >= 290 && mx <= 520 && my >= 225 && my <= 265 &&  seclick==0) { 

    seclick=!seclick; nclick=0;aclick=0;hclick=0;sclick=0;
    settings=!settings ;savedgame=0;highscore=0;newgame=0;aboutus=0;
    iPlaySound("assets/sounds/button_sound.wav", false,80);}

    else if(mx >= 270 && mx <= 540 && my >= 155 && my <= 195 && aclick==0) {
 
    aclick=!aclick; nclick=0; seclick=0;hclick=0;sclick=0;
    aboutus=!aboutus ;savedgame=0;highscore=0;settings=0;newgame=0;
      iPlaySound("assets/sounds/button_sound.wav", false,80);}

    }
}


// --- Unused iGraphics functions ---
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}
void iSpecialKeyboard(unsigned char key) {}

int main(int argc, char *argv[]) 
{
    glutInit(&argc, argv);
    iInitializeSound();
    bgSoundIdx= iPlaySound("assets/sounds/bgsound.wav", true,50);
    loadHighScores(); 
    iSetTimer(16, gameLogicUpdate); //to get almost 60 fps
    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Bubble Shooter");
    return 0;
}