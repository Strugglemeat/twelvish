#include <genesis.h>
#include <resources.h>
#include <functions.h>

void printDebug();
void createPiece(Player* player);
void manageFalling(Player* player);
void pieceIntoBoard(Player* player);

void checkMatches(Player* player);
void processDestroy(Player* player);
void processGravity(Player* player);

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

    setSharedNext();
    drawPlayerNext(&P1);
    drawPlayerNext(&P2);

    //loadDebugFieldData();
    //printBoard(&P1, 1,1,maxX+1,maxY+1);

    while(1)
    {
        if(P1.moveDelay>0)P1.moveDelay--;
        if(P2.moveDelay>0)P2.moveDelay--;

        if(P1.fallDelay>0)P1.fallDelay--;
        if(P2.fallDelay>0)P2.fallDelay--;

        if(P1.rotateDelay>0)P1.rotateDelay--;
        if(P2.rotateDelay>0)P2.rotateDelay--;

        if(P1.flag_status==needPiece)createPiece(&P1);
        if(P2.flag_status==needPiece)createPiece(&P2);

        handleInput(&P1, JOY_readJoypad(JOY_1));
        handleInput(&P2, JOY_readJoypad(JOY_2));

        if(collisionTest(&P1, BOTTOM)==false){}
        else pieceIntoBoard(&P1);

        if(collisionTest(&P2, BOTTOM)==false){}
        else pieceIntoBoard(&P2);

        if(P1.flag_checkmatches==true)checkMatches(&P1);
        if(P2.flag_checkmatches==true)checkMatches(&P2);

        if(P1.flag_destroy==true)processDestroy(&P1);
        if(P2.flag_destroy==true)processDestroy(&P2);

        if(P1.flag_gravity==true)processGravity(&P1);
        if(P2.flag_gravity==true)processGravity(&P2);

        SYS_doVBlankProcess();
        
        if(P1.flag_redraw==true)
        {
            printBoard(&P1, P1.drawStartX,P1.drawStartY,P1.drawEndX,P1.drawEndY);
            drawPlayerNext(&P1);

            P1.flag_redraw=false;

            //debugIncrementer++;
        }

        if(P2.flag_redraw==true)
        {
            printBoard(&P2, P2.drawStartX,P2.drawStartY,P2.drawEndX,P2.drawEndY);
            drawPlayerNext(&P2);

            P2.flag_redraw=false;
        }

        //drawSharedNext(); //moved to within drawPlayerNext

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
    if(P2.flag_status==toppedOut)
    {
        sprintf(debug_string,"P2 topped out");
        VDP_drawText(debug_string,27,2);
    }

    //sprintf(debug_string,"P2:%d", P2.flag_status);
    //VDP_drawText(debug_string,25,5);

    VDP_clearTextBG(BG_A,16,10,12);//VDP_clearTextBG(VDPPlane plane, u16 x, u16 y, u16 w);
    sprintf(debug_string,"P1:%lu", getTimer(0,false));
    VDP_drawText(debug_string,13,10);

    //sprintf(debug_string,"dInc:%d", debugIncrementer);
    //VDP_drawText(debug_string,25,5);

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

    if(player->board[4][3]==0)player->flag_status=needPiece;
    else player->flag_status=toppedOut;
    
    player->flag_checkmatches=true;

    player->flag_redraw=true;

    player->chainAmount=0;

    sprintf(debug_string,"       ");//this is to clear out the combo text
    VDP_drawText(debug_string,2,1);

    sprintf(debug_string,"       ");//this is to clear out the chain text
    VDP_drawText(debug_string,2,2);
}

void checkMatches(Player* player)
{
    u8 connectionAmount,connectionColor;

    for (u8 checkX=1;checkX<maxX+1;checkX++)
    {
        for (u8 checkY=maxY+1;checkY>0;checkY--)
        {
            if(player->board[checkX][checkY]!=0)
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
                        //sprintf(debug_string,"matched %d laterally starting at %d",connectionAmount,checkX);
                        //VDP_drawText(debug_string,1,1);
                        
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
                    //VDP_drawText(debug_string,1,1);

                    for (u8 advance=checkY-2;advance>0;advance--)
                    {
                        if(player->board[checkX][advance]==connectionColor)connectionAmount++;
                        else if(player->board[checkX][advance]!=connectionColor)break;
                    }

                    if(connectionAmount>=3)
                    {
                        //sprintf(debug_string,"matched %d vertically starting at %d",connectionAmount,checkY);
                        //VDP_drawText(debug_string,1,1);
                        
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
                    //VDP_drawText(debug_string,1,1);

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
                        //VDP_drawText(debug_string,1,2);
                        
                        for (u8 i=0;i<connectionAmount;i++)
                        {
                            player->boardDestructionQueue[checkX+i][checkY-i]=true;
                        
                            //sprintf(debug_string,"diagUp clr %d,%d",checkX+i,checkY-i);
                            //VDP_drawText(debug_string,26,1+i);
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
                        //VDP_drawText(debug_string,1,2);
                        
                        for (u8 i=0;i<connectionAmount;i++)
                        {
                            player->boardDestructionQueue[checkX+i][checkY+i]=true;
                        
                            //sprintf(debug_string,"diagDOWN clr %d,%d",checkX+i,checkY+i);
                            //VDP_drawText(debug_string,22,1+i);
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
        if(player==&P1)getTimer(0,true);//restart p1 timer
        else if(player==&P2)getTimer(1,true);
    }

    player->flag_checkmatches=false;
}

void processDestroy(Player* player)
{
    player->chainAmount++;
    u8 howManyDestroyed=0;
    //u8 debug_firstX=0;

    for (u8 destroyX=1;destroyX<maxX+1;destroyX++)
    {
        for (u8 destroyY=1;destroyY<maxY+1;destroyY++)
        {
            if (player->boardDestructionQueue[destroyX][destroyY]==true)
            {
                player->board[destroyX][destroyY]=0;
                player->boardDestructionQueue[destroyX][destroyY]=false;
                howManyDestroyed++;

                //if(debug_firstX==0)debug_firstX=destroyX;
            }
        }
    }

    player->flag_destroy=false;
    player->flag_gravity=true;

    if(howManyDestroyed>3)
    {
        //sprintf(debug_string,"destroyed %d starting at %d",howManyDestroyed,debug_firstX);
        sprintf(debug_string,"combo %d",howManyDestroyed);
        VDP_drawText(debug_string,2,1);
    }

//    u8 drawStartX,drawStartY,drawEndX,drawEndY;

}

void processGravity(Player* player)
{
    u8 howMuchGravity=0;

    for (u8 gravityX=1;gravityX<maxX+1;gravityX++)
    {
        for (u8 gravityY=maxY;gravityY>0;gravityY--)//#define maxY 17
        {
            if (player->board[gravityX][gravityY]==0 && player->board[gravityX][gravityY-1]!=0)
            {
                player->board[gravityX][gravityY]=player->board[gravityX][gravityY-1];
                player->board[gravityX][gravityY-1]=0;

                //if(gravityX<=player->drawStartX)player->drawStartX=gravityX;
                //if(gravityY<player->drawStartY)player->drawStartY=gravityY-1;
                //if(gravityX>=player->drawEndX)player->drawEndX=gravityX;
                //if(gravityY>player->drawEndY)player->drawEndY=gravityY;

                gravityY=maxY+1;

                howMuchGravity++;
            }
        }
    }

    player->drawStartX=1;
    player->drawStartY=1;
    player->drawEndX=maxX+1;
    player->drawEndY=maxY+1;
    player->flag_redraw=true;

    player->flag_gravity=false;

    if(howMuchGravity!=0)player->flag_checkmatches=true;

    if(player->chainAmount>1)
    {
        sprintf(debug_string,"chain:%d",player->chainAmount);
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