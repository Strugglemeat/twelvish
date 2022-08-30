#include <genesis.h>
#include <resources.h>
#include <functions.h>

void printDebug();
void loadDebugFieldData();
void initialize();
void drawFallingSprite(Player* player);
void createPiece(Player* player);
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);
void handleInput(Player* player, u16 buttons);
bool collisionTest(Player* player, u8 direction);

int main()
{
    VDP_clearPlane(BG_B,TRUE);
    VDP_clearPlane(BG_A,TRUE);

    SYS_disableInts();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    loadTiles();

    VDP_setPalette(PAL3,fallingSingleAll.palette->data);
    
    VDP_setTextPalette(PAL1);

    VDP_drawImageEx(BG_B,&gridbg,0x57E,0,0,TRUE,FALSE);//font uses symbols we might never need

    SYS_enableInts();

    SPR_init();

    initialize();

    #define spriteXorigin 48+36
    #define spriteYorigin -12//0//12

    //loadDebugFieldData();
    //printBoard();

    while(1)
    {
        if(P1.moveDelay>0)P1.moveDelay--;
        if(P2.moveDelay>0)P2.moveDelay--;

        if(P1.fallDelay>0)P1.fallDelay--;
        if(P2.fallDelay>0)P2.fallDelay--;

        if(P1.flag_status==needPiece)createPiece(&P1);
        if(P2.flag_status==needPiece)createPiece(&P2);

        handleInput(&P1, JOY_readJoypad(JOY_1));
        handleInput(&P2, JOY_readJoypad(JOY_2));


        if(collisionTest(&P1, BOTTOM)==false){}
        else pieceIntoBoard(&P1);

        if(collisionTest(&P2, BOTTOM)==false){}
        else pieceIntoBoard(&P2);

        if(P1.board[4][3]!=0)P1.flag_status=toppedOut;
        if(P2.board[4][3]!=0)P2.flag_status=toppedOut;

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

    //sprintf(debug_string,"P1 %d", P1.flag_status);
    //VDP_drawText(debug_string,32,1);

    //sprintf(debug_string,"P1y:%d", P1.yPosition);
    //VDP_drawText(debug_string,2,4);

    //sprintf(debug_string,"P1x:%d", P1.xPosition);
    //VDP_drawText(debug_string,2,5);

    if(P1.flag_status==toppedOut)
    {
        sprintf(debug_string,"P1 topped out");
        VDP_drawText(debug_string,5,2);
    }
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

    P1.fallingPiece[0] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    P1.fallingPiece[1] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));

    P2.fallingPiece[0] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    P2.fallingPiece[1] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));

    P1.flag_status=needPiece;
    P2.flag_status=needPiece;

    P1.fallingIncrement=0;
    P2.fallingIncrement=0;
}

void drawFallingSprite(Player* player)
{
    SPR_setPosition(player->fallingPiece[0],player->spriteX,player->spriteY);
    SPR_setPosition(player->fallingPiece[1],player->spriteX,player->spriteY-TILESIZE);
}

void createPiece(Player* player)
{
    if(player==&P1)player->spriteX=spriteXorigin;
    else if(player==&P2)player->spriteX=spriteXorigin+(10*TILESIZE);
    player->spriteY=spriteYorigin+TILESIZE+TILESIZE+TILESIZE;

    for (u8 createIndex=0;createIndex<2;createIndex++)
    {
        player->newPiece[createIndex]=randomRange(1,player->numColors);//this is assigning color
        SPR_setFrame(player->fallingPiece[createIndex],player->newPiece[createIndex]-1);
    }

    player->xPosition=4;
    player->yPosition=1;
    player->moveDelay=0;

    player->flag_status=redraw;
}

void manageFalling(Player* player)
{
    if(player->spriteY>0)//because it starts high up on the screen and we don't want to use -1 for spriteY
    {
        player->fallingIncrement++;

        if(player->fallingIncrement==TILESIZE)
        {
            player->yPosition++;
            player->fallingIncrement=0;
        }
    }

    player->spriteY++;
}

void pieceIntoBoard(Player* player)
{
    player->board[player->xPosition][player->yPosition]=player->newPiece[0];//where are the colors stored? need new vals probly
    //also need the second piece, we need to account for rotation position when we lock it in
    player->board[player->xPosition][player->yPosition-1]=player->newPiece[1];

    player->flag_status=needPiece;
}

bool collisionTest(Player* player, u8 direction)
{
    if(direction==LEFT)
    {
        if(player->board[player->xPosition-1][player->yPosition+1]!=0)return true;
        if(player->xPosition <= 1)return true;
    }

    if(direction==RIGHT)
    {
        if(player->board[player->xPosition+1][player->yPosition+1]!=0)return true;
        if(player->xPosition >= maxX)return true;
    }
    
    if(direction==BOTTOM)
    {
        if(player->yPosition >= maxY) return true;
        if(player->board[player->xPosition][player->yPosition+1]!=0) return true;
    }

    return false;
}

void handleInput(Player* player, u16 buttons)
{
    if(buttons & BUTTON_LEFT && player->moveDelay==0 && collisionTest(player, LEFT)==FALSE)
    {
        player->xPosition--;
        player->moveDelay=MOVE_DELAY_AMOUNT;
        player->spriteX-=TILESIZE;
    }
    else if(buttons & BUTTON_RIGHT && player->moveDelay==0 && collisionTest(player, RIGHT)==FALSE)
    {
        player->xPosition++;
        player->moveDelay=MOVE_DELAY_AMOUNT;
        player->spriteX+=TILESIZE;
    }

    if (buttons & BUTTON_DOWN && player->fallDelay==0 && collisionTest(player, BOTTOM)==FALSE)
    {
        //player->fallingIncrement++;
        player->yPosition++;
        player->fallDelay=FALL_DELAY_AMOUNT;
        player->spriteY+=TILESIZE;
    }
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

    s8 drawPosX,drawPosY;
    u8 i;

//updown
    for (u8 updownX=1;updownX<maxX+1;updownX++)
    {
        for (u8 updownY=2;updownY<maxY;updownY+=2)
        {
            if(P1.board[updownX][updownY]!=0 || P1.board[updownX][updownY+1]!=0)
            {
                i=0;

                P1.updown[updownX][i]=(P1.board[updownX][updownY]<<4)+P1.board[updownX][updownY+1];

                //sprintf(debug_string,"UD:%d", P1.updown[updownX][i]);
                //VDP_drawText(debug_string,34,1+yDrawAdd);

                drawPosX=xOffset+updownX+((updownX)>>1);
                drawPosY=yOffset+updownY+((updownY)>>1);

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

//leftright 
    for (u8 leftrightY=1;leftrightY<maxY+1;leftrightY++)
    {
        for (u8 leftrightX=1;leftrightX<maxX;leftrightX+=2)
        {
            if(P1.board[leftrightX][leftrightY]!=0 || P1.board[leftrightX+1][leftrightY]!=0)
            {
                i=0;

                P1.leftright[i][leftrightY]=(P1.board[leftrightX][leftrightY]<<4)+P1.board[leftrightX+1][leftrightY];
                //now we have the color of the left cell in the left half of this byte and the right color in the right half of this byte

                //sprintf(debug_string,"LR:%d", P1.leftright[i][leftrightY]);
                //VDP_drawText(debug_string,34,1+xDrawAdd);

                drawPosX=xOffset+leftrightX+((leftrightX)>>1)+1;
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

//dynamic inner section vram load + draw

    #define allred 0x4444
    #define allyel 0x5555
    #define allgrn 0x6666
    #define allblu 0x7777
    #define allprp 0x8888
    #define allgbg 0x9999
    #define allblank 0x0000

    u32 tile[8];
    u16 leftside,rightside;
    u8 tileIncrementer=0;
    u8 sectionAdd;

    for(u8 innerConnectorRow=1;innerConnectorRow<maxX;innerConnectorRow+=2)
    {
        for(u8 innerConnectorColumn=3;innerConnectorColumn<maxY+1;innerConnectorColumn+=2)
        {
            if(P1.board[innerConnectorRow][innerConnectorColumn]!=0 || P1.board[innerConnectorRow+1][innerConnectorColumn]!=0)
            {
                for (u8 section=0;section<=4;section+=4)
                {
                    for (u8 yDraw=0;yDraw<4;yDraw++)
                    {
                        if(section==0)sectionAdd=1;//upper half
                        else sectionAdd=0;//lower half

                        if(P1.board[innerConnectorRow][innerConnectorColumn-sectionAdd]==1)leftside=allred;
                        else if(P1.board[innerConnectorRow][innerConnectorColumn-sectionAdd]==2)leftside=allyel;
                        else if(P1.board[innerConnectorRow][innerConnectorColumn-sectionAdd]==3)leftside=allgrn;
                        else if(P1.board[innerConnectorRow][innerConnectorColumn-sectionAdd]==4)leftside=allblu;
                        else if(P1.board[innerConnectorRow][innerConnectorColumn-sectionAdd]==5)leftside=allprp;
                        else if(P1.board[innerConnectorRow][innerConnectorColumn-sectionAdd]==6)leftside=allgbg;
                        else leftside=allblank;

                        if(P1.board[innerConnectorRow+1][innerConnectorColumn-sectionAdd]==1)rightside=allred;
                        else if(P1.board[innerConnectorRow+1][innerConnectorColumn-sectionAdd]==2)rightside=allyel;
                        else if(P1.board[innerConnectorRow+1][innerConnectorColumn-sectionAdd]==3)rightside=allgrn;
                        else if(P1.board[innerConnectorRow+1][innerConnectorColumn-sectionAdd]==4)rightside=allblu;
                        else if(P1.board[innerConnectorRow+1][innerConnectorColumn-sectionAdd]==5)rightside=allprp;
                        else if(P1.board[innerConnectorRow+1][innerConnectorColumn-sectionAdd]==6)rightside=allgbg;
                        else rightside=allblank;

                        tile[yDraw+section]=(leftside<<16)+rightside;
                    }
                }
                
                VDP_loadTileData(tile, innerSectionsVRAM+tileIncrementer, 1, CPU);//VDP_loadTileData (const u32 *data, u16 index, u16 num, TransferMethod tm)
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, innerSectionsVRAM+tileIncrementer), xOffset+innerConnectorRow+(innerConnectorRow>>1)+1, yOffset+innerConnectorColumn+(innerConnectorColumn>>1)-1, 1, 1);
            }
            tileIncrementer++;
        }
    }
}