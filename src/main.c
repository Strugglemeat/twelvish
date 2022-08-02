#include <genesis.h>
#include <resources.h>

char debug_string[40] = "";

void printDebug();
void loadTiles();
void printBoard();
void loadDebugFieldData();
void drawTile(u8 xPos, u8 yPos);
void initialize();
u8 randomRange(u8 rangeStart, u8 rangeEnd);

//320/12 = 26.66667      320/8 = 40
//224/12 = 18.66667      224/8 = 28

typedef struct {

    u8 board[10][18];//8 wide by 16 tall. [0][0] not used.
    u8 flag_status;

    Sprite* fallingPiece[2];
    u16 spriteX;
    s16 spriteY;
    u8 xPosition,yPosition;
    u8 numColors;

    u8 leftright[4][16];
    u8 updown[7][8];
    u8 topbot[4][2];
    u8 innerconnect[4][7][2];

    u8 fallingIncrement;
    u8 rotation;//0=spawn. secondary piece above. 1=secondary piece on the right. 2=secondary piece below. 3=to the left
    u8 fallingPieceArea[3][3];//[0][0] is top left, [2][2] is bot right

} Player;

Player P1;
Player P2;

void clearBoard(Player* player);
void drawFallingSprite(Player* player);
void createPiece(Player* player);
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);

#define maxX 8
#define maxY 16

#define xOffset 5
#define yOffset 1

enum status
{
    nothing,
    redraw,
    needPiece,
    toppedOut
};

#define innerSectionsVRAM 22//56 tiles worth of VRAM (28 per player)

int main()
{
    VDP_clearPlane(BG_B,TRUE);
    VDP_clearPlane(BG_A,TRUE);

    SYS_disableInts();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    loadTiles();

    VDP_setPalette(PAL3,fullblock_color3.palette->data);
    VDP_setTextPalette(PAL1);

    VDP_drawImageEx(BG_B,&gridbg,0x57E,0,0,TRUE,FALSE);//font uses symbols we might never need

    SYS_enableInts();

    initialize();

    SPR_init();

    #define spriteXorigin 48+36
    #define spriteYorigin -12//0//12

    while(1)
    {
        if(P1.flag_status==needPiece)createPiece(&P1);
        if(P2.flag_status==needPiece)createPiece(&P2);

        if(P1.yPosition<=maxY && P1.board[P1.xPosition][P1.yPosition+1]==0)manageFalling(&P1);
        else if (P1.yPosition>maxY || P1.board[P1.xPosition][P1.yPosition+1]!=0)pieceIntoBoard(&P1);

        if(P2.yPosition<=maxY && P2.board[P2.xPosition][P2.yPosition+1]==0)manageFalling(&P2);
        else if (P2.yPosition>maxY || P2.board[P2.xPosition][P2.yPosition+1]!=0)pieceIntoBoard(&P2);

        if(P1.board[4][1]!=0)P1.flag_status=toppedOut;
        if(P2.board[4][1]!=0)P2.flag_status=toppedOut;

        SYS_doVBlankProcess();
        
        if(P1.flag_status==redraw)
        {
            printBoard();
            P1.flag_status=nothing;
        }

        drawFallingSprite(&P1);
        drawFallingSprite(&P2);

        SPR_update();

        printDebug();
    }
    
    return 0;
}

void printDebug()
{
    sprintf(debug_string,"%ld", SYS_getFPS());
    VDP_drawText(debug_string,0,0);

    //sprintf(debug_string,"P1Y%d", P1.yPosition);
    //VDP_drawText(debug_string,32,1);
}

void drawTile(u8 xPos, u8 yPos)//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
{
    u8 colorAdd=0;//4 tiles for each color. so color 2 is adding 4, color 3 is adding 8
    
    if(P1.board[xPos][yPos]>1)//if we are drawing a tile other than color 1, we need to increase the tile index
    {
        colorAdd=P1.board[xPos][yPos]-1;
        colorAdd=colorAdd<<2;//multiply by 4
    }

    u8 drawingxPos=xPos;
    u8 drawingyPos=yPos;
    drawingxPos+=xPos>>1;
    drawingyPos+=yPos>>1;

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

void loadTiles()
{
    VDP_loadFontData(tileset_Font_Namco.tiles, 96, CPU);
    
    VDP_loadTileSet(fullblock_color1.tileset,1,CPU);
    VDP_loadTileSet(fullblock_color2.tileset,1+4,CPU);
    VDP_loadTileSet(fullblock_color3.tileset,1+4+4,CPU);

    VDP_loadTileSet(topblock_color1.tileset,2,CPU);
    VDP_loadTileSet(topblock_color2.tileset,2+4,CPU);
    VDP_loadTileSet(topblock_color3.tileset,2+4+4,CPU);
    
    VDP_loadTileSet(rightblock_color1.tileset,3,CPU);
    VDP_loadTileSet(rightblock_color2.tileset,3+4,CPU);
    VDP_loadTileSet(rightblock_color3.tileset,3+4+4,CPU);

    VDP_loadTileSet(cornerblock_color1.tileset,4,CPU);
    VDP_loadTileSet(cornerblock_color2.tileset,4+4,CPU);
    VDP_loadTileSet(cornerblock_color3.tileset,4+4+4,CPU);

    VDP_loadTileSet(leftright12.tileset,13,CPU);//1,2
    VDP_loadTileSet(leftright13.tileset,14,CPU);//1,3
    VDP_loadTileSet(leftright23.tileset,15,CPU);//2,3

    VDP_loadTileSet(updown12.tileset,16,CPU);//1,2
    VDP_loadTileSet(updown13.tileset,17,CPU);//1,3
    VDP_loadTileSet(updown23.tileset,18,CPU);//2,3

    VDP_loadTileSet(topbot12.tileset,19,CPU);//1,2
    VDP_loadTileSet(topbot13.tileset,20,CPU);//1,3
    VDP_loadTileSet(topbot23.tileset,21,CPU);//2,3
}

void initialize()
{
    P1.numColors=3;
    P2.numColors=P1.numColors;

    //setRandomSeed(getTime(0));
    //waitMs(randomRange(1,8));

    clearBoard(&P1);
    clearBoard(&P2);

    //P1.flag_status=redraw;

    P1.flag_status=needPiece;
    P2.flag_status=needPiece;

    P1.fallingIncrement=0;
    P2.fallingIncrement=0;
}

u8 randomRange(u8 rangeStart, u8 rangeEnd)//general use function
{
    return (random() % (rangeEnd + 1 - rangeStart)) + rangeStart;
}

void printBoard()
{  
    for(u8 xDraw=1;xDraw<maxX+1;xDraw++)
    {
        for(u8 yDraw=1;yDraw<maxY+1;yDraw++)
        {
           if(P1.board[xDraw][yDraw]!=0)drawTile(xDraw,yDraw);
        }
    }

//leftright
    u8 xDrawAdd=0;
    s8 drawPosX,drawPosY;
    u8 i;
    
    for (u8 leftrightY=1;leftrightY<maxY+1;leftrightY++)
    {
        xDrawAdd=0;

        for (u8 leftrightX=1;leftrightX<maxX;leftrightX+=2)
        {
            if(P1.board[leftrightX][leftrightY]!=0 || P1.board[leftrightX+1][leftrightY]!=0)
            {
                xDrawAdd++;
                i=0;

                P1.leftright[i][leftrightY]=(P1.board[leftrightX][leftrightY]<<4)+P1.board[leftrightX+1][leftrightY];
                //now we have the color of the left cell in the left half of this byte and the right color in the right half of this byte

                //sprintf(debug_string,"LR:%d", P1.leftright[i][leftrightY]);
                //VDP_drawText(debug_string,34,1+xDrawAdd);

                drawPosX=xOffset+leftrightX+xDrawAdd;
                drawPosY=yOffset+leftrightY+((leftrightY-1)>>1);

                switch(P1.leftright[i][leftrightY])//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
                {
                    case 1://0,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3), drawPosX,drawPosY, 1, 1);
                    break;

                    case 2://0,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 3://0,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 16://1,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3), drawPosX, drawPosY, 1, 1);
                    break;

                    case 17://1,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1), drawPosX, drawPosY, 1, 1);
                    break;

                    case 18://1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 13), drawPosX, drawPosY, 1, 1);
                    break;

                    case 19://1,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 14), drawPosX, drawPosY, 1, 1);
                    break;

                    case 32://2,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 33://2,1 - flipped 1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 13), drawPosX, drawPosY, 1, 1);
                    break;

                    case 34://2,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 1+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 35://2,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 15), drawPosX, drawPosY, 1, 1);
                    break;

                    case 48://3,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 49://3,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 14), drawPosX, drawPosY, 1, 1);
                    break;

                    case 50://3,2 - a flipped 23
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 15), drawPosX, drawPosY, 1, 1);
                    break;

                    case 51://3,3 solid green
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+4+4), drawPosX, drawPosY, 1, 1);
                    break;
                }
            
                i++;
            }
        }
    }

//updown
    u8 yDrawAdd=0;
    drawPosX=0,drawPosY=0;

    for (u8 updownX=1;updownX<maxX+1;updownX++)
    {
        yDrawAdd=0;

        for (u8 updownY=2;updownY<maxY;updownY+=2)
        {
            if(P1.board[updownX][updownY]!=0 || P1.board[updownX][updownY+1]!=0)
            {
                yDrawAdd++;
                i=0;

                P1.updown[updownX][i]=(P1.board[updownX][updownY]<<4)+P1.board[updownX][updownY+1];

                //sprintf(debug_string,"UD:%d", P1.updown[updownX][i]);
                //VDP_drawText(debug_string,34,1+yDrawAdd);

                drawPosX=xOffset+updownX+((updownX)>>1);
                drawPosY=yOffset+updownY+yDrawAdd;

                switch(P1.updown[updownX][i])//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
                {
                    case 1://0,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2), drawPosX,drawPosY, 1, 1);
                    break;

                    case 2://0,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 3://0,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 16://1,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 17://1,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1), drawPosX, drawPosY, 1, 1);
                    break;

                    case 18://1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 16), drawPosX, drawPosY, 1, 1);
                    break;

                    case 19://1,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 17), drawPosX, drawPosY, 1, 1);
                    break;

                    case 32://2,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 33://2,1 - flipped 1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 16), drawPosX, drawPosY, 1, 1);
                    break;

                    case 34://2,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 35://2,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 18), drawPosX, drawPosY, 1, 1);
                    break;

                    case 48://3,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 49://3,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 17), drawPosX, drawPosY, 1, 1);
                    break;

                    case 50://3,2 - a flipped 23
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 18), drawPosX, drawPosY, 1, 1);
                    break;

                    case 51://3,3 solid green
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+4+4), drawPosX, drawPosY, 1, 1);
                    break;
                }
                
                i++;
            }
        }
    }

//top and bottom
    u8 topbotAdd=0,topbotAddInverse=1;

    for (u8 topbotY=1;topbotY<=maxY;topbotY+=maxY-1)//#define maxY 16
    {
        for (u8 topbotX=1;topbotX<maxX;topbotX+=2)
        {
            if(P1.board[topbotX][topbotY]!=0 || P1.board[topbotX+1][topbotY]!=0)
            {
                i=0;

                P1.topbot[i][topbotAdd]=(P1.board[topbotX][topbotY]<<4)+P1.board[topbotX+1][topbotY];//u8 topbot[4][2];

                //sprintf(debug_string,"TB:%d", P1.topbot[i][topbotAdd]);
                //VDP_drawText(debug_string,34,1+topbotX+topbotAdd);

                drawPosX=xOffset+topbotX+1+((topbotX-1)>>1);
                if(topbotAdd==0)drawPosY=yOffset;
                else if(topbotAdd==1)drawPosY=yOffset+maxY+8;

                switch(P1.topbot[i][topbotAdd])//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
                {
                    case 1://0,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAdd, FALSE, 4), drawPosX,drawPosY, 1, 1);
                    break;

                    case 2://0,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAdd, FALSE, 4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 3://0,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAdd, FALSE, 4+4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 16://1,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAdd, TRUE, 4), drawPosX,drawPosY, 1, 1);
                    break;

                    case 17://1,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, FALSE, 2), drawPosX, drawPosY, 1, 1);
                    break;

                    case 18://1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, FALSE, 19), drawPosX, drawPosY, 1, 1);
                    break;

                    case 19://1,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, FALSE, 20), drawPosX, drawPosY, 1, 1);
                    break;

                    case 32://2,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAdd, TRUE, 4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 33://2,1 - flipped 1,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, TRUE, 19), drawPosX, drawPosY, 1, 1);
                    break;

                    case 34://2,2
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, FALSE, 2+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 35://2,3
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, FALSE, 21), drawPosX, drawPosY, 1, 1);
                    break;

                    case 48://3,0
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAdd, TRUE, 4+4+4), drawPosX, drawPosY, 1, 1);
                    break;

                    case 49://3,1
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, TRUE, 20), drawPosX, drawPosY, 1, 1);
                    break;

                    case 50://3,2 - a flipped 23
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, TRUE, 21), drawPosX, drawPosY, 1, 1);
                    break;

                    case 51://3,3 solid green TOP HALF IS BLANK - OK
                    VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, topbotAddInverse, FALSE, 2+4+4), drawPosX, drawPosY, 1, 1);
                    break;
                }

                i++;
            }
        }
        topbotAdd++;
        topbotAddInverse--;
    }

//dynamic inner section vram load + draw

    #define allred 0x4444
    #define allyel 0x5555
    #define allgrn 0x6666

    u32 tile[8];
    u16 leftside,rightside;
    u8 tileIncrementer=0;

    for(u8 innerConnectorRow=1;innerConnectorRow<maxX;innerConnectorRow+=2)
    {
        for(u8 innerConnectorColumn=3;innerConnectorColumn<maxY;innerConnectorColumn+=2)
        {
            if(P1.board[innerConnectorRow][innerConnectorColumn]!=0 || P1.board[innerConnectorRow+1][innerConnectorColumn]!=0)
            {
                for (u8 section=0;section<=4;section+=4)
                {
                    for (u8 yDraw=0;yDraw<4;yDraw++)
                    {
                        if(section==0)//upper half
                        {
                            if(P1.board[innerConnectorRow][innerConnectorColumn-1]==1)leftside=allred;
                            else if(P1.board[innerConnectorRow][innerConnectorColumn-1]==2)leftside=allyel;
                            else if(P1.board[innerConnectorRow][innerConnectorColumn-1]==3)leftside=allgrn;

                            if(P1.board[innerConnectorRow+1][innerConnectorColumn-1]==1)rightside=allred;
                            else if(P1.board[innerConnectorRow+1][innerConnectorColumn-1]==2)rightside=allyel;
                            else if(P1.board[innerConnectorRow+1][innerConnectorColumn-1]==3)rightside=allgrn;
                        }
                        else if(section!=0)//lower half
                        {
                            if(P1.board[innerConnectorRow][innerConnectorColumn]==1)leftside=allred;
                            else if(P1.board[innerConnectorRow][innerConnectorColumn]==2)leftside=allyel;
                            else if(P1.board[innerConnectorRow][innerConnectorColumn]==3)leftside=allgrn;

                            if(P1.board[innerConnectorRow+1][innerConnectorColumn]==1)rightside=allred;
                            else if(P1.board[innerConnectorRow+1][innerConnectorColumn]==2)rightside=allyel;
                            else if(P1.board[innerConnectorRow+1][innerConnectorColumn]==3)rightside=allgrn;
                        }   

                        tile[yDraw+section]=(leftside<<16)+rightside;
                    }
                }

                VDP_loadTileData(tile, innerSectionsVRAM+tileIncrementer, 1, CPU);
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, innerSectionsVRAM+tileIncrementer), xOffset+1+innerConnectorRow+(innerConnectorRow>>1), yOffset+innerConnectorColumn+(innerConnectorColumn>>1)-1, 1, 1);
            }
            tileIncrementer++;
        }
    }
}

void loadDebugFieldData()
{
/*  
    for (u8 x=1;x<maxX+1;x++)
    {
        for (u8 y=1;y<maxY+1;y++)
        {
            P1.board[x][y]=randomRange(1,P1.numColors);
        }
    }
*/

    P1.board[1][maxY]=randomRange(1,P1.numColors);
    P1.board[2][maxY]=randomRange(1,P1.numColors);

    P1.board[4][maxY]=randomRange(1,P1.numColors);
    P1.board[5][maxY]=randomRange(1,P1.numColors);

    P1.board[maxX-1][maxY]=randomRange(1,P1.numColors);
    P1.board[maxX][maxY]=randomRange(1,P1.numColors);


}

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

void drawFallingSprite(Player* player)
{
    SPR_setPosition(player->fallingPiece[0],player->spriteX,player->spriteY);
    SPR_setPosition(player->fallingPiece[1],player->spriteX,player->spriteY-12);
}

void createPiece(Player* player)
{
    u8 newPiece[2];

    if(player==&P1)player->spriteX=spriteXorigin;
    else if(player==&P2)player->spriteX=spriteXorigin+(10*12);
    player->spriteY=spriteYorigin;

    for (u8 createIndex=0;createIndex<2;createIndex++)
    {
        newPiece[createIndex]=randomRange(1,player->numColors);
        switch(newPiece[createIndex])
        {
            case 1:
            player->fallingPiece[createIndex] = SPR_addSpriteSafe(&fallingSingle1, -12, -12, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
            break;
            
            case 2:
            player->fallingPiece[createIndex] = SPR_addSpriteSafe(&fallingSingle2, -12, -12, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
            break;

            case 3:
            player->fallingPiece[createIndex] = SPR_addSpriteSafe(&fallingSingle3, -12, -12, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
            break;
        }
        
    }
    
    if(player==&P1)player->xPosition=4;
    else if(player==&P2)player->xPosition=4+15;
    player->yPosition=0;

    player->flag_status=nothing;
}

void manageFalling(Player* player)
{
    if(player->spriteY>0)//because it starts high up on the screen and we don't want to use -1 for spriteY
    {
        player->fallingIncrement++;

        if(player->fallingIncrement==12)
        {
            player->yPosition++;
            player->fallingIncrement=0;
        }
    }

    player->spriteY++;
}

void pieceIntoBoard(Player* player)
{
    player->board[player->xPosition][player->yPosition]=1;//where are the colors stored? need new vals probly
    //also need the second piece, we need to account for rotation position when we lock it in
    player->flag_status=needPiece;
}