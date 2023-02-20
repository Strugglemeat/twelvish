#include <genesis.h>
#include <resources.h>
#include <functions.h>
#include <kdebug.h>

int main()
{
    VDP_clearPlane(BG_B,TRUE);
    VDP_clearPlane(BG_A,TRUE);

    SYS_disableInts();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    //VDP_setHilightShadow(1);

    loadTiles();

    setRandomSeed(GET_HVCOUNTER*GET_VCOUNTER*GET_HCOUNTER);

    VDP_drawImageEx(BG_B,&gridbg,TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, 0x58B),0,0,TRUE,TRUE);//0x58d
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

        manageDrawing(&P1);
        manageDrawing(&P2);

        if(flag_sharedNextStatus==true)drawSharedNext();

        if(globalSpawnCloudVisibilityTimer>20)SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);

        if(getTimer(33,false)>=62000 && patrako_is_cheering==true)
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

void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY)//from left to right, from top to bottom
{
    KLog_U4("printBoard: from X ",startX," to X ",endX,"    from Y ",startY," to Y ",endY);

    if(startX==0)
    {
       startX=1;
       KLog("printBoard: fixed StartX at zero");//OPTIMIZE - at source, don't permit 0
    }
        
    if(startY==0)
    {
        startY=1;
        KLog("printBoard: fixed StartY at zero");//OPTIMIZE - at source, don't permit 0
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

                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, TRUE, updownLUTflag(player->updown[updownX]), FALSE, updownLUT(player->updown[updownX])), drawPosX,drawPosY, 1, 1);
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
                player->leftright[leftrightY]=(player->board[leftrightX][leftrightY]<<4)+player->board[leftrightX+1][leftrightY];//now we have the color of the left cell in the left half of this byte and the right color in the right half of this byte
                
                //KLog_U1("LR:",P1.leftright[leftrightY]);
                drawPosX=xOffset+leftrightX+((leftrightX)>>1)+1+p2offsetX;
                drawPosY=yOffset+leftrightY+((leftrightY-1)>>1);

                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, TRUE, FALSE, leftrightLUTflag(player->leftright[leftrightY]), leftrightLUT(player->leftright[leftrightY])), drawPosX,drawPosY, 1, 1);
            }
        }
    }

//dynamic inner section vram load + draw
//32 4-way tiles per side (64 total)
//currently we are calculating and drawing all 32 every time each person draws anywhere on the field

    u32 tile[8];

    u8 tileIncrementer=0;
    u8 UpperHalfFlag;

    u8 vramOffsetP2=0;
    if(player==&P2)vramOffsetP2=32;

    s8 innerDrawingLowestY=startY;
    if((innerDrawingLowestY & 1) == 0)innerDrawingLowestY--;
    else innerDrawingLowestY-=2;
    if(innerDrawingLowestY<3)innerDrawingLowestY=3;

/*
//maxX is 7
//maxY is 17
    u8 innerDrawingHighestY=endY;
    if((innerDrawingHighestY & 1) == 0)innerDrawingHighestY++;
    else innerDrawingHighestY+=2;
    if(innerDrawingHighestY>maxY)innerDrawingHighestY=maxY;
    //we need to add to tileIncrementer here
    tileIncrementer+=maxY-endY;//innerDrawingHighestY;

    for(u8 innerConnectorColumn=innerDrawingHighestY;innerConnectorColumn>innerDrawingLowestY;innerConnectorColumn-=2)
*/
    //u8 skipAmountEndX=2;

    for(u8 innerConnectorColumn=maxY;innerConnectorColumn>=innerDrawingLowestY;innerConnectorColumn-=2)
    {
        for(u8 innerConnectorRow=1;innerConnectorRow<=maxX;innerConnectorRow+=2)
        //for(u8 innerConnectorRow=1;innerConnectorRow<=maxX-skipAmountEndX;innerConnectorRow+=2)
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
                VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, innerSectionsVRAM+tileIncrementer+vramOffsetP2), xOffset+innerConnectorRow+(innerConnectorRow>>1)+1+p2offsetX, yOffset+innerConnectorColumn+(innerConnectorColumn>>1)-1, 1, 1);
            }
            tileIncrementer++;
            //tileIncrementer+=skipAmountEndX;
        }
    }
    KLog_U1("tileIncrementer ended at ",tileIncrementer);//this is ending at 32 - shouldn't it be only 28?
}

void manageDrawing(Player* player)
{
    drawCombosAndChains(player);//this needs to be restricted

    if(player->flag_status==fallingPiece)drawFallingSprite(player);//only draw if we're falling

    if(player->flag_redraw==true){
        printBoard(player, player->drawStartX,player->drawStartY,player->drawEndX,player->drawEndY);//printBoard(&P1, 1,1,maxX+1,maxY+2);
        player->flag_redraw=false;
    }

    if(player->flag_drawNext==true){
        drawPlayerNext(player);
        player->flag_drawNext=false;
    }
}