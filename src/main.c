#include <genesis.h>
#include <resources.h>
#include <functions.h>

void printDebug();
void createPiece(Player* player);
void manageFalling(Player* player);
void handleInput(Player* player, u16 buttons);
bool collisionTest(Player* player, u8 direction);
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

        if(P1.board[4][3]!=0)P1.flag_status=toppedOut;
        if(P2.board[4][3]!=0)P2.flag_status=toppedOut;

        SYS_doVBlankProcess();
        
        //if(P1.flag_status==redraw)
        if(P1.flag_redraw==true)
        {
            printBoard(&P1, P1.drawStartX,P1.drawStartY,P1.drawEndX,P1.drawEndY);
            drawPlayerNext(&P1);

            //P1.flag_status=nothing;
            P1.flag_redraw=false;
        }

        //if(P2.flag_status==redraw)
        if(P2.flag_redraw==true)
        {
            printBoard(&P2, P2.drawStartX,P2.drawStartY,P2.drawEndX,P2.drawEndY);
            drawPlayerNext(&P2);

            //P2.flag_status=nothing;
            P2.flag_redraw=false;
        }

        drawSharedNext();

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

    //sprintf(debug_string,"P1:%d,%d,%d", P1.fallingPiece[0],P1.fallingPiece[1],P1.fallingPiece[2]);
    //VDP_drawText(debug_string,2,1);
    //sprintf(debug_string,"Pre:%d,%d,%d", P1.nextPiece[0],P1.nextPiece[1],P1.nextPiece[2]);
    //VDP_drawText(debug_string,1,2);
/*
    if(P1.flag_status==needPiece)
    {
        for (u8 printBoardX=1;printBoardX<maxX+1;printBoardX++)
        {
            for (u8 printBoardY=6;printBoardY<maxY+1;printBoardY++)
            {
                sprintf(debug_string,"%d",P1.board[printBoardX][printBoardY]);
                VDP_drawText(debug_string,printBoardX,printBoardY-3);
            }
        }
    }
*/
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

    if(buttons & BUTTON_C)//debug
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

    player->flag_status=needPiece;

    player->flag_checkmatches=true;

    player->flag_redraw=true;
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

                }
                if(player->board[checkX][checkY]==player->board[checkX+1][checkY-1])//match diagonally up 2 tiles
                {

                }
                if(player->board[checkX][checkY]==player->board[checkX+1][checkY+1])//match diagonally down 2 tiles
                {

                }
                else if(player->board[checkX][checkY]==0)break;//empty tile, leave
            }
        }
    }

    player->flag_checkmatches=false;
}

void processDestroy(Player* player)
{
    u8 howManyDestroyed=0;
    u8 debug_firstX=0;

    for (u8 destroyX=1;destroyX<maxX+1;destroyX++)
    {
        for (u8 destroyY=1;destroyY<maxY+1;destroyY++)
        {
            if (player->boardDestructionQueue[destroyX][destroyY]==true)
            {
                player->board[destroyX][destroyY]=0;
                player->boardDestructionQueue[destroyX][destroyY]=false;
                howManyDestroyed++;

                if(debug_firstX==0)debug_firstX=destroyX;
            }
        }
    }

    player->flag_destroy=false;
    player->flag_gravity=true;

/*
    if(howManyDestroyed>=3)
    {
        sprintf(debug_string,"destroyed %d starting at %d",howManyDestroyed,debug_firstX);
        VDP_drawText(debug_string,1,2);
    }
*/
}

void processGravity(Player* player)
{
    //u8 debug_howMuchGravity=0;

    for (u8 gravityY=maxY-1;gravityY>0;gravityY--)//don't need to start at maxY+1 cus there's no gravity for bottom tiles
    {
        for (u8 gravityX=1;gravityX<maxX+1;gravityX++)
        {
            if (player->board[gravityX][gravityY]!=0 && player->board[gravityX][gravityY+1]==0)
            {
                player->board[gravityX][gravityY+1]=player->board[gravityX][gravityY];
                player->board[gravityX][gravityY]=0;

                //debug_howMuchGravity++;
            }
            //else if(player->board[gravityX][gravityY]==0)break;
        }
    }

    player->drawStartX=1;
    player->drawStartY=1;
    player->drawEndX=maxX+1;
    player->drawEndY=maxY+1;
    player->flag_redraw=true;

    player->flag_gravity=false;
/*
    if(debug_howMuchGravity>0)
    {
        sprintf(debug_string,"gravity moved %d",debug_howMuchGravity);
        VDP_drawText(debug_string,1,2);
    }
*/
}