#define fallingPieceNumberOfTiles 3

typedef struct {
    u8 board[9][18];//7 wide by 16 tall. [0][0] not used. array is [9] because [8] gets fucky with the inner tiles
    bool boardDestructionQueue[9][18];

    u8 flag_status;
    bool flag_redraw;

    u8 blinkTimes;

    Sprite* fallingPieceSprite[fallingPieceNumberOfTiles];
    u8 fallingPiece[fallingPieceNumberOfTiles];//used in createPiece
    u8 nextPiece[fallingPieceNumberOfTiles];
    u16 spriteX;
    s16 spriteY;
    u8 xPosition,yPosition;

    u8 leftright[4][16];
    u8 updown[7][8];
    u8 innerconnect[4][7][2];

    u8 fallingIncrement;

    u8 moveDelay;

    u8 cycleDelay;
    bool has_released_cycle;

    u8 drawStartX,drawStartY,drawEndX,drawEndY;

    u8 damageToBeReceived;

    bool flag_locking;
    u8 lockingLateralCounter;

    u8 chainAmount;
    u8 howManyDestroyed;
} Player;

Player P1;
Player P2;

u8 randomRange(u8 rangeStart, u8 rangeEnd);
void initialize();
void loadTiles();
void loadDebugFieldData();
void printDebug();

void clearBoardData(Player* player);
void drawFallingSprite(Player* player);
void drawFullTile(Player* player, u8 xPos, u8 yPos);
void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY);

void gameLogicSwitch(Player* player);

void doCycle(Player* player, u8 direction);
void handleInput(Player* player, u16 buttons);
void manageDelays();
bool collisionTest(Player* player, u8 direction);
void checkMatches(Player* player);
void processGravity(Player* player);

void setSharedNext();
void drawSharedNext();
void drawPlayerNext(Player* player);
void createPiece(Player* player);

void effectFastDrop(Player* player);
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);

void sendDamage(Player* player, u8 amountDamageTaken);

void processDestroy(Player* player);
void blinkMatches(Player* player);

u8 sharedNext[fallingPieceNumberOfTiles];
u8 sharedNextStatus;
char debug_string[40] = "";

#define TILESIZE 12

#define maxX 7
#define maxY 17 //because 0 is the top of the spawning piece

#define xOffset 0
#define yOffset 1

#define ySpawn 0
#define topOutYpos 2

#define spriteXorigin 44
#define spriteYorigin -36//-24

#define player2offset 27
#define p2spriteXcreate (18*TILESIZE)

enum status{
    spawningPiece,

    fallingPiece,
    checkingMatches,
    blinkingMatches,
    doingGravity,
    destroyingMatches,

    toppedOut,
};

enum direction{
    TOP = 1,
    BOTTOM = 2,
    LEFT = 3,
    RIGHT = 4
};

#define globalNumColors 6//this includes gray garbage color

enum colors{
    COLOR_BLANK = 0,
    COLOR_RED = 1,
    COLOR_YELLOW = 2,
    COLOR_GREEN = 3,
    COLOR_PURPLE = 4,
    COLOR_ORANGE = 5,
    COLOR_GARBAGE = globalNumColors
};

#define false 0
#define true 1

#define DOWN 0
#define UP 1

#define MOVE_DELAY_AMOUNT 6
#define CYCLE_DELAY_AMOUNT 8

#define ADDAMOUNT 4
#define ADDAMOUNT2 8
#define ADDAMOUNT3 12
#define ADDAMOUNT4 16
#define ADDAMOUNT5 20
#define extra_tiles_start  (4 + (ADDAMOUNT*(globalNumColors-1)) + 1)

enum timers{
    P1fallLockingTimer,
    P2fallLockingTimer,
    P1_blink_timer,
    P2_blink_timer
};

#define lockingDelayMaxTime 24000//24000 //higher is more time to let the player lock
#define maxLaterals maxX-1 //how many times a player can fiddle with their sonic dropped piece

void loadTiles()
{
    VDP_loadFontData(tileset_Font.tiles, 96, CPU);
    
    VDP_loadTileSet(fullblock_color1.tileset,1,CPU);
    VDP_loadTileSet(fullblock_color2.tileset,1+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(fullblock_color3.tileset,1+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(fullblock_color4.tileset,1+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(fullblock_color5.tileset,1+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(fullblock_color6.tileset,1+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(topblock_color1.tileset,2,CPU);
    VDP_loadTileSet(topblock_color2.tileset,2+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(topblock_color3.tileset,2+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(topblock_color4.tileset,2+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(topblock_color5.tileset,2+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(topblock_color6.tileset,2+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(rightblock_color1.tileset,3,CPU);
    VDP_loadTileSet(rightblock_color2.tileset,3+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(rightblock_color3.tileset,3+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(rightblock_color4.tileset,3+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(rightblock_color5.tileset,3+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(rightblock_color6.tileset,3+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(cornerblock_color1.tileset,4,CPU);
    VDP_loadTileSet(cornerblock_color2.tileset,4+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(cornerblock_color3.tileset,4+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(cornerblock_color4.tileset,4+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(cornerblock_color5.tileset,4+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(cornerblock_color6.tileset,4+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(leftright12.tileset,extra_tiles_start+0,CPU);//1,2
    VDP_loadTileSet(leftright13.tileset,extra_tiles_start+1,CPU);//1,3
    VDP_loadTileSet(leftright14.tileset,extra_tiles_start+2,CPU);
    VDP_loadTileSet(leftright15.tileset,extra_tiles_start+3,CPU);
    VDP_loadTileSet(leftright16.tileset,extra_tiles_start+4,CPU);

    VDP_loadTileSet(leftright23.tileset,extra_tiles_start+5,CPU);//2,3
    VDP_loadTileSet(leftright24.tileset,extra_tiles_start+6,CPU);
    VDP_loadTileSet(leftright25.tileset,extra_tiles_start+7,CPU);
    VDP_loadTileSet(leftright26.tileset,extra_tiles_start+8,CPU);

    VDP_loadTileSet(leftright34.tileset,extra_tiles_start+9,CPU);
    VDP_loadTileSet(leftright35.tileset,extra_tiles_start+10,CPU);
    VDP_loadTileSet(leftright36.tileset,extra_tiles_start+11,CPU);

    VDP_loadTileSet(leftright45.tileset,extra_tiles_start+12,CPU);
    VDP_loadTileSet(leftright46.tileset,extra_tiles_start+13,CPU);

    VDP_loadTileSet(leftright56.tileset,extra_tiles_start+14,CPU);


    VDP_loadTileSet(updown12.tileset,extra_tiles_start+15,CPU);//1,2
    VDP_loadTileSet(updown13.tileset,extra_tiles_start+16,CPU);//1,3
    VDP_loadTileSet(updown14.tileset,extra_tiles_start+17,CPU);
    VDP_loadTileSet(updown15.tileset,extra_tiles_start+18,CPU);
    VDP_loadTileSet(updown16.tileset,extra_tiles_start+19,CPU);

    VDP_loadTileSet(updown23.tileset,extra_tiles_start+20,CPU);//2,3
    VDP_loadTileSet(updown24.tileset,extra_tiles_start+21,CPU);
    VDP_loadTileSet(updown25.tileset,extra_tiles_start+22,CPU);
    VDP_loadTileSet(updown26.tileset,extra_tiles_start+23,CPU);

    VDP_loadTileSet(updown34.tileset,extra_tiles_start+24,CPU);
    VDP_loadTileSet(updown35.tileset,extra_tiles_start+25,CPU);
    VDP_loadTileSet(updown36.tileset,extra_tiles_start+26,CPU);

    VDP_loadTileSet(updown45.tileset,extra_tiles_start+27,CPU);
    VDP_loadTileSet(updown46.tileset,extra_tiles_start+28,CPU);

    VDP_loadTileSet(updown56.tileset,extra_tiles_start+29,CPU);
}

#define innerSectionsVRAM extra_tiles_start+30// 64 tiles worth of VRAM (32 per player)
#define endOfInnerSectionsVRAM innerSectionsVRAM+64

void clearBoardData(Player* player)//only called at initialization
{
    for (u8 boardX=1;boardX<maxX+1;boardX++)
    {
        for (u8 boardY=1;boardY<maxY+1;boardY++)
        {
            player->board[boardX][boardY]=0;
        }
    }
}

u8 randomRange(u8 rangeStart, u8 rangeEnd)//general use function
{
    return (random() % (rangeEnd + 1 - rangeStart)) + rangeStart;
}

void loadDebugFieldData()
{
    for (u8 x=1;x<maxX+1;x++)
    {
        for (u8 y=7;y<maxY+1;y++)
        {
            P1.board[x][y]=randomRange(1,(globalNumColors-1));//because one color is reserved for garbage
            P2.board[x][y]=randomRange(1,(globalNumColors-1));
        }
    }

/*
    P1.board[1][maxY]=randomRange(1,P1.numColors);
    P1.board[2][maxY]=randomRange(1,P1.numColors);

    P1.board[4][maxY]=randomRange(1,P1.numColors);
    P1.board[5][maxY]=randomRange(1,P1.numColors);

    P1.board[maxX-1][maxY]=randomRange(1,P1.numColors);
    P1.board[maxX][maxY]=randomRange(1,P1.numColors);
*/
}

Sprite* sharedNextSpawnCloud;

Sprite* patrako_idle;
Sprite* patrako_lost;
Sprite* patrako_cheer;
bool patrako_is_cheering;

Sprite* sakuraSpr;

void initialize()
{
    //P1.numColors=globalNumColors;//this would be used for having diff # of total colors for the players, but with only 5 max it doesn't make sense
    //P2.numColors=P1.numColors;

    sharedNextSpawnCloud = SPR_addSpriteSafe(&cloud, -64, -64, TILE_ATTR(PAL1, FALSE, FALSE, FALSE));//TILE_ATTR(pal, prio, flipV, flipH)
    SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);
    SPR_setPosition(sharedNextSpawnCloud,148,27);

    PAL_setPalette(PAL2,patrakoIdle.palette->data,DMA);
    patrako_idle = SPR_addSpriteSafe(&patrakoIdle, 110, 144, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    patrako_cheer = SPR_addSpriteSafe(&patrakoCheer, 98, 130, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(patrako_cheer,HIDDEN);

    patrako_is_cheering=false;

    PAL_setPalette(PAL1,sakura.palette->data,DMA);
    sakuraSpr = SPR_addSpriteSafe(&sakura, 158, 136,TILE_ATTR(PAL1, FALSE, FALSE, FALSE));

    clearBoardData(&P1);
    clearBoardData(&P2);

    for (u8 createIndex=0;createIndex<3;createIndex++)
    {
        P1.fallingPieceSprite[createIndex] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
        P2.fallingPieceSprite[createIndex] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));

        SPR_setVisibility(P1.fallingPieceSprite[createIndex],HIDDEN);
        SPR_setVisibility(P2.fallingPieceSprite[createIndex],HIDDEN);

        P1.nextPiece[createIndex]=randomRange(1,(globalNumColors-1));
        P2.nextPiece[createIndex]=P1.nextPiece[createIndex];//this doesn't work to give them the same color to start
    }

    //P1.flag_status=spawningPiece;
    //P2.flag_status=spawningPiece;

    //P1.fallingIncrement=0;
    //P2.fallingIncrement=0;

    P1.has_released_cycle=true;
    P2.has_released_cycle=true;

    P1.blinkTimes=0;
    P2.blinkTimes=0;
}

void drawFallingSprite(Player* player)
{
    SPR_setPosition(player->fallingPieceSprite[2],player->spriteX,player->spriteY);
    SPR_setPosition(player->fallingPieceSprite[1],player->spriteX,player->spriteY-TILESIZE);
    SPR_setPosition(player->fallingPieceSprite[0],player->spriteX,player->spriteY-TILESIZE-TILESIZE);
}

u8 globalSpawnCloudVisibilityTimer;//replace with SPR_getAnimationDone(sharedNextSpawnCloud) in later SGDK

void setSharedNext()
{
    SPR_setFrame(sharedNextSpawnCloud, 0);//cloud sprite
    SPR_setVisibility(sharedNextSpawnCloud,VISIBLE);
    globalSpawnCloudVisibilityTimer=0;

    for (u8 i=0;i<fallingPieceNumberOfTiles;i++)
    {
        sharedNext[i]=randomRange(1,(globalNumColors-1));
    }

    sharedNextStatus=1;
}

void drawSharedNext()
{
    #define nextPieceYPos 4
    #define sharedNextxPos 20
    u8 sharedNextyPos=nextPieceYPos;
    u8 colorAdd=0;

    for (u8 i=0;i<fallingPieceNumberOfTiles;i++)
    {
        colorAdd=(sharedNext[i]-1)<<2;//multiply by 4
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), sharedNextxPos, sharedNextyPos, 1, 1);
        sharedNextyPos++;
    }

    sharedNextStatus=0;
}

void drawPlayerNext(Player* player)
{
    u8 playerNextxPos=15;
    if(player==&P2)playerNextxPos+=10;
    u8 playerNextyPos=nextPieceYPos;
    u8 colorAdd=0;

    for (u8 i=0;i<fallingPieceNumberOfTiles;i++)
    {
        colorAdd=(player->nextPiece[i]-1)<<2;//multiply by 4
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), playerNextxPos, playerNextyPos, 1, 1);

        playerNextyPos++;        
    }

    drawSharedNext();
}

void doCycle(Player* player, u8 direction)
{
    u8 tempPieceHolder;

    if(direction==DOWN)
    {
        tempPieceHolder=player->fallingPiece[0];
        player->fallingPiece[0]=player->fallingPiece[2];
        player->fallingPiece[2]=player->fallingPiece[1];
        player->fallingPiece[1]=tempPieceHolder;
    }
    else if(direction==UP)
    {
        tempPieceHolder=player->fallingPiece[0];
        player->fallingPiece[0]=player->fallingPiece[1];
        player->fallingPiece[1]=player->fallingPiece[2];
        player->fallingPiece[2]=tempPieceHolder;
    }

    for (u8 spriteIndex=0;spriteIndex<3;spriteIndex++)SPR_setFrame(player->fallingPieceSprite[spriteIndex],player->fallingPiece[spriteIndex]-1);
}

bool collisionTest(Player* player, u8 direction)
{
    if(player->flag_locking==false)
    {
        if(direction==LEFT)
        {
            if(player->board[player->xPosition-1][player->yPosition+1]!=0)return true;
            if(player->xPosition == 1)return true;

            if(player->board[player->xPosition-1][player->yPosition]!=0 && player->yPosition==maxY)return true;//stop shifting at the bottom
        }
        else if(direction==RIGHT)
        {
            if(player->board[player->xPosition+1][player->yPosition+1]!=0)return true;
            if(player->xPosition == maxX)return true;
            
            if(player->board[player->xPosition+1][player->yPosition]!=0 && player->yPosition==maxY)return true;//stop shifting at the bottom
        }
    }
    else if(player->flag_locking==true)
    {
        if(direction==LEFT)
        {
            if(player->xPosition == 1)return true;
            if(player->board[player->xPosition-1][player->yPosition]!=0)return true;
        }
        else if(direction==RIGHT)
        {
            if(player->xPosition == maxX)return true;
            if(player->board[player->xPosition+1][player->yPosition]!=0)return true;
        }
    }
    
    if(direction==BOTTOM)
    {
        if(player->yPosition >= maxY) return true;
        if(player->board[player->xPosition][player->yPosition+1]!=0) return true;
    }

    return false;
}

void createPiece(Player* player)
{
    if(player==&P1)player->spriteX=spriteXorigin;
    else if(player==&P2)player->spriteX=spriteXorigin+p2spriteXcreate;
    player->spriteY=spriteYorigin+TILESIZE+TILESIZE+TILESIZE;

    for (u8 createIndex=0;createIndex<3;createIndex++)
    {
        player->fallingPiece[createIndex]=player->nextPiece[createIndex];//this is assigning color
        SPR_setFrame(player->fallingPieceSprite[createIndex],player->fallingPiece[createIndex]-1);

        player->nextPiece[createIndex]=sharedNext[createIndex];
    }

    player->xPosition=4;
    player->yPosition=ySpawn;
    player->moveDelay=0;

    player->howManyDestroyed=0;
    player->chainAmount=0;//reset chain counter

    SPR_setVisibility(player->fallingPieceSprite[2],VISIBLE);//bottom piece
    SPR_setVisibility(player->fallingPieceSprite[1],VISIBLE);//middle piece
    SPR_setVisibility(player->fallingPieceSprite[0],VISIBLE);//top piece

    
    player->flag_locking=false;
    player->flag_status=fallingPiece;
    KLog("$fallingPiece (set in createPiece)");

    setSharedNext();
    drawPlayerNext(player);
}

void checkMatches(Player* player)
{
    u8 connectionAmount,connectionColor;

    for (u8 checkX=1;checkX<maxX+1;checkX++)
    {
        for (u8 checkY=maxY+1;checkY>0;checkY--)
        {
            if(player->board[checkX][checkY]!=COLOR_BLANK && player->board[checkX][checkY]!=COLOR_GARBAGE)
            {
                if(player->board[checkX][checkY]==player->board[checkX+1][checkY])//match horizontally 2 tiles
                {
                    //KLog("INIT match - hori");
                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];

                    for (u8 advance=checkX+2;advance<maxX+1;advance++)
                    {
                        if(player->board[advance][checkY]==connectionColor)connectionAmount++;
                        else if(player->board[advance][checkY]!=connectionColor)break;
                    }

                    if(connectionAmount>=3){//KLog("***match - hori");
                        
                        for (u8 xAddDestructionQueue=0;xAddDestructionQueue<connectionAmount;xAddDestructionQueue++)
                        {
                            player->boardDestructionQueue[checkX+xAddDestructionQueue][checkY]=true;
                        }
                        player->flag_status=blinkingMatches;//KLog("blinkingMatches");
                    }
                }

                if(player->board[checkX][checkY]==player->board[checkX][checkY-1])//match vertically 2 tiles
                {
                    //KLog("INIT match - vert");
                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];

                    //sprintf(debug_string,"init vert match at %d,%d",checkX,checkY);
                    //VDP_drawText(debug_string,13,28);

                    for (u8 advance=checkY-2;advance>0;advance--)
                    {
                        if(player->board[checkX][advance]==connectionColor)connectionAmount++;
                        else if(player->board[checkX][advance]!=connectionColor)break;
                    }

                    if(connectionAmount>=3){//KLog("***match - vert");
                        for (u8 yAddDestructionQueue=0;yAddDestructionQueue<connectionAmount;yAddDestructionQueue++)
                        {
                            player->boardDestructionQueue[checkX][checkY-yAddDestructionQueue]=true;
                        }
                        player->flag_status=blinkingMatches;//KLog("blinkingMatches");
                    }
                }

                if(player->board[checkX][checkY]==player->board[checkX+1][checkY-1])//match diagonally up 2 tiles
                {
                    //KLog("init match - diagUP");
                    //sprintf(debug_string,"init diagUp match at %d,%d",checkX,checkY);
                    //VDP_drawText(debug_string,13,28);

                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];
                    
                    u8 incrementer=2;

                    for (u8 advance=checkY-2;(advance>0 && ((checkX+incrementer)<(maxX+1)));advance--)
                    {
                        if(player->board[checkX+incrementer][advance]==connectionColor)connectionAmount++;
                        else if(player->board[checkX+incrementer][advance]!=connectionColor)break;

                        incrementer++;
                    }
                    
                    if(connectionAmount>=3){//KLog("***match - diagUp");
                        for (u8 i=0;i<connectionAmount;i++)
                        {
                            player->boardDestructionQueue[checkX+i][checkY-i]=true;

                            //sprintf(debug_string,"diagUP %d,%d",checkX+i,checkY-i);
                            //VDP_drawText(debug_string,13,19+i);
                        }
                        player->flag_status=blinkingMatches;//KLog("blinkingMatches");
                    }
                
                }

                if(player->board[checkX][checkY]==player->board[checkX+1][checkY+1])//match diagonally down 2 tiles
                {
                    //KLog("INIT match - diagDOWN");
                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];

                    u8 incrementer=2;

                    for (u8 advance=checkY+2;(advance<maxY+1 && ((checkX+incrementer)<(maxX+1)));advance++)
                    {
                        if(player->board[checkX+incrementer][advance]==connectionColor)connectionAmount++;
                        else if(player->board[checkX+incrementer][advance]!=connectionColor)break;

                        incrementer++;
                    }

                    if(connectionAmount>=3){//KLog("***match - diagDown");
                        for (u8 i=0;i<connectionAmount;i++)
                        {
                            player->boardDestructionQueue[checkX+i][checkY+i]=true;
                        
                            //sprintf(debug_string,"diagDOWN %d,%d",checkX+i,checkY+i);
                            //VDP_drawText(debug_string,13,19+i);
                        }
                        player->flag_status=blinkingMatches;//KLog("blinkingMatches");
                    }
                }
            }
        }
    }
    if(player->flag_status!=blinkingMatches)player->flag_status=spawningPiece;//there were no connections
}

/*
void sendDamage(Player* player, u8 amountDamageTaken)
{
    u8 sendingX=1;
    u8 sendingY=0;

    for(u8 damageAmount=0;damageAmount<amountDamageTaken;damageAmount++)
    {
        player->board[sendingX][sendingY]=6;
        sendingX++;
        if(sendingX>7)
            {
                sendingX=1;
                sendingY++;
            }
    }
    player->damageToBeReceived-=amountDamageTaken;

    processGravity(player);
}
*/

void effectFastDrop(Player* player)
{
    s8 i;//has to be outside of the for loop so it can be used afterwards

    for(i=player->yPosition;i<maxY;i++)
        if(player->board[player->xPosition][i+1]!=0)break;
    
    player->yPosition=i;
    player->spriteY=(i<<3)+(i<<2)+1;//player->spriteY=i*12

    player->fallingIncrement=0;
}


void manageFalling(Player* player)
{
    if(collisionTest(player, BOTTOM)==false)
    {
        player->fallingIncrement++;

        if(player->fallingIncrement>=TILESIZE)
        {
            player->yPosition++;
            player->fallingIncrement=0;
        }

        player->spriteY++;

        player->flag_locking=false;//reset
    }
    else if(player->flag_locking==false)
    {
        getTimer(P1fallLockingTimer,true);//start the timer
        player->lockingLateralCounter=0;
        player->flag_locking=true;

        return;
    }
    else if(player->flag_locking==true)
    {
        if(player->fallingIncrement>TILESIZE)player->spriteY+=(TILESIZE-player->fallingIncrement);
        if((player->lockingLateralCounter>=maxLaterals || (getTimer(P1fallLockingTimer,false)>=lockingDelayMaxTime)))pieceIntoBoard(player);
    }
}

void pieceIntoBoard(Player* player)
{
    KLog("$PIECE INTO BOARD");
//write the colors of the locked pieces into the array
    player->board[player->xPosition][player->yPosition]=player->fallingPiece[2];
    player->board[player->xPosition][player->yPosition-1]=player->fallingPiece[1];
    player->board[player->xPosition][player->yPosition-2]=player->fallingPiece[0];

//set falling sprites to invis
    SPR_setVisibility(player->fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(player->fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(player->fallingPieceSprite[2],HIDDEN);

    KLog("^^pieceIntoBoard updated the draw parameters");
    player->drawStartX=player->xPosition;
    player->drawStartY=player->yPosition-2;

    player->drawEndX=player->xPosition+1;
    player->drawEndY=player->yPosition+1;

    player->flag_redraw=true;

    if(player==&P1)
    {
        sprintf(debug_string,"        ");//this is to clear out the combo text
        VDP_drawText(debug_string,2,1);

        sprintf(debug_string,"        ");//this is to clear out the chain text
        VDP_drawText(debug_string,2,2);
    }

    if(player->board[4][1]!=0)player->flag_status=toppedOut;
    else player->flag_status=checkingMatches;
}

void processGravity(Player* player)
{
    u8 howMuchGravity=0;

    //u8 endGravityX=0;
    u8 firstGravityY=player->drawStartY;

    for (u8 gravityX=1;gravityX<maxX+1;gravityX++)
    //for (u8 gravityX=maxX;gravityX>0;gravityX--)
    {
        for (u8 gravityY=maxY;gravityY>0;gravityY--)//#define maxY 17
        {
            if (player->board[gravityX][gravityY]==0 && player->board[gravityX][gravityY-1]!=0)
            {
                player->board[gravityX][gravityY]=player->board[gravityX][gravityY-1];
                player->board[gravityX][gravityY-1]=0;

                if(gravityY<firstGravityY)firstGravityY=gravityY;

                //if(endGravityX==0)endGravityX=gravityX;
                //if(firstGravityY==0)firstGravityY=gravityY;

                gravityY=maxY+1;

                howMuchGravity++;
            }
        }
    }

    KLog("^^processGravity DID NOT update the draw parameters");
    //KLog("^^processGravity updated the draw parameters");
    //player->drawStartX=1;
    //player->drawStartY=firstGravityY-1;

    //player->drawEndX=maxX+1;
    //player->drawEndY=maxY+1;
    player->flag_redraw=true;

    player->flag_status=checkingMatches;

    //if(howMuchGravity>0)KLog_U1("gravity moved ",howMuchGravity);
}

void manageDelays()
{
    if(P1.moveDelay>0)P1.moveDelay--;
    if(P2.moveDelay>0)P2.moveDelay--;

    if(P1.cycleDelay>0)P1.cycleDelay--;
    if(P2.cycleDelay>0)P2.cycleDelay--;

    if(globalSpawnCloudVisibilityTimer<40)globalSpawnCloudVisibilityTimer++;
}

void handleInput(Player* player, u16 buttons)
{
    //KLog("handleinput started");
    if(player->moveDelay==0 && player->yPosition>ySpawn)//ySpawn=0
    {
        if(buttons & BUTTON_LEFT && collisionTest(player, LEFT)==FALSE)
        {
            player->xPosition--;
            player->moveDelay=MOVE_DELAY_AMOUNT;
            player->spriteX-=TILESIZE;

            if(player->flag_locking==true)player->lockingLateralCounter++;
        }
        else if(buttons & BUTTON_RIGHT && collisionTest(player, RIGHT)==FALSE)
        {
            player->xPosition++;
            player->moveDelay=MOVE_DELAY_AMOUNT;
            player->spriteX+=TILESIZE;

            if(player->flag_locking==true)player->lockingLateralCounter++;
        }
    }

    if(collisionTest(player, BOTTOM)==FALSE)
    {
        if (buttons & BUTTON_DOWN)
        {
            #define holdDownFallAmount 2

            if(player->fallingIncrement<TILESIZE-holdDownFallAmount)
            {
                player->fallingIncrement+=holdDownFallAmount;
                player->spriteY+=holdDownFallAmount;
            }
        }

        //if (buttons & BUTTON_UP && player->moveDelay==0)
        if (buttons & BUTTON_UP && player->moveDelay<=1)
        {
            effectFastDrop(player);
        }
    }
    else if((collisionTest(player, BOTTOM)==TRUE) && (buttons & BUTTON_DOWN))player->lockingLateralCounter=maxLaterals;//lock in

    if(!(buttons & BUTTON_C) && !(buttons & BUTTON_B))player->has_released_cycle=true;

    if(player->cycleDelay==0 && player->has_released_cycle==true)
    {
        if (buttons & BUTTON_C)
        {
            doCycle(player, DOWN);
            player->cycleDelay=CYCLE_DELAY_AMOUNT;
            player->has_released_cycle=false;

        }
        else if (buttons & BUTTON_B)
        {
            doCycle(player, UP);
            player->cycleDelay=CYCLE_DELAY_AMOUNT;
            player->has_released_cycle=false;
        }
    }
}

void gameLogicSwitch(Player* player)
{
    switch(player->flag_status)
    {
        case spawningPiece:
            KLog("$spawned piece");
            createPiece(player);
            break;

        case fallingPiece:
            if(player==&P1)handleInput(player, JOY_readJoypad(JOY_1));
            else if(player==&P2)handleInput(player, JOY_readJoypad(JOY_2));
            manageFalling(player);
            break;

        case checkingMatches:
            KLog("$checkingMatches");
            checkMatches(player);
            break;

        case blinkingMatches:
            blinkMatches(player);
            break;

        case destroyingMatches:
            processDestroy(player);
            break;

        case doingGravity:
            processGravity(player);
            break;
    }
}

void printDebug()
{
    sprintf(debug_string,"%ldFPS", SYS_getFPS());
    VDP_drawText(debug_string,13,27);

    sprintf(debug_string,"%d", SYS_getCPULoad());
    strcat(debug_string, "%");
    strcat(debug_string, "CPU");
    VDP_drawText(debug_string,21,27);

    if(SYS_getCPULoad()>100)KLog("CPU usage over 100%");
    //KLog_U1("CPU USAGE ",SYS_getCPULoad());

    //sprintf(debug_string,"P1 %d", P1.flag_status);
    //VDP_drawText(debug_string,32,1);

    if(P1.flag_status==toppedOut)
    {
        sprintf(debug_string,"TOPPED OUT");
        VDP_drawText(debug_string,1,2);
    }
    if(P2.flag_status==toppedOut)
    {
        sprintf(debug_string,"TOPPED OUT");
        VDP_drawText(debug_string,28,2);
    }
}

void gameOver()
{
    SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);
    SPR_setVisibility(P1.fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(P1.fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(P1.fallingPieceSprite[2],HIDDEN);
    SPR_setVisibility(P2.fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(P2.fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(P2.fallingPieceSprite[2],HIDDEN);

    SPR_releaseSprite(patrako_idle);
    SPR_releaseSprite(patrako_cheer);

    patrako_lost = SPR_addSpriteSafe(&patrakoLost, 106,152, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    u8 frameCounter=0;
    while(frameCounter<30)
    {
        frameCounter++;
        SYS_doVBlankProcess();
        SPR_update();
    }
}