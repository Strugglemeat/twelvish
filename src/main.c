#include <genesis.h>
#include <resources.h>
#include <functions.h>

void printDebug();
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);

void processDestroy(Player* player);
void processGravity(Player* player);
void manageDelays();
void sendDamage(Player* player, u8 amountDamageTaken);
void drawMeter();

void doRedraw(Player* player);
void doCollisionLocking(Player* player);

#define destroyDelay 36000
#define lockingDelay 24000

//PAL0
//PAL1
//PAL2
//PAL3 - falling pieces (6), trans (1), 10 free

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

    VDP_drawImageEx(BG_A,&monster,TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, endOfInnerSectionsVRAM),13,10,TRUE,TRUE);
    VDP_drawImageEx(BG_A,&monster2,TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, endOfInnerSectionsVRAM+80),21,10,TRUE,TRUE);

    SYS_enableInts();

    SPR_init();

    initialize();

    setSharedNext();
    drawPlayerNext(&P1);
    drawPlayerNext(&P2);

    //loadDebugFieldData();
    //printBoard(&P1, 1,1,maxX+1,maxY+1);

    //while(P1.flag_status!=toppedOut && P2.flag_status!=toppedOut)
    while(P1.flag_status!=toppedOut)
    {
        manageDelays();

        if(P1.flag_status==needPiece && P1.damageToBeReceived==0 && P1.flag_destroy==false && P1.flag_checkmatches==false)createPiece(&P1);
        else if(P1.flag_status!=needPiece && P1.flag_destroy==false && P1.flag_checkmatches==false)handleInput(&P1, JOY_readJoypad(JOY_1));
        
        if(P2.flag_status==needPiece && P2.damageToBeReceived==0 && P2.flag_destroy==false && P2.flag_checkmatches==false)createPiece(&P2);
        else if(P2.flag_status!=needPiece && P2.flag_destroy==false && P2.flag_checkmatches==false)handleInput(&P2, JOY_readJoypad(JOY_2));

        if(P1.flag_destroy==false && P1.flag_checkmatches==false)doCollisionLocking(&P1);
        if(P2.flag_destroy==false && P2.flag_checkmatches==false)doCollisionLocking(&P2);

        if(P1.damageToBeReceived>0 && P1.flag_status==needPiece && P1.flag_checkmatches==false && P1.flag_destroy==false)sendDamage(&P1, P1.damageToBeReceived);
        if(P2.damageToBeReceived>0 && P2.flag_status==needPiece && P2.flag_checkmatches==false && P2.flag_destroy==false)sendDamage(&P2, P2.damageToBeReceived);

        if(P1.flag_checkmatches==true)checkMatches(&P1);
        if(P2.flag_checkmatches==true)checkMatches(&P2);

        if(P1.flag_destroy==true && getTimer(P1destroyTimer,false)>destroyDelay)processDestroy(&P1);
        if(P2.flag_destroy==true && getTimer(P2destroyTimer,false)>destroyDelay)processDestroy(&P2);

        if(P1.flag_gravity==true)processGravity(&P1);
        if(P2.flag_gravity==true)processGravity(&P2);

        if(P1.board[4][topOutYpos]!=0 && P1.flag_destroy==false && P1.flag_checkmatches==false)P1.flag_status=toppedOut;
        if(P2.board[4][topOutYpos]!=0 && P2.flag_destroy==false && P2.flag_checkmatches==false)P2.flag_status=toppedOut;

        SYS_doVBlankProcess();
        
        if(P1.flag_redraw==true && P1.flag_status!=toppedOut)doRedraw(&P1);
        if(P2.flag_redraw==true && P2.flag_status!=toppedOut)doRedraw(&P2);

        if(P1.flag_status!=toppedOut)drawFallingSprite(&P1);
        if(P2.flag_status!=toppedOut)drawFallingSprite(&P2);

        if(globalSpawnCloudVisibilityTimer>20)SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);

        SPR_update();

        //drawMeter();

        printDebug();
    }

    //game over stuff goes here
    SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);
    SPR_setVisibility(P1.fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(P1.fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(P1.fallingPieceSprite[2],HIDDEN);
    SPR_setVisibility(P2.fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(P2.fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(P2.fallingPieceSprite[2],HIDDEN);
    SPR_update();
    
    return 0;
}

void printDebug()
{
    sprintf(debug_string,"%ldFPS", SYS_getFPS());
    VDP_drawText(debug_string,13,27);

    sprintf(debug_string,"%d", SYS_getCPULoad());
    strcat(debug_string, "%");
    strcat(debug_string, "CPU");
    VDP_drawText(debug_string,21,27);

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

    //VDP_clearTextBG(BG_A,16,8,12);//VDP_clearTextBG(VDPPlane plane, u16 x, u16 y, u16 w);
    //sprintf(debug_string,"P1:%lu", getTimer(P1destroyTimer,false));
    //VDP_drawText(debug_string,13,8);
}

void manageFalling(Player* player)
{
    if(player->flag_fastdrop==false)
    {
        player->fallingIncrement++;
        player->spriteY++;

        if(player->fallingIncrement>=TILESIZE)
        {
            player->yPosition++;
            player->fallingIncrement=0;
        }
    }
    else if(player->flag_fastdrop==true)
    {
        s8 i;//has to be outside of the for loop so it can be used afterwards
        for(i=player->yPosition;i<maxY;i++)
        {
            if(player->board[player->xPosition][i+1]!=0)break;
        }
        
        player->yPosition=i;
        player->spriteY=(i<<3)+(i<<2);//player->spriteY=i*12;//MULU is not good
    }
}

void pieceIntoBoard(Player* player)
{
    player->board[player->xPosition][player->yPosition]=player->fallingPiece[2];
    player->board[player->xPosition][player->yPosition-1]=player->fallingPiece[1];
    player->board[player->xPosition][player->yPosition-2]=player->fallingPiece[0];

    SPR_setVisibility(player->fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(player->fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(player->fallingPieceSprite[2],HIDDEN);

    player->drawStartX=player->xPosition;
    player->drawStartY=player->yPosition-2;
    player->drawEndX=player->xPosition+1;
    player->drawEndY=player->yPosition+1;

    if(player->flag_fastdrop==true)player->flag_fastdrop=false;

    //if(player->board[4][3]==0)player->flag_status=needPiece;
    //else if(player->board[4][3]!=0)player->flag_status=toppedOut;
    //this doesn't account for if they are clearing a piece that touches top
    player->flag_status=needPiece;

    player->flag_checkmatches=true;

    player->flag_redraw=true;

    player->chainAmount=0;//reset chain counter

    if(player==&P1)
    {
        sprintf(debug_string,"        ");//this is to clear out the combo text
        VDP_drawText(debug_string,2,1);

        sprintf(debug_string,"        ");//this is to clear out the chain text
        VDP_drawText(debug_string,2,2);
    }
}

void processDestroy(Player* player)
{
    player->chainAmount++;
    u8 howManyDestroyed=0;

    //for(u8 clearTextY=13;clearTextY<17;clearTextY++)VDP_clearTextBG(BG_A,13,clearTextY,9);

    u8 firstDestroyY=player->drawStartY;

    for (u8 destroyX=1;destroyX<maxX+1;destroyX++)
    {
        for (u8 destroyY=1;destroyY<maxY+1;destroyY++)
        {
            if (player->boardDestructionQueue[destroyX][destroyY]==true)
            {
                //sprintf(debug_string,"%d,%d,%d",destroyX,destroyY,player->board[destroyX][destroyY]);
                //VDP_drawText(debug_string,13,10+howManyDestroyed);

                //check the surrounding for garbage to be transformed
                if(player->board[destroyX+1][destroyY]==6){player->board[destroyX+1][destroyY]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}
                if(player->board[destroyX][destroyY+1]==6){player->board[destroyX][destroyY+1]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}
                if(player->board[destroyX-1][destroyY]==6){player->board[destroyX-1][destroyY]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}
                if(player->board[destroyX][destroyY-1]==6){player->board[destroyX][destroyY-1]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}

                player->board[destroyX][destroyY]=0;
                player->boardDestructionQueue[destroyX][destroyY]=false;
                howManyDestroyed++;

                if(destroyY<firstDestroyY)firstDestroyY=destroyY;
            }
        }
    }

    player->flag_destroy=false;
    player->flag_gravity=true;

    if(howManyDestroyed>3 && player==&P1)
    {
        sprintf(debug_string,"COMBO:%d",howManyDestroyed);
        VDP_drawText(debug_string,2,1);
    }

// METER
    player->meter++;//1 for the clear
    if(howManyDestroyed>3)player->meter+=howManyDestroyed-3;//combo
    if(player->chainAmount>1)player->meter+=(howManyDestroyed<<1);

    if(player->meter>99)player->meter=99;
// END METER

//redrawing less
    if(howManyDestroyed>=3)player->drawStartY=firstDestroyY-1;
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

    //if(howMuchGravity>0)
    //{
    player->drawStartX=1;
    player->drawStartY=firstGravityY-1;
    //player->drawStartX=firstGravityX-1;
    //player->drawStartY=1;
    //if(firstGravityY<player->drawStartY)player->drawStartY=firstGravityY-1;
    //player->drawEndX=maxX+1;
    player->drawEndX=maxX+1;
    player->drawEndY=maxY+1;
    player->flag_redraw=true;

    player->flag_checkmatches=true;
    //}

    player->flag_gravity=false;

    //if(howMuchGravity!=0)player->flag_checkmatches=true;

    if(player->chainAmount>1 && player==&P1)
    {
        sprintf(debug_string,"CHAIN:%d",player->chainAmount);
        VDP_drawText(debug_string,2,2);
    }
/*
    if(howMuchGravity>0)
    {
        sprintf(debug_string,"gravity moved %d",howMuchGravity);
        VDP_drawText(debug_string,1,2);
    }
*/
}

void manageDelays()
{
    if(P1.moveDelay>0)P1.moveDelay--;
    if(P2.moveDelay>0)P2.moveDelay--;

    //if(P1.fallDelay>0)P1.fallDelay--;
    //if(P2.fallDelay>0)P2.fallDelay--;

    if(P1.rotateDelay>0)P1.rotateDelay--;
    if(P2.rotateDelay>0)P2.rotateDelay--;

    if(globalSpawnCloudVisibilityTimer<40)globalSpawnCloudVisibilityTimer++;
}

void handleInput(Player* player, u16 buttons)
{
    if(player->flag_status!=toppedOut)
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

        //if (buttons & BUTTON_DOWN && player->fallDelay==0 && collisionTest(player, BOTTOM)==FALSE)
        if (buttons & BUTTON_DOWN && collisionTest(player, BOTTOM)==FALSE)
        {
            #define holdDownFallAmount 2

            if(player->fallingIncrement<TILESIZE-holdDownFallAmount)
            {
                player->fallingIncrement+=holdDownFallAmount;
                player->spriteY+=holdDownFallAmount;
            }

            if(player->fallingIncrement>=TILESIZE)
            {
                player->yPosition++;
                player->fallingIncrement=0;
            }
        }

        if (buttons & BUTTON_UP)// && player->yPosition>2)
        {
            player->flag_fastdrop=true;
        }

        if (buttons & BUTTON_B && player->rotateDelay==0 && player->has_let_go_B==true)
        {
            doRotate(player, DOWN);
            player->rotateDelay=ROTATE_DELAY_AMOUNT;
            player->has_let_go_B=false;
        }
        else if (buttons & BUTTON_A && player->rotateDelay==0 && player->has_let_go_A==true)
        {
            doRotate(player, UP);
            player->rotateDelay=ROTATE_DELAY_AMOUNT;
            player->has_let_go_A=false;
        }

        if(!(buttons & BUTTON_A))player->has_let_go_A=true;
        if(!(buttons & BUTTON_B))player->has_let_go_B=true;
    }

    /*
    if(buttons & BUTTON_C)//debug
    {
        //processGravity(&P1);

        P1.damageToBeReceived=8;
    }
    */
}

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

void drawMeter()
{
    #define meterYpos 1

    sprintf(debug_string,"%d", P1.meter);
    VDP_drawText(debug_string,1,meterYpos);

    sprintf(debug_string,"%d", P2.meter);
    VDP_drawText(debug_string,38,meterYpos);
}

void doRedraw(Player* player)
{
    printBoard(player, player->drawStartX,player->drawStartY,player->drawEndX,player->drawEndY);
    drawPlayerNext(player);

    player->flag_redraw=false;
    player->drawStartY=maxY-1;
}

void doCollisionLocking(Player* player)
{
    if(collisionTest(player, BOTTOM)==false)manageFalling(player);
    else if (player->flag_locking==false)
    {
        player->flag_locking=true;
        if(player==&P1)getTimer(P1fallLockingTimer,true);      //P1fallLockingTimer put it in struct
        else if(player==&P2)getTimer(P2fallLockingTimer,true);
    }
    else if(player->flag_locking==true)
    {
        if(player==&P1)
        {
            if(getTimer(P1fallLockingTimer,false)>=lockingDelay)  //P1fallLockingTimer put it in struct
                {
                    pieceIntoBoard(player);
                    player->flag_locking=false;
                }
        }
        else if(player==&P2)
        {
            if(getTimer(P2fallLockingTimer,false)>=lockingDelay) 
                {
                    pieceIntoBoard(player);
                    player->flag_locking=false;
                }
        }
    }
}