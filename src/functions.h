#define fallingPieceNumberOfTiles 3

typedef struct {

    u8 board[9][18];//7 wide by 16 tall. [0][0] not used. array is [9] because [8] gets fucky with the inner tiles
    bool boardDestructionQueue[9][18];

    bool flag_destroy;
    bool flag_gravity;
    bool flag_checkmatches;
    bool flag_redraw;

    u8 flag_status;

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
    //u8 fallDelay;

    u8 rotateDelay;
    u8 has_let_go_A;
    u8 has_let_go_B;

    u8 drawStartX,drawStartY,drawEndX,drawEndY;

    u8 chainAmount;

    u8 damageToBeReceived;
} Player;

Player P1;
Player P2;

u8 randomRange(u8 rangeStart, u8 rangeEnd);
void initialize();
void loadTiles();
void loadDebugFieldData();
void clearBoard(Player* player);
void drawFallingSprite(Player* player);
void printBoardAll();
void drawTile(Player* player, u8 xPos, u8 yPos);
void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY);
void doRotate(Player* player, u8 direction);

void handleInput(Player* player, u16 buttons);
bool collisionTest(Player* player, u8 direction);

void setSharedNext();
void drawSharedNext();
void drawPlayerNext(Player* player);
void createPiece(Player* player);

u8 sharedNext[fallingPieceNumberOfTiles];
u8 sharedNextStatus;
char debug_string[40] = "";

#define TILESIZE 12

#define maxX 7
#define maxY 17 //because 0 is the top of the spawning piece

#define xOffset 0
#define yOffset 1

#define ySpawn 0

#define spriteXorigin 44
#define spriteYorigin -36//-24

#define player2offset 27
#define p2spriteXcreate (18*TILESIZE)

enum status
{
    nothing,
    needPiece,
    toppedOut,
};

enum direction
{
    TOP = 1,
    BOTTOM = 2,
    LEFT = 3,
    RIGHT = 4
};

#define false 0
#define true 1

#define DOWN 0
#define UP 1

#define MOVE_DELAY_AMOUNT 6
//#define FALL_DELAY_AMOUNT 4
#define ROTATE_DELAY_AMOUNT 8

#define globalNumColors 6
#define ADDAMOUNT 4
#define ADDAMOUNT2 8
#define ADDAMOUNT3 12
#define ADDAMOUNT4 16
#define ADDAMOUNT5 20
#define extra_tiles_start  (4 + (ADDAMOUNT*(globalNumColors-1)) + 1)

enum timers
{
    P1destroyTimer,
    P2destroyTimer,
    P1fallLockingTimer,
    P2fallLockingTimer
};

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

void clearBoard(Player* player)
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

void initialize()
{
    //P1.numColors=globalNumColors;
    //P2.numColors=P1.numColors;

    clearBoard(&P1);
    clearBoard(&P2);

    for (u8 createIndex=0;createIndex<3;createIndex++)
    {
        P1.fallingPieceSprite[createIndex] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
        P2.fallingPieceSprite[createIndex] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));

        SPR_setVisibility(P1.fallingPieceSprite[createIndex],HIDDEN);
        SPR_setVisibility(P2.fallingPieceSprite[createIndex],HIDDEN);

        P1.nextPiece[createIndex]=randomRange(1,(globalNumColors-1));
        P2.nextPiece[createIndex]=P1.nextPiece[createIndex];//this doesn't work to give them the same color to start
    }

    P1.flag_status=needPiece;
    P2.flag_status=needPiece;

    P1.fallingIncrement=0;
    P2.fallingIncrement=0;

    P1.has_let_go_A=true;
    P1.has_let_go_B=true;
    P2.has_let_go_A=true;
    P2.has_let_go_B=true;

    startTimer(0);//p1
    startTimer(1);//p2
}

void drawTile(Player* player, u8 xPos, u8 yPos)//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
{
    u8 colorAdd=0;//4 tiles for each color. so color 2 is adding 4, color 3 is adding 8
    bool flag_erase=false;

    //if we are drawing a tile other than color 1, we need to increase the tile index    
    if(player->board[xPos][yPos]>1)colorAdd=(player->board[xPos][yPos]-1)<<2;//multiply by 4
    else if(player->board[xPos][yPos]==0)flag_erase=true;

    u8 p2offsetX=0;
    if(player==&P2)p2offsetX=player2offset;

    u8 drawingxPos=xPos+(xPos>>1)+p2offsetX;
    u8 drawingyPos=yPos+(yPos>>1);

    if(flag_erase==false)
    {
        if(xPos%2!=0 && yPos%2!=0)//odd column, odd row (1,1)
        {
            //top left: bottom half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //top right: bottom left corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 4+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
            //bottom left: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
            //bottom right: left half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);
        }
        else if(xPos%2==0 && yPos%2!=0)//even column, odd row (2,1)
        {
            //top left: bottom right corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 4+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
            //top right: bottom half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //bottom left: right half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);
            //bottom right: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
        }
        else if(xPos%2!=0 && yPos%2==0)//odd column, even row (1,2)
        {
            //top left: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //top right: left half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
            //bottom left: top half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
            //bottom right: top left corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, TRUE, 4+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);//was FALSE,TRUE,5
        }
        else if(xPos%2==0 && yPos%2==0)//even column, even row (2,2)
        {
            //top left: right half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
            //top right: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //bottom left: top right corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 4+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);//was FALSE,FALSE,5
            //bottom right: top half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
        }
    }
    else if(flag_erase==true)
    {
        if(xPos%2!=0 && yPos%2!=0)//odd column, odd row (1,1)
        {
            //top left: bottom half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //top right: bottom left corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 0), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
            //bottom left: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
            //bottom right: left half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 0), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);
        }
        else if(xPos%2==0 && yPos%2!=0)//even column, odd row (2,1)
        {
            //top left: bottom right corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
            //top right: bottom half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //bottom left: right half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);
            //bottom right: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
        }
        else if(xPos%2!=0 && yPos%2==0)//odd column, even row (1,2)
        {
            //top left: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //top right: left half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 0), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
            //bottom left: top half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
            //bottom right: top left corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, TRUE, 0), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);//was FALSE,TRUE,5
        }
        else if(xPos%2==0 && yPos%2==0)//even column, even row (2,2)
        {
            //top left: right half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
            //top right: full square
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
            //bottom left: top right corner
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 0), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);//was FALSE,FALSE,5
            //bottom right: top half
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
        }
    }
}

void drawFallingSprite(Player* player)
{
    SPR_setPosition(player->fallingPieceSprite[2],player->spriteX,player->spriteY);
    SPR_setPosition(player->fallingPieceSprite[1],player->spriteX,player->spriteY-TILESIZE);
    SPR_setPosition(player->fallingPieceSprite[0],player->spriteX,player->spriteY-TILESIZE-TILESIZE);

    if(player->yPosition==1)SPR_setVisibility(player->fallingPieceSprite[2],VISIBLE);
    if(player->yPosition==2)SPR_setVisibility(player->fallingPieceSprite[1],VISIBLE);
    if(player->yPosition==3)SPR_setVisibility(player->fallingPieceSprite[0],VISIBLE);
}

void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY)//from left to right, from top to bottom
{  
    if(startY<2)startY=2;//no need to draw tiles above the barrier area

    for(u8 xDraw=startX;xDraw<endX;xDraw++)
    {
        for(u8 yDraw=startY;yDraw<endY;yDraw++)
        {
           drawTile(player, xDraw,yDraw);
           //if(player->board[xDraw][yDraw]!=0)drawTile(player, xDraw,yDraw);
        }
    }

    u8 p2offsetX=0;
    if(player==&P2)p2offsetX=player2offset;

    s8 drawPosX,drawPosY;
    u8 i;

    //u8 yOddAdder=0;
    //if((startY&1)!=0)yOddAdder=1;

//updown
    for (u8 updownX=startX;updownX<endX;updownX++)//u8 updownX=1 starts on updownX at 1, what happens if it's 2?
    {
        //for (u8 updownY=startY+yOddAdder;updownY<endY+1;updownY+=2)
        for (u8 updownY=2;updownY<maxY;updownY+=2)//u8 updownY=2 starts on updownY at 2, what happens if it's 1?
        {
            if(player->board[updownX][updownY]!=0 || player->board[updownX][updownY+1]!=0)
            {
                i=0;

                player->updown[updownX][i]=(player->board[updownX][updownY]<<4)+player->board[updownX][updownY+1];

                //sprintf(debug_string,"UD:%d", P1.updown[updownX][i]);
                //VDP_drawText(debug_string,34,1+yDrawAdd);

                drawPosX=xOffset+updownX+((updownX)>>1)+p2offsetX;
                drawPosY=yOffset+updownY+((updownY)>>1);

                switch(player->updown[updownX][i])//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
                {
                    case 1://0,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2), drawPosX,drawPosY, 1, 1);
                    break;

                    case 2://0,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+ADDAMOUNT), drawPosX, drawPosY, 1, 1);
                    break;

                    case 3://0,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+ADDAMOUNT2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 4://0,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+ADDAMOUNT3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 5://0,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+ADDAMOUNT4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 6://0,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+ADDAMOUNT5), drawPosX, drawPosY, 1, 1);
                    break;

                    case 16://1,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 17://1,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1), drawPosX, drawPosY, 1, 1);
                    break;

                    case 18://1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+15), drawPosX, drawPosY, 1, 1);
                    break;

                    case 19://1,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+16), drawPosX, drawPosY, 1, 1);
                    break;

                    case 20://1,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+17), drawPosX, drawPosY, 1, 1);
                    break;

                    case 21://1,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+18), drawPosX, drawPosY, 1, 1);
                    break;

                    case 22://1,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+19), drawPosX, drawPosY, 1, 1);
                    break;

                    case 32://2,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+ADDAMOUNT), drawPosX, drawPosY, 1, 1);
                    break;

                    case 33://2,1 - flipped 1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+15), drawPosX, drawPosY, 1, 1);
                    break;

                    case 34://2,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT), drawPosX, drawPosY, 1, 1);
                    break;

                    case 35://2,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+20), drawPosX, drawPosY, 1, 1);
                    break;

                    case 36://2,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+21), drawPosX, drawPosY, 1, 1);
                    break;

                    case 37://2,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+22), drawPosX, drawPosY, 1, 1);
                    break;

                    case 38://2,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+23), drawPosX, drawPosY, 1, 1);
                    break;

                    case 48://3,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+ADDAMOUNT2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 49://3,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+16), drawPosX, drawPosY, 1, 1);
                    break;

                    case 50://3,2 - a flipped 23
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+20), drawPosX, drawPosY, 1, 1);
                    break;

                    case 51://3,3 solid green
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 52://3,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+24), drawPosX, drawPosY, 1, 1);
                    break;

                    case 53://3,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+25), drawPosX, drawPosY, 1, 1);
                    break;

                    case 54://3,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+26), drawPosX, drawPosY, 1, 1);
                    break;

                    case 64://4,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+ADDAMOUNT3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 65://4,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+17), drawPosX, drawPosY, 1, 1);
                    break;

                    case 66://4,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+21), drawPosX, drawPosY, 1, 1);
                    break;

                    case 67://4,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+24), drawPosX, drawPosY, 1, 1);
                    break;

                    case 68://4,4 solid
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 69://4,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+27), drawPosX, drawPosY, 1, 1);
                    break;

                    case 70://4,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+28), drawPosX, drawPosY, 1, 1);
                    break;

                    case 80://5,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+ADDAMOUNT4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 81://5,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+18), drawPosX, drawPosY, 1, 1);
                    break;

                    case 82://5,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+22), drawPosX, drawPosY, 1, 1);
                    break;

                    case 83://5,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+25), drawPosX, drawPosY, 1, 1);
                    break;

                    case 84://5,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+27), drawPosX, drawPosY, 1, 1);
                    break;

                    case 85://5,5 solid
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 86://5,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+29), drawPosX, drawPosY, 1, 1);
                    break;

                    case 96://6,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+ADDAMOUNT5), drawPosX, drawPosY, 1, 1);
                    break;

                    case 97://6,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+19), drawPosX, drawPosY, 1, 1);
                    break;

                    case 98://6,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+23), drawPosX, drawPosY, 1, 1);
                    break;

                    case 99://6,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+26), drawPosX, drawPosY, 1, 1);
                    break;

                    case 100://6,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+28), drawPosX, drawPosY, 1, 1);
                    break;

                    case 101://6,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, extra_tiles_start+29), drawPosX, drawPosY, 1, 1);
                    break;

                    case 102://6,6 solid
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT5), drawPosX, drawPosY, 1, 1);
                    break;
                }
                
            i++;
            
            }
        }
    }

    //u8 xOddAdder=0;
    //if((startX&1)==0)xOddAdder=1;

    //u8 endXadder=0;
    //if(endX<=maxX)endXadder=1;

//leftright 
    for (u8 leftrightY=startY;leftrightY<endY;leftrightY++)
    {
        for (u8 leftrightX=1;leftrightX<maxX;leftrightX+=2)//leftrightX<endX+endXadder u8 leftrightX=startX-xOddAdder;
        {
            if(player->board[leftrightX][leftrightY]!=0 || player->board[leftrightX+1][leftrightY]!=0)
            {
                i=0;

                player->leftright[i][leftrightY]=(player->board[leftrightX][leftrightY]<<4)+player->board[leftrightX+1][leftrightY];
                //now we have the color of the left cell in the left half of this byte and the right color in the right half of this byte

                //sprintf(debug_string,"LR:%d", P1.leftright[i][leftrightY]);
                //VDP_drawText(debug_string,34,1+xDrawAdd);

                drawPosX=xOffset+leftrightX+((leftrightX)>>1)+1+p2offsetX;
                drawPosY=yOffset+leftrightY+((leftrightY-1)>>1);

                switch(player->leftright[i][leftrightY])//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
                {
                    case 1://0,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3), drawPosX,drawPosY, 1, 1);
                    break;

                    case 2://0,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+ADDAMOUNT), drawPosX, drawPosY, 1, 1);
                    break;

                    case 3://0,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+ADDAMOUNT2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 4://0,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+ADDAMOUNT3), drawPosX,drawPosY, 1, 1);
                    break;

                    case 5://0,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+ADDAMOUNT4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 6://0,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+ADDAMOUNT5), drawPosX, drawPosY, 1, 1);
                    break;

                    case 16://1,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 17://1,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1), drawPosX, drawPosY, 1, 1);
                    break;

                    case 18://1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+0), drawPosX, drawPosY, 1, 1);
                    break;

                    case 19://1,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+1), drawPosX, drawPosY, 1, 1);
                    break;

                    case 20://1,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 21://1,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 22://1,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 32://2,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+ADDAMOUNT), drawPosX, drawPosY, 1, 1);
                    break;

                    case 33://2,1 - flipped 1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+0), drawPosX, drawPosY, 1, 1);
                    break;

                    case 34://2,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 1+ADDAMOUNT), drawPosX, drawPosY, 1, 1);
                    break;

                    case 35://2,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+5), drawPosX, drawPosY, 1, 1);
                    break;

                    case 36://2,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+6), drawPosX, drawPosY, 1, 1);
                    break;

                    case 37://2,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+7), drawPosX, drawPosY, 1, 1);
                    break;

                    case 38://2,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+8), drawPosX, drawPosY, 1, 1);
                    break;

                    case 48://3,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+ADDAMOUNT2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 49://3,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+1), drawPosX, drawPosY, 1, 1);
                    break;

                    case 50://3,2 - a flipped 23
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+5), drawPosX, drawPosY, 1, 1);
                    break;

                    case 51://3,3 solid green
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 52://3,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+9), drawPosX, drawPosY, 1, 1);
                    break;

                    case 53://3,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+10), drawPosX, drawPosY, 1, 1);
                    break;

                    case 54://3,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+11), drawPosX, drawPosY, 1, 1);
                    break;

                    case 64://4,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+ADDAMOUNT3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 65://4,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 66://4,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+6), drawPosX, drawPosY, 1, 1);
                    break;

                    case 67://4,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+9), drawPosX, drawPosY, 1, 1);
                    break;

                    case 68://4,4 solid
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 69://4,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+12), drawPosX, drawPosY, 1, 1);
                    break;

                    case 70://4,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+13), drawPosX, drawPosY, 1, 1);
                    break;

                    case 80://5,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+ADDAMOUNT4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 81://5,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 82://5,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+7), drawPosX, drawPosY, 1, 1);
                    break;

                    case 83://5,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+10), drawPosX, drawPosY, 1, 1);
                    break;

                    case 84://5,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+12), drawPosX, drawPosY, 1, 1);
                    break;

                    case 85://5,5 solid
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 86://5,6
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, extra_tiles_start+14), drawPosX, drawPosY, 1, 1);
                    break;

                    case 96://6,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+ADDAMOUNT5), drawPosX, drawPosY, 1, 1);
                    break;

                    case 97://6,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 98://6,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+8), drawPosX, drawPosY, 1, 1);
                    break;

                    case 99://6,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+11), drawPosX, drawPosY, 1, 1);
                    break;

                    case 100://6,4
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+13), drawPosX, drawPosY, 1, 1);
                    break;

                    case 101://6,5
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, extra_tiles_start+14), drawPosX, drawPosY, 1, 1);
                    break;

                    case 102://6,6 solid
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+ADDAMOUNT5), drawPosX, drawPosY, 1, 1);
                    break;
                }
            
            i++;
            
            }
        }
    }

//dynamic inner section vram load + draw
    #define allblank 0x0000
    #define allcolor1 0x4444
    #define allcolor2 0x5555
    #define allcolor3 0x6666
    #define allcolor4 0x7777
    #define allcolor5 0x8888
    #define allgarbage 0x9999

    u32 tile[8];

    u16 leftside,rightside;
    u8 tileIncrementer=0;
    u8 UpperHalfFlag;

    u8 vramOffsetP2=0;
    if(player==&P2)vramOffsetP2=32;

    //if(startX>2)tileIncrementer+=(startX-1);

    //if(startY>3)tileIncrementer-=(startY-1);
    //if(startY>12)tileIncrementer+=1;

    for(u8 innerConnectorRow=1;innerConnectorRow<maxX+1;innerConnectorRow+=2)//for(u8 innerConnectorRow=startX-xOddAdder;innerConnectorRow<endX+endXadder;innerConnectorRow+=2)
    {
        for(u8 innerConnectorColumn=3;innerConnectorColumn<maxY+1;innerConnectorColumn+=2)
        {
            if(player->board[innerConnectorRow][innerConnectorColumn]!=0 || player->board[innerConnectorRow+1][innerConnectorColumn]!=0)
            {
                for (u8 section=0;section<=4;section+=4)
                {
                    for (u8 yDraw=0;yDraw<4;yDraw++)
                    {
                        if(section<=2)UpperHalfFlag=1;//upper half
                        else UpperHalfFlag=0;//lower half

                        if(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag]==1)leftside=allcolor1;
                        else if(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag]==2)leftside=allcolor2;
                        else if(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag]==3)leftside=allcolor3;
                        else if(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag]==4)leftside=allcolor4;
                        else if(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag]==5)leftside=allcolor5;
                        else if(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag]==6)leftside=allgarbage;
                        else leftside=allblank;

                        if(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]==1)rightside=allcolor1;
                        else if(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]==2)rightside=allcolor2;
                        else if(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]==3)rightside=allcolor3;
                        else if(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]==4)rightside=allcolor4;
                        else if(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]==5)rightside=allcolor5;
                        else if(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]==6)rightside=allgarbage;
                        //else if(innerConnectorRow+1>=maxX)rightside=allblank;
                        else rightside=allblank;

                        tile[yDraw+section]=(leftside<<16)+rightside;
                    }
                }
                
                VDP_loadTileData(tile, innerSectionsVRAM+tileIncrementer+vramOffsetP2, 1, CPU);//VDP_loadTileData (const u32 *data, u16 index, u16 num, TransferMethod tm)
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, innerSectionsVRAM+tileIncrementer+vramOffsetP2), xOffset+innerConnectorRow+(innerConnectorRow>>1)+1+p2offsetX, yOffset+innerConnectorColumn+(innerConnectorColumn>>1)-1, 1, 1);
            }
            tileIncrementer++;
        }
    }
}

void setSharedNext()
{
    for (u8 i=0;i<fallingPieceNumberOfTiles;i++)
    {
        sharedNext[i]=randomRange(1,(globalNumColors-1));
    }

    sharedNextStatus=1;
}

#define nextPieceYPos 4

void drawSharedNext()
{
    #define sharedNextxPos 20
    u8 sharedNextyPos=nextPieceYPos;
    u8 colorAdd=0;

    for (u8 i=0;i<fallingPieceNumberOfTiles;i++)
    {
        colorAdd=(sharedNext[i]-1)<<2;//multiply by 4
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), sharedNextxPos, sharedNextyPos, 1, 1);
        //VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), sharedNextxPos-1, sharedNextyPos, 1, 1);
        //VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), sharedNextxPos+1, sharedNextyPos, 1, 1);
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
/*
        if(player==&P1)
        {
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), playerNextxPos-1, playerNextyPos, 1, 1);
            //VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), playerNextxPos-2, playerNextyPos, 1, 1);
        }
        else if(player==&P2)
        {
            VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), playerNextxPos+1, playerNextyPos, 1, 1);
            //VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), playerNextxPos+2, playerNextyPos, 1, 1);
        }
*/
        playerNextyPos++;        
    }

    drawSharedNext();
}

void doRotate(Player* player, u8 direction)
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
    if(direction==LEFT)
    {
        if(player->board[player->xPosition-1][player->yPosition+1]!=0)return true;
        if(player->xPosition <= 1)return true;

        if(player->board[player->xPosition-1][player->yPosition]!=0 && player->yPosition==maxY)return true;//stop shifting at the bottom
    }

    if(direction==RIGHT)
    {
        if(player->board[player->xPosition+1][player->yPosition+1]!=0)return true;
        if(player->xPosition >= maxX)return true;
        
        if(player->board[player->xPosition+1][player->yPosition]!=0 && player->yPosition==maxY)return true;//stop shifting at the bottom
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

    player->flag_status=nothing;

    setSharedNext();
    drawPlayerNext(player);
}