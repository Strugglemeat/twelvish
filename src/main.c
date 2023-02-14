#include <genesis.h>
#include <resources.h>
#include <functions.h>
#include <kdebug.h>

void gameLogicSwitch(Player* player);
void printDebug();
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);
void blinkMatches(Player* player);
void processDestroy(Player* player);
void processGravity(Player* player);
void manageDelays();
void sendDamage(Player* player, u8 amountDamageTaken);
void effectFastDrop(Player* player);

void doRedraw(Player* player);

void gameOver();

#define lockingDelayMaxTime 32000//24000 //higher is more time to let the player lock

#define maxLaterals maxX //how many times a player can fiddle with their sonic dropped piece

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

    while(1)//while(P1.flag_status!=toppedOut && P2.flag_status!=toppedOut)
    {
        manageDelays();

        gameLogicSwitch(&P1);

        SYS_doVBlankProcess();

        if(P1.flag_status<=fallingPiece)drawFallingSprite(&P1);//only draw if we're in spawning or falling
        if(P1.flag_redraw==true)doRedraw(&P1);

        if(globalSpawnCloudVisibilityTimer>20)SPR_setVisibility(sharedNextSpawnCloud,HIDDEN);

        SPR_update();

        printDebug();
    }

    gameOver();

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

void pieceIntoBoard(Player* player)
{
    KLog("**PIECE INTO BOARD**");
//write the colors of the locked pieces into the array
    player->board[player->xPosition][player->yPosition]=player->fallingPiece[2];
    player->board[player->xPosition][player->yPosition-1]=player->fallingPiece[1];
    player->board[player->xPosition][player->yPosition-2]=player->fallingPiece[0];

//set falling sprites to invis
    SPR_setVisibility(player->fallingPieceSprite[0],HIDDEN);
    SPR_setVisibility(player->fallingPieceSprite[1],HIDDEN);
    SPR_setVisibility(player->fallingPieceSprite[2],HIDDEN);

    player->chainAmount=0;//reset chain counter

    player->flag_status=checkingMatches;
    player->flag_locking=false;

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
}

void processDestroy(Player* player)
{
    KLog("processDestroy just started");
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
                if(player->board[destroyX+1][destroyY]==COLOR_GARBAGE){player->board[destroyX+1][destroyY]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                if(player->board[destroyX][destroyY+1]==COLOR_GARBAGE){player->board[destroyX][destroyY+1]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                if(player->board[destroyX-1][destroyY]==COLOR_GARBAGE){player->board[destroyX-1][destroyY]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                if(player->board[destroyX][destroyY-1]==COLOR_GARBAGE){player->board[destroyX][destroyY-1]=player->board[destroyX][destroyY];player->flag_status=checkingMatches;}
                //if(player->board[destroyX+1][destroyY]==COLOR_GARBAGE){player->board[destroyX+1][destroyY]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}
                //if(player->board[destroyX][destroyY+1]==COLOR_GARBAGE){player->board[destroyX][destroyY+1]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}
                //if(player->board[destroyX-1][destroyY]==COLOR_GARBAGE){player->board[destroyX-1][destroyY]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}
                //if(player->board[destroyX][destroyY-1]==COLOR_GARBAGE){player->board[destroyX][destroyY-1]=player->board[destroyX][destroyY];player->flag_checkmatches=true;}

                player->board[destroyX][destroyY]=0;
                player->boardDestructionQueue[destroyX][destroyY]=false;
                howManyDestroyed++;

                if(destroyY<firstDestroyY){
                    firstDestroyY=destroyY;
                    KLog("!!we reset firstDestroyY");
                }
            }
        }
    }

    player->flag_status=doingGravity;

    if(howManyDestroyed>3 && player==&P1)
    {
        sprintf(debug_string,"COMBO:%d",howManyDestroyed);
        VDP_drawText(debug_string,2,1);
    }

//redrawing less
    //if(howManyDestroyed>=3)player->drawStartY=firstDestroyY-1;//conditional doesn't make sense since we never get here w/o destroying at least 3
    player->drawStartY=firstDestroyY-1;
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

    player->drawStartX=1;
    player->drawStartY=firstGravityY-1;

    player->drawEndX=maxX+1;
    player->drawEndY=maxY+1;
    player->flag_redraw=true;

    player->flag_status=checkingMatches;

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

    if(P1.rotateDelay>0)P1.rotateDelay--;
    if(P2.rotateDelay>0)P2.rotateDelay--;

    if(globalSpawnCloudVisibilityTimer<40)globalSpawnCloudVisibilityTimer++;
}

void handleInput(Player* player, u16 buttons)
{
    //KLog("handleinput started");

    if(buttons & BUTTON_LEFT && player->moveDelay==0 && collisionTest(player, LEFT)==FALSE)
    {
        player->xPosition--;
        player->moveDelay=MOVE_DELAY_AMOUNT;
        player->spriteX-=TILESIZE;

        if(player->flag_locking==true)player->lockingLateralCounter++;
    }
    //else if (collisionTest(player, LEFT)==true)KLog("^^collis left");
    else if(buttons & BUTTON_RIGHT && player->moveDelay==0 && collisionTest(player, RIGHT)==FALSE)
    {
        player->xPosition++;
        player->moveDelay=MOVE_DELAY_AMOUNT;
        player->spriteX+=TILESIZE;

        if(player->flag_locking==true)player->lockingLateralCounter++;
    }
    //else if (collisionTest(player, RIGHT)==true)KLog("^^collis right");

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
/*
            if(player->fallingIncrement>=TILESIZE)
            {
                player->yPosition++;
                player->fallingIncrement=0;
            }
*/
        }

        if (buttons & BUTTON_UP)
        {
            effectFastDrop(player);
        }
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

    if(buttons & BUTTON_C)player->lockingLateralCounter=maxLaterals;
}

void doRedraw(Player* player)
{
//separate this into different elements
//do the VDP upload stuff, then when that is completely done, do the redraw
    printBoard(player, player->drawStartX,player->drawStartY,player->drawEndX,player->drawEndY);
    drawPlayerNext(player);

    player->flag_redraw=false;
    player->drawStartY=maxY-1;
}

void blinkMatches(Player* player)
{
    KLog("blinkMatches just started");//player->flag_status==blinkMatches;
    player->blinkTimes++;
    if(player->blinkTimes>=8)player->flag_status=destroyingMatches;
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
    SPR_update();
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

void manageFalling(Player* player)
{
    if(collisionTest(player, BOTTOM)==false)
    {
        player->fallingIncrement++;
        player->spriteY++;

        if(player->fallingIncrement>=TILESIZE)
        {
            player->yPosition++;
            player->fallingIncrement=0;
        }

        player->flag_locking=false;//reset
    }
    else if(player->flag_locking==false)
    {
        //if(player->fallingIncrement<TILESIZE-2)player->spriteY+=(TILESIZE-player->fallingIncrement-4);

        getTimer(P1fallLockingTimer,true);//start the timer
        player->lockingLateralCounter=0;
        player->flag_locking=true;
        return;
    }
    else if(player->flag_locking==true)
    {
        if((player->lockingLateralCounter>=maxLaterals || (getTimer(P1fallLockingTimer,false)>=lockingDelayMaxTime)))pieceIntoBoard(player);
    }

}

void effectFastDrop(Player* player)
{
    s8 i;//has to be outside of the for loop so it can be used afterwards

    for(i=player->yPosition;i<maxY;i++)
        if(player->board[player->xPosition][i+1]!=0)break;
    
    player->yPosition=i;
    player->spriteY=(i<<3)+(i<<2);//player->spriteY=i*12;//MULU is not good
}