#include <genesis.h>
#include <resources.h>
#include <functions.h>

void printDebug();
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);

void checkMatches(Player* player);
void processDestroy(Player* player);
void processGravity(Player* player);
void manageDelays();
void sendDamage(Player* player, u8 amountDamageTaken);
void drawMeter();

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

    VDP_setPalette(PAL3,fallingSingleAll.palette->data);
    
    VDP_setTextPalette(PAL3);

    VDP_drawImageEx(BG_B,&gridbg,0x57E,0,0,TRUE,TRUE);//font uses symbols we might never need

    //VDP_setPalette(PAL2,monster.palette->data);
    VDP_drawImageEx(BG_A,&monster,TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, endOfInnerSectionsVRAM),13,10,TRUE,TRUE);
    VDP_drawImageEx(BG_A,&monster2,TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, endOfInnerSectionsVRAM+80),21,10,TRUE,TRUE);
    //VDP_fillTileMapRectInc(BG_A, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, endOfInnerSectionsVRAM), 21, 10, 6, 16);
    //VDP_setTileMapDataRect(BG_A, &monster, 21, 10, 6, 16, 10, DMA);

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

        if(P1.flag_destroy==false && P1.flag_checkmatches==false)
        {
            if(P1.flag_status==needPiece && P1.damageToBeReceived==0)createPiece(&P1);

            if(P1.flag_status!=needPiece)handleInput(&P1, JOY_readJoypad(JOY_1));

            if(collisionTest(&P1, BOTTOM)==false)manageFalling(&P1);
            else if (P1.flag_locking==false)
            {
                P1.flag_locking=true;
                getTimer(P1fallLockingTimer,true);
            }
            else if(P1.flag_locking==true)
            {
                if(getTimer(P1fallLockingTimer,false)>=lockingDelay)
                    {
                        pieceIntoBoard(&P1);
                        P1.flag_locking=false;
                    }
            }
        }

        if(P2.flag_destroy==false && P2.flag_checkmatches==false)
        {
            if(P2.flag_status==needPiece)createPiece(&P2);

            handleInput(&P2, JOY_readJoypad(JOY_2));

            if(collisionTest(&P2, BOTTOM)==false)manageFalling(&P2);
            else if (P2.flag_locking==false)
            {
                P2.flag_locking=true;
                getTimer(P2fallLockingTimer,true);
            }
            else if(P2.flag_locking==true)
            {
                if(getTimer(P2fallLockingTimer,false)>=lockingDelay)
                    {
                        pieceIntoBoard(&P2);
                        P2.flag_locking=false;
                    }
            }
        }

        if(P1.damageToBeReceived>0 && P1.flag_status==needPiece)sendDamage(&P1, P1.damageToBeReceived);
        if(P2.damageToBeReceived>0 && P2.flag_status==needPiece)sendDamage(&P2, P2.damageToBeReceived);

        if(P1.flag_checkmatches==true)checkMatches(&P1);
        if(P2.flag_checkmatches==true)checkMatches(&P2);

        if(P1.flag_destroy==true && getTimer(P1destroyTimer,false)>destroyDelay)processDestroy(&P1);
        if(P2.flag_destroy==true && getTimer(P2destroyTimer,false)>destroyDelay)processDestroy(&P2);

        if(P1.flag_gravity==true)processGravity(&P1);
        if(P2.flag_gravity==true)processGravity(&P2);

        if(P1.board[4][topOutYpos]!=0 && P1.flag_destroy==false && P1.flag_checkmatches==false)P1.flag_status=toppedOut;
        if(P2.board[4][topOutYpos]!=0 && P2.flag_destroy==false && P2.flag_checkmatches==false)P2.flag_status=toppedOut;

        SYS_doVBlankProcess();
        
        if(P1.flag_redraw==true)
        {
            //if(P1.drawStartY==0)P1.drawStartY=1;
            printBoard(&P1, P1.drawStartX,P1.drawStartY,P1.drawEndX,P1.drawEndY);
            drawPlayerNext(&P1);

            P1.flag_redraw=false;
            P1.drawStartY=maxY-1;
        }

        if(P2.flag_redraw==true)
        {
            printBoard(&P2, P2.drawStartX,P2.drawStartY,P2.drawEndX,P2.drawEndY);
            drawPlayerNext(&P2);

            P2.flag_redraw=false;
        }

        drawFallingSprite(&P1);
        drawFallingSprite(&P2);

        SPR_update();

        drawMeter();

        printDebug();
    }
    
    return 0;
}

void printDebug()
{
    sprintf(debug_string,"%ld", SYS_getFPS());
    VDP_drawText(debug_string,19,27);

    //sprintf(debug_string,"P1 %d", P1.flag_status);
    //VDP_drawText(debug_string,32,1);

    //sprintf(debug_string,"P1y:%d", P1.yPosition);
    //VDP_drawText(debug_string,2,4);

    //sprintf(debug_string,"P1x:%d", P1.xPosition);
    //VDP_drawText(debug_string,1,2);

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

/*
    //if(P1.flag_destroy==true)
    //if(1)
    {
        for (u8 printBoardX=1;printBoardX<maxX+1;printBoardX++)
        {
            for (u8 printBoardY=6;printBoardY<maxY+1;printBoardY++)
            {
                sprintf(debug_string,"%d",P1.boardDestructionQueue[printBoardX][printBoardY]);
                VDP_drawText(debug_string,printBoardX+27,printBoardY-3);
            }
        }
    }
*/
}

void manageFalling(Player* player)
{
    player->fallingIncrement++;
    player->spriteY++;

    if(player->fallingIncrement>=TILESIZE)
    {
        player->yPosition++;
        player->fallingIncrement=0;
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

    //for(u8 clearTextY=13;clearTextY<29;clearTextY++)VDP_clearTextBG(BG_A,13,clearTextY,18);
}

void checkMatches(Player* player)
{
    //for(u8 clearTextY=22;clearTextY<26;clearTextY++)VDP_clearTextBG(BG_A,13,clearTextY,16);//debug

    u8 connectionAmount,connectionColor;

    for (u8 checkX=1;checkX<maxX+1;checkX++)
    {
        for (u8 checkY=maxY+1;checkY>0;checkY--)
        {
            if(player->board[checkX][checkY]!=0 && player->board[checkX][checkY]!=globalNumColors)
            {
                if(player->board[checkX][checkY]==player->board[checkX+1][checkY])//match laterally 2 tiles
                {
                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];

                    for (u8 advance=checkX+2;advance<maxX+1;advance++)
                    {
                        if(player->board[advance][checkY]==connectionColor)connectionAmount++;
                        else if(player->board[advance][checkY]!=connectionColor)break;
                    }

                    if(connectionAmount>=3)
                    {
                        //sprintf(debug_string,"hori %d at %d,%d",connectionAmount,checkX,checkY);
                        //VDP_drawText(debug_string,13,19);
                        
                        for (u8 xAddDestructionQueue=0;xAddDestructionQueue<connectionAmount;xAddDestructionQueue++)
                        {
                            player->boardDestructionQueue[checkX+xAddDestructionQueue][checkY]=true;
                        }

                        player->flag_destroy=true;
                    }
                }
                if(player->board[checkX][checkY]==player->board[checkX][checkY-1])//match vertically 2 tiles
                {
                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];

                    //sprintf(debug_string,"init vert match at %d,%d",checkX,checkY);
                    //VDP_drawText(debug_string,13,28);

                    for (u8 advance=checkY-2;advance>0;advance--)
                    {
                        if(player->board[checkX][advance]==connectionColor)connectionAmount++;
                        else if(player->board[checkX][advance]!=connectionColor)break;
                    }

                    if(connectionAmount>=3)
                    {
                        //sprintf(debug_string,"vert %d at %d,%d",connectionAmount,checkX,checkY);
                        //VDP_drawText(debug_string,13,19);
                        
                        for (u8 yAddDestructionQueue=0;yAddDestructionQueue<connectionAmount;yAddDestructionQueue++)
                        {
                            player->boardDestructionQueue[checkX][checkY-yAddDestructionQueue]=true;
                        }

                        player->flag_destroy=true;
                    }
                }
                if(player->board[checkX][checkY]==player->board[checkX+1][checkY-1])//match diagonally up 2 tiles
                {
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
                    
                    if(connectionAmount>=3)
                    {
                        //sprintf(debug_string,"matched %d diagUp starting at %d,%d",connectionAmount,checkX,checkY);
                        //VDP_drawText(debug_string,13,28);
                        
                        for (u8 i=0;i<connectionAmount;i++)
                        {
                            player->boardDestructionQueue[checkX+i][checkY-i]=true;

                            //sprintf(debug_string,"diagUP %d,%d",checkX+i,checkY-i);
                            //VDP_drawText(debug_string,13,19+i);
                        }

                        player->flag_destroy=true;
                    }
                
                }
                if(player->board[checkX][checkY]==player->board[checkX+1][checkY+1])//match diagonally down 2 tiles
                {
                    connectionAmount=2;
                    connectionColor=player->board[checkX][checkY];

                    u8 incrementer=2;

                    for (u8 advance=checkY+2;(advance<maxY+1 && ((checkX+incrementer)<(maxX+1)));advance++)
                    {
                        if(player->board[checkX+incrementer][advance]==connectionColor)connectionAmount++;
                        else if(player->board[checkX+incrementer][advance]!=connectionColor)break;

                        incrementer++;
                    }

                    if(connectionAmount>=3)
                    {
                        //sprintf(debug_string,"matched %d diagDOWN starting at %d,%d",connectionAmount,checkX,checkY);
                        //VDP_drawText(debug_string,13,28);
                        
                        for (u8 i=0;i<connectionAmount;i++)
                        {
                            player->boardDestructionQueue[checkX+i][checkY+i]=true;
                        
                            //sprintf(debug_string,"diagDOWN %d,%d",checkX+i,checkY+i);
                            //VDP_drawText(debug_string,13,19+i);
                        }

                        player->flag_destroy=true;
                    }
                }
                else if(player->board[checkX][checkY]==0)break;//empty tile, leave
            }
        }
    }

    if(player->flag_destroy==true)
    {
        if(player==&P1)getTimer(P1destroyTimer,true);//restart p1 timer
        else if(player==&P2)getTimer(P2destroyTimer,true);//restart p2 timer
    }

    player->flag_checkmatches=false;

/*
    if(player->flag_destroy==false && player->board[4][3]!=0)
    {
        player->flag_status=toppedOut;
        sprintf(debug_string,"TOPOUT:CHECKMATCHES");
        VDP_drawText(debug_string,8,9);
    }
*/
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
            //player->fallingIncrement++;
            //player->spriteY+=2;

            //player->yPosition++;
            //player->spriteY+=TILESIZE;

            //manageFalling(player);

            //player->fallDelay=FALL_DELAY_AMOUNT;

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

    if(buttons & BUTTON_C)//debug
    {
        //processGravity(&P1);

        P1.damageToBeReceived=8;
    }
    if(buttons & BUTTON_START)//debug
    {
        for (u8 printBoardX=1;printBoardX<maxX+1;printBoardX++)
        {
            for (u8 printBoardY=9;printBoardY<maxY+1;printBoardY++)
            {
                sprintf(debug_string,"%d",P1.board[printBoardX][printBoardY]);
                VDP_drawText(debug_string,printBoardX,printBoardY-6);
            }
        }
    }
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