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

    PAL_setPalette(PAL3,fallingSingleAll.palette->data,DMA);//VDP_setPalette(PAL3,fallingSingleAll.palette->data);
    VDP_setTextPalette(PAL3);

    VDP_drawImageEx(BG_B,&gridbg,0x57E,0,0,TRUE,TRUE);//font uses symbols we might never need, wasteful

    SYS_enableInts();

    SPR_init();

    initialize();

    setSharedNext();
    drawPlayerNext(&P1);
    drawPlayerNext(&P2);

    //loadDebugFieldData();

    //while(P1.flag_status!=toppedOut && P2.flag_status!=toppedOut)
    while(P1.flag_status!=toppedOut)
    {
        manageDelays();

        gameLogicSwitch(&P1);

        SYS_doVBlankProcess();

        if(P1.howManyDestroyed>3){
            sprintf(debug_string,"COMBO:%d",P1.howManyDestroyed);
            VDP_drawText(debug_string,2,1);
        }

        if(P1.chainAmount>1)
        {
            sprintf(debug_string,"CHAIN:%d",P1.chainAmount);
            VDP_drawText(debug_string,2,2);

            SPR_setVisibility(patrako_cheer,VISIBLE);
            SPR_setVisibility(patrako_idle,HIDDEN);
            SPR_setFrame(patrako_cheer,0);
            patrako_is_cheering=true;
            getTimer(33,true);//start a timer for this
        }

        if(P1.flag_status==fallingPiece)drawFallingSprite(&P1);//only draw if we're falling
        
        if(P1.flag_redraw==true){
            //printBoard(&P1, 1,1,maxX+1,maxY+2);
            printBoard(&P1, P1.drawStartX,P1.drawStartY,P1.drawEndX,P1.drawEndY);
            
            if(P1.flag_status==spawningPiece)drawPlayerNext(&P1);

            P1.flag_redraw=false;
        }

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

void processDestroy(Player* player)//we ONLY get here if we are destroying tiles
{
    KLog("processDestroy just started");
    player->chainAmount++;
    //u8 howManyDestroyed=0;

/*
    player->drawStartX-=1;
    player->drawEndX+=1;
    player->drawStartY-=1;
    player->drawEndY+=1;
*/

    for (u8 destroyX=1;destroyX<maxX+1;destroyX++)
    {
        for (u8 destroyY=1;destroyY<maxY+1;destroyY++)
        {
            if (player->boardDestructionQueue[destroyX][destroyY])
            {
//check the surrounding for garbage to be transformed
                if(player->board[destroyX+1][destroyY]==COLOR_GARBAGE){player->board[destroyX+1][destroyY]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                if(player->board[destroyX][destroyY+1]==COLOR_GARBAGE){player->board[destroyX][destroyY+1]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                if(player->board[destroyX-1][destroyY]==COLOR_GARBAGE){player->board[destroyX-1][destroyY]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                if(player->board[destroyX][destroyY-1]==COLOR_GARBAGE){player->board[destroyX][destroyY-1]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}

                player->board[destroyX][destroyY]=0;
                player->boardDestructionQueue[destroyX][destroyY]=false;
                player->howManyDestroyed++;

                if(destroyX<player->drawStartX){
                    player->drawStartX=destroyX;
                    KLog("^^processDestroy updated the draw parameters (drawStartX)");                    
                }
                else if(destroyX>player->drawEndX){
                    player->drawEndX=destroyX;
                    KLog("^^processDestroy updated the draw parameters (drawEndX)");
                }

                if(destroyY<player->drawStartY){
                    player->drawStartY=destroyY;
                    KLog("^^processDestroy updated the draw parameters (drawStartY)");
                }
                else if(destroyY>player->drawEndY){
                    player->drawEndY=destroyY;
                    KLog("^^processDestroy updated the draw parameters (drawEndY)");
                }
            }
        }
    }

    player->flag_status=doingGravity;

/*
    if(howManyDestroyed>3 && player==&P1)//NEEDS TO GO AFTER VBLANK
    {
        sprintf(debug_string,"COMBO:%d",howManyDestroyed);
        VDP_drawText(debug_string,2,1);
    }
*/
}

u8 blinkingSave[9][18];
#define blinkingTimeAmt 8000
#define blinkNumOfTimes 16

void blinkMatches(Player* player)
{
/*
    //u8 temp1,temp2,temp3,temp4;

    KLog("$^^blinkMatches!!!");
    if(player->blinkTimes==0)
    {
    
//save the drawParameters?
    //temp1=player->drawStartX;
    //temp2=player->drawStartY;
    //temp3=player->drawEndX;
    //temp4=player->drawEndY;
    
//start the timer
        getTimer(P1_blink_timer,true);

//save the color of the pieces beyond destroyed
        for(u8 i=1;i<maxX;i++)
        {
            for(u8 j=1;j<maxY;j++)
            {
                if(player->boardDestructionQueue[i][j]==true)blinkingSave[i][j]=player->board[i][j];
            }
        }

    player->blinkTimes++;
    }

    if(getTimer(P1_blink_timer,false)>=blinkingTimeAmt && player->blinkTimes<blinkNumOfTimes)
    {
        player->blinkTimes++;
        getTimer(P1_blink_timer,true);

        if((player->blinkTimes & 1) == 0)
        {
            for(u8 i=1;i<maxX;i++)
            {
                for(u8 j=1;j<maxY;j++)
                {
                    if(player->boardDestructionQueue[i][j]==true)player->board[i][j]=blinkingSave[i][j];
                }
            }     
        }
        else if((player->blinkTimes & 1) != 0)
        {
            for(u8 i=1;i<maxX;i++)
            {
                for(u8 j=1;j<maxY;j++)
                {
                    if(player->boardDestructionQueue[i][j]==true)player->board[i][j]=0;
                }
            }              
        }

        player->drawStartX=1;
        player->drawStartY=1;
        player->drawEndX=maxX;
        player->drawEndY=maxY+1;

        player->flag_redraw=true;
        return;
    }
    else if(player->blinkTimes>=blinkNumOfTimes)
        {
//restore the pieces
            for(u8 i=1;i<maxX;i++)
            {
                for(u8 j=1;j<maxY;j++)
                {
                    if(player->boardDestructionQueue[i][j]==true)player->board[i][j]=blinkingSave[i][j];
                }
            }     

//restore the draw parameters    
            //player->drawStartX=temp1;
            //player->drawStartY=temp2;
            //player->drawEndX=temp3;
            //player->drawEndY=temp4;

            player->flag_redraw=true;

            player->flag_status=destroyingMatches;
            player->blinkTimes=0;
        }
*/
    player->flag_status=destroyingMatches;
}


void printBoard(Player* player, u8 startX, u8 startY, u8 endX, u8 endY)//from left to right, from top to bottom
{
    //KLog_U4("X:[",startX,"]  Y:[",startY,"]  EX:[",endX,"]  EY:[",endY);
    KLog_U4("from X ",startX," to X ",endX,"    from Y ",startY," to Y ",endY);

    if(startY<1)//if(startY<2)
    {
        startY=1;//startY=2
        KLog("%%@@@we NEWreset startY in printboard!!!!!!!");
    }

    for(u8 xDraw=startX;xDraw<endX;xDraw++)
    {
        for(u8 yDraw=startY;yDraw<endY;yDraw++)
        {
           drawFullTile(player, xDraw,yDraw);
        }
    }

    u8 p2offsetX=0;
    if(player==&P2)p2offsetX=player2offset;

    s8 drawPosX,drawPosY;
    u8 i;

//updown
    for (u8 updownX=startX;updownX<endX;updownX++)//u8 updownX=1 starts on updownX at 1, what happens if it's 2?
    {
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

void drawFullTile(Player* player, u8 xPos, u8 yPos)//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
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

    if(!(flag_erase))
    {
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
    else if(flag_erase)
    {
        KLog_U2("erased at X: ",xPos," Y: ",yPos);
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