#include <genesis.h>
#include <resources.h>
#include <functions.h>

void printDebug();
void createPiece(Player* player);
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);
void handleInput(Player* player, u16 buttons);
bool collisionTest(Player* player, u8 direction);
void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY);

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

    //loadDebugFieldData();
    //printBoard(&P1, 1,1,maxX+1,maxY+1);

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
            printBoard(&P1, P1.drawStartX,P1.drawStartY,P1.drawEndX,P1.drawEndY);
            P1.flag_status=nothing;
        }

        if(P2.flag_status==redraw)
        {
            printBoard(&P2, P2.drawStartX,P2.drawStartY,P2.drawEndX,P2.drawEndY);
            P2.flag_status=nothing;
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
    //VDP_drawText(debug_string,1,2);

    if(P1.flag_status==toppedOut)
    {
        sprintf(debug_string,"P1 topped out");
        VDP_drawText(debug_string,1,2);
    }

    //sprintf(debug_string,"P2:%d", P2.flag_status);
    //VDP_drawText(debug_string,25,5);
}

void createPiece(Player* player)
{
    if(player==&P1)player->spriteX=spriteXorigin;
    else if(player==&P2)player->spriteX=spriteXorigin+p2spriteXcreate;
    player->spriteY=spriteYorigin+TILESIZE+TILESIZE+TILESIZE;

    for (u8 createIndex=0;createIndex<3;createIndex++)
    {
        player->newPiece[createIndex]=randomRange(1,player->numColors);//this is assigning color
        SPR_setFrame(player->fallingPiece[createIndex],player->newPiece[createIndex]-1);
    }

    player->xPosition=4;
    player->yPosition=ySpawn;
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
        //player->fallingIncrement+=2;
        //player->spriteY+=2;

        player->yPosition++;
        player->spriteY+=TILESIZE;

        player->fallDelay=FALL_DELAY_AMOUNT;
    }
}

void pieceIntoBoard(Player* player)
{
    player->board[player->xPosition][player->yPosition]=player->newPiece[0];
    player->board[player->xPosition][player->yPosition-1]=player->newPiece[1];
    player->board[player->xPosition][player->yPosition-2]=player->newPiece[2];

    SPR_setVisibility(player->fallingPiece[0],HIDDEN);
    SPR_setVisibility(player->fallingPiece[1],HIDDEN);
    SPR_setVisibility(player->fallingPiece[2],HIDDEN);

    player->drawStartX=player->xPosition;
    player->drawStartY=player->yPosition-2;
    player->drawEndX=player->xPosition+1;
    player->drawEndY=player->yPosition+1;

    player->flag_status=needPiece;
}

void drawFallingSprite(Player* player)
{
    SPR_setPosition(player->fallingPiece[0],player->spriteX,player->spriteY);
    SPR_setPosition(player->fallingPiece[1],player->spriteX,player->spriteY-TILESIZE);
    SPR_setPosition(player->fallingPiece[2],player->spriteX,player->spriteY-TILESIZE-TILESIZE);

    if(player->yPosition==1)SPR_setVisibility(player->fallingPiece[0],VISIBLE);
    if(player->yPosition==2)SPR_setVisibility(player->fallingPiece[1],VISIBLE);
    if(player->yPosition==3)SPR_setVisibility(player->fallingPiece[2],VISIBLE);
}

void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY)//from left to right, from top to bottom
{  
    if(startY<2)startY=2;//no need to draw tiles above the barrier area

    for(u8 xDraw=startX;xDraw<endX;xDraw++)
    {
        for(u8 yDraw=startY;yDraw<endY;yDraw++)
        {
           if(player->board[xDraw][yDraw]!=0)drawTile(player, xDraw,yDraw);
        }
    }

    u8 p2offsetX=0;
    if(player==&P2)p2offsetX=player2offset;

    s8 drawPosX,drawPosY;
    u8 i;

    u8 yOddAdder=0;
    if((startY&1)!=0)yOddAdder=1;

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

    u8 xOddAdder=0;
    if((startX&1)==0)xOddAdder=1;

    u8 endXadder=0;
    if(endX<=maxX)endXadder=1;

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