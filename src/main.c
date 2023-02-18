#include <genesis.h>
#include <resources.h>
#include <functions.h>
#include <kdebug.h>

//PAL0
//PAL1
//PAL2
//PAL3 - transparent (1), pieces (6), 9 free

int main()
{
    VDP_clearPlane(BG_B,TRUE);
    VDP_clearPlane(BG_A,TRUE);

    SYS_disableInts();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    loadTiles();

    setRandomSeed(GET_HVCOUNTER*GET_VCOUNTER*GET_HCOUNTER);

    VDP_drawImageEx(BG_B,&gridbg,0x58d,0,0,TRUE,TRUE);//0x57E,58c
    PAL_setPalette(PAL0,cloud.palette->data,DMA);
    PAL_setColor(0,RGB8_8_8_TO_VDPCOLOR(255,255,255));

    VDP_setTextPalette(1);

    SYS_enableInts();

    startupOptionsMenu();

    VDP_setTextPalette(0);
    PAL_setColor(1,RGB8_8_8_TO_VDPCOLOR(1,1,1));//grid outline
    PAL_setColor(2,RGB8_8_8_TO_VDPCOLOR(96,96,85));//tiles at the bottom
    PAL_setColor(3,RGB8_8_8_TO_VDPCOLOR(randomRange(49,100),randomRange(49,100),randomRange(80,130)));//49,75,122//background
    PAL_setColor(4,RGB8_8_8_TO_VDPCOLOR(172,199,227));//inside the fields

    SPR_init();

    initialize();

    setSharedNext();
    drawPlayerNext(&P1);
    drawPlayerNext(&P2);

    //loadDebugFieldData();

    while(P1.flag_status!=toppedOut && P2.flag_status!=toppedOut)
    //while(P1.flag_status!=toppedOut)
    {
        manageDelays();

        gameLogicSwitch(&P1);
        gameLogicSwitch(&P2);

        if(P2.AIplayer==true)processAI();

        SYS_doVBlankProcess();

        drawCombosAndChains(&P1);//this needs to be restricted
        drawCombosAndChains(&P2);//this needs to be restricted

        if(P1.chainAmount>1 && P1.flag_status<=fallingPiece)
        {
            SPR_setVisibility(patrako_cheer,VISIBLE);
            SPR_setVisibility(patrako_idle,HIDDEN);
            SPR_setFrame(patrako_cheer,0);
            patrako_is_cheering=true;
            getTimer(33,true);//start a timer for this
        }

        if(P1.flag_status==fallingPiece)drawFallingSprite(&P1);//only draw if we're falling
        if(P2.flag_status==fallingPiece)drawFallingSprite(&P2);//only draw if we're falling
        
        if(P1.flag_drawNext==true){drawPlayerNext(&P1);P1.flag_drawNext=false;}
        if(P2.flag_drawNext==true){drawPlayerNext(&P2);P2.flag_drawNext=false;}

        if(P1.flag_redraw==true){
            //printBoard(&P1, 1,1,maxX+1,maxY+2);
            printBoard(&P1, P1.drawStartX,P1.drawStartY,P1.drawEndX,P1.drawEndY);
            P1.flag_redraw=false;
        }

        if(P2.flag_redraw==true){
            printBoard(&P2, P2.drawStartX,P2.drawStartY,P2.drawEndX,P2.drawEndY);
            P2.flag_redraw=false;
        }

        if(sharedNextStatus==1)drawSharedNext();

        if(globalSpawnCloudVisibilityTimer>20)SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);

        if(getTimer(33,false)>=64000 && patrako_is_cheering==true)
        {
            SPR_setFrame(patrako_idle,0);
            SPR_setVisibility(patrako_cheer,HIDDEN);
            SPR_setVisibility(patrako_idle,VISIBLE);
            patrako_is_cheering=false;
        }
        
        SPR_update();

        printDebug();
    }

    gameOver();

    return 0;
}

void blinkMatches(Player* player)
{
    #define blinkingTimeAmt 6000
    #define blinkNumOfTimes 8

    //KLog("$^^blinkMatches!!!");
    if(player->blinkTimes==0)//initialization
    {
        KLog("^^blinkmatches set draw parameters to FULL BOARD");
        player->drawStartX=maxX;
        player->drawEndX=1;
        player->drawStartY=maxY;
        player->drawEndY=1;

//start the timer
        getTimer(player->blinkTimerNum,true);

        for(u8 i=0;i<player->howManyMatched;i++)//don't iterate through the entire board, only the destruction queue pieces
        {
//update the drawing boundaries
            if(player->matchedQueueX[i]<player->drawStartX)
                {
                    player->drawStartX=player->matchedQueueX[i];
                    KLog_U1("^^drawStartX updated to: ",player->drawStartX);
                }
            if(player->matchedQueueX[i]>player->drawEndX)
                {
                    player->drawEndX=player->matchedQueueX[i];
                    KLog_U1("^^drawEndX updated to: ",player->drawEndX);
                }
            if(player->matchedQueueY[i]<player->drawStartY)
                {
                    player->drawStartY=player->matchedQueueY[i];
                    KLog_U1("^^drawStartY updated to: ",player->drawStartY);
                }
            if(player->matchedQueueY[i]>player->drawEndY)
                {
                    player->drawEndY=player->matchedQueueY[i];
                    KLog_U1("^^drawEndY updated to: ",player->drawEndY);
                }
            
//save the cleared pieces to blinkingSave array
                player->blinkingSave[player->matchedQueueX[i]][player->matchedQueueY[i]]=player->board[player->matchedQueueX[i]][player->matchedQueueY[i]];
        }
    
        player->drawStartX--;KLog_U2("manual update of drawStartX from ",player->drawStartX+1," to ",player->drawStartX);
        player->drawEndX++;KLog_U2("manual update of drawEndX from ",player->drawEndX-1," to ",player->drawEndX);
        player->drawStartY--;KLog_U2("manual update of drawStartY from ",player->drawStartY+1," to ",player->drawStartY);
        player->drawEndY++;KLog_U2("manual update of drawEndY from ",player->drawEndY-1," to ",player->drawEndY);
        
        player->blinkTimes++;
    }

    if(getTimer(player->blinkTimerNum,false)>=blinkingTimeAmt && player->blinkTimes<blinkNumOfTimes)
    {
        player->blinkTimes++;//increment the blinking number
        getTimer(player->blinkTimerNum,true);//reset the timer

        if((player->blinkTimes & 1) == 0)//if blinktimes is even
        {
            for(u8 i=0;i<player->howManyMatched;i++)
            {
                player->board[player->matchedQueueX[i]][player->matchedQueueY[i]]=player->blinkingSave[player->matchedQueueX[i]][player->matchedQueueY[i]];
            }
        }
        else if((player->blinkTimes & 1) != 0)//if blinktimes is odd
        {
            for(u8 i=0;i<player->howManyMatched;i++)
            {
                player->board[player->matchedQueueX[i]][player->matchedQueueY[i]]=0;
            }    
        }

        KLog_U4("^^blink drawing ",player->drawStartX,",",player->drawEndX," | ",player->drawStartY,",",player->drawEndY);
        player->flag_redraw=true;
        return;
    }
    else if(player->blinkTimes>=blinkNumOfTimes)
        {
//restore the pieces so they can be properly destroyed
            for(u8 i=0;i<player->howManyMatched;i++)
            {
                player->board[player->matchedQueueX[i]][player->matchedQueueY[i]]=player->blinkingSave[player->matchedQueueX[i]][player->matchedQueueY[i]];
//reset blinkingSave array (necessary?)
                player->blinkingSave[player->matchedQueueX[i]][player->matchedQueueY[i]]=0;
            }

            player->flag_redraw=true;

            player->flag_status=destroyingMatches;
            player->blinkTimes=0;
        }
}

void processDestroy(Player* player)//we ONLY get here if we are destroying tiles
{
    KLog("processDestroy just started");
    player->chainAmount++;

    for (u8 i=0;i<player->howManyMatched;i++)
    {
//check the surrounding for garbage to be transformed
        if(player->board[player->matchedQueueX[i]+1][player->matchedQueueY[i]]==COLOR_GARBAGE){
            player->board[player->matchedQueueX[i]+1][player->matchedQueueY[i]]=player->board[player->matchedQueueX[i]][player->matchedQueueY[i]];
            player->flag_status=checkingMatches;
        }
        if(player->board[player->matchedQueueX[i]][player->matchedQueueY[i]+1]==COLOR_GARBAGE){
            player->board[player->matchedQueueX[i]][player->matchedQueueY[i]+1]=player->board[player->matchedQueueX[i]][player->matchedQueueY[i]];
            player->flag_status=checkingMatches;
        }
        if(player->board[player->matchedQueueX[i]-1][player->matchedQueueY[i]]==COLOR_GARBAGE){
            player->board[player->matchedQueueX[i]-1][player->matchedQueueY[i]]=player->board[player->matchedQueueX[i]][player->matchedQueueY[i]];
            player->flag_status=checkingMatches;
        }
        if(player->board[player->matchedQueueX[i]][player->matchedQueueY[i]-1]==COLOR_GARBAGE){
            player->board[player->matchedQueueX[i]][player->matchedQueueY[i]-1]=player->board[player->matchedQueueX[i]][player->matchedQueueY[i]];
            player->flag_status=checkingMatches;
        }

        player->board[player->matchedQueueX[i]][player->matchedQueueY[i]]=0;
    }

    player->flag_redraw=true; //redrawing based on dimensions set in blinking - NOT reset here
    player->flag_status=doingGravity;
}

void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY)//from left to right, from top to bottom
{
    KLog_U4("printBoard: from X ",startX," to X ",endX,"    from Y ",startY," to Y ",endY);

    if(startX==0)
    {
       startX=1;
       KLog("printBoard: fixed StartX at zero");//OPTIMIZE - at source, don't allow 0
    }
        
    if(startY==0)
    {
        startY=1;
        KLog("printBoard: fixed StartY at zero");//OPTIMIZE - at source, don't allow 0
    }

    for(u8 xDraw=startX;xDraw<endX;xDraw++)
    {
        for(u8 yDraw=startY;yDraw<endY;yDraw++)
            {
                drawFullTile(player, xDraw,yDraw);
            }
    }

    u8 p2offsetX=0;
    if(player==&P2)p2offsetX=PLAYER2OFFSET;

    s8 drawPosX,drawPosY;

//updown
u8 updownYstart;
if((startY &1) == 0)updownYstart=startY;
else updownYstart=startY-1;

    for (u8 updownX=startX;updownX<endX;updownX++)
    {
        for (u8 updownY=updownYstart;updownY<maxY;updownY+=2)
        {
            if(player->board[updownX][updownY]!=0 || player->board[updownX][updownY+1]!=0)
            {
                player->updown[updownX]=(player->board[updownX][updownY]<<4)+player->board[updownX][updownY+1];

                //KLog_U1("UD:",P1.updown[updownX]);

                drawPosX=xOffset+updownX+((updownX)>>1)+p2offsetX;
                drawPosY=yOffset+updownY+((updownY)>>1);

                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, updownLUTflag(player->updown[updownX]), FALSE, updownLUT(player->updown[updownX])), drawPosX,drawPosY, 1, 1);
            }
        }
    }

//leftright
u8 leftrightXstart;
if((startX & 1) != 0)leftrightXstart=startX;
else leftrightXstart=startX-1;

    for (u8 leftrightY=startY;leftrightY<endY;leftrightY++)
    {
        for (u8 leftrightX=leftrightXstart;leftrightX<maxX;leftrightX+=2)
        {
            if(player->board[leftrightX][leftrightY]!=0 || player->board[leftrightX+1][leftrightY]!=0)
            {
                //KLog_U2("leftright at X: ",leftrightX," , Y: ",leftrightY);

                player->leftright[leftrightY]=(player->board[leftrightX][leftrightY]<<4)+player->board[leftrightX+1][leftrightY];
                //now we have the color of the left cell in the left half of this byte and the right color in the right half of this byte

                //KLog_U1("LR:",P1.leftright[leftrightY]);

                drawPosX=xOffset+leftrightX+((leftrightX)>>1)+1+p2offsetX;
                drawPosY=yOffset+leftrightY+((leftrightY-1)>>1);

                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, leftrightLUTflag(player->leftright[leftrightY]), leftrightLUT(player->leftright[leftrightY])), drawPosX,drawPosY, 1, 1);
            }
        }
    }

//dynamic inner section vram load + draw
//28 4-way tiles per side (56 total)
//currently we are calculating and drawing all 28 every time each person draws anywhere on the field

    u32 tile[8];

    u8 tileIncrementer=0;
    u8 UpperHalfFlag;

    u8 vramOffsetP2=0;
    if(player==&P2)vramOffsetP2=32;

    for(u8 innerConnectorRow=1;innerConnectorRow<maxX+1;innerConnectorRow+=2)//OPTIMIZE - RESTRICT START AND END
    //for(u8 innerConnectorRow=startX;innerConnectorRow<endX;innerConnectorRow+=2)
    {
        for(u8 innerConnectorColumn=3;innerConnectorColumn<maxY+1;innerConnectorColumn+=2)//OPTIMIZE - RESTRICT START AND END
        //for(u8 innerConnectorColumn=startY;innerConnectorColumn<endY;innerConnectorColumn+=2)
        {
            if(player->board[innerConnectorRow][innerConnectorColumn]!=0 || player->board[innerConnectorRow+1][innerConnectorColumn]!=0)
            {
                for (u8 section=0;section<=4;section+=4)
                {
                    for (u8 yDraw=0;yDraw<4;yDraw++)
                    {
                        if(section<=2)UpperHalfFlag=1;//upper half
                        else UpperHalfFlag=0;//lower half

                        tile[yDraw+section]=(innerConnectorLUT(player->board[innerConnectorRow][innerConnectorColumn-UpperHalfFlag])<<16)+innerConnectorLUT(player->board[innerConnectorRow+1][innerConnectorColumn-UpperHalfFlag]);
                    }
                }
                
                VDP_loadTileData(tile, innerSectionsVRAM+tileIncrementer+vramOffsetP2, 1, CPU);//VDP_loadTileData (const u32 *data, u16 index, u16 num, TransferMethod tm)
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, innerSectionsVRAM+tileIncrementer+vramOffsetP2), xOffset+innerConnectorRow+(innerConnectorRow>>1)+1+p2offsetX, yOffset+innerConnectorColumn+(innerConnectorColumn>>1)-1, 1, 1);
            }
            tileIncrementer++;
        }
    }
}

void drawFullTile(Player* player, u8 xPos, u8 yPos)//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
{
    u8 colorAdd=0;//4 tiles for each color. so color 2 is adding 4, color 3 is adding 8
    bool flag_erase=false;

    //if we are drawing a tile other than color 1, we need to increase the tile index    
    if(player->board[xPos][yPos]==0)flag_erase=true;
    else if(player->board[xPos][yPos]>1)colorAdd=(player->board[xPos][yPos]-1)<<2;//multiply by 4

    u8 drawingxPos;
    if(player==&P1)drawingxPos=xPos+(xPos>>1);
    else if(player==&P2)drawingxPos=xPos+(xPos>>1)+PLAYER2OFFSET;
    u8 drawingyPos=yPos+(yPos>>1);

    if(flag_erase==false)
    {
        //KLog_U2("drawFullTile: drew at X: ",xPos," Y: ",yPos);
        if((yPos & 1) != 0)
        {
            if((xPos & 1) != 0){//odd column, odd row (1,1)
                //top left: bottom half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
                //top right: bottom left corner
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 4+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
                //bottom left: full square
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
                //bottom right: left half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);
                
                return;
            }
            else if((xPos & 1) == 0){//even column, odd row (2,1)
                //top left: bottom right corner
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 4+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
                //top right: bottom half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
                //bottom left: right half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);
                //bottom right: full square
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
            
                return;
            }
        }
        else if((yPos & 1) == 0)
        {
            if((xPos & 1) != 0){//odd column, even row (1,2)
                //top left: full square
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
                //top right: left half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
                //bottom left: top half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
                //bottom right: top left corner
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, TRUE, 4+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);//was FALSE,TRUE,5
            
                return;
            }
            else if((xPos & 1) == 0){//even column, even row (2,2)
                //top left: right half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
                //top right: full square
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
                //bottom left: top right corner
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 4+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);//was FALSE,FALSE,5
                //bottom right: top half
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
            
                return;
            }
        }
    }
    else if(flag_erase==true)
    {
        //KLog_U2("drawFullTile: erased at X: ",xPos," Y: ",yPos);
        if((yPos & 1) != 0)
        {
            if((xPos & 1) != 0){//odd column, odd row (1,1)
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);//top left: bottom half
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);//top right: bottom left corner
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);//bottom left: full square
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);//bottom right: left half
                return;
            }
            else if((xPos & 1) == 0){//even column, odd row (2,1)
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);//top left: bottom right corner
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);//top right: bottom half
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);//bottom left: right half
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);//bottom right: full square
                return;
            }
        }
        else if((yPos & 1) == 0)
        {
            if((xPos & 1) != 0){//odd column, even row (1,2)
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);//top left: full square
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);//top right: left half
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);//bottom left: top half
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);//bottom right: top left corner
                return;
            }
            else if((xPos & 1) == 0){//even column, even row (2,2)
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);//top left: right half
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);//top right: full square
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);//bottom left: top right corner
                VDP_fillTileMapRect(BG_A, 0, drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);//bottom right: top half
                return;
            }
        }        
    }
}