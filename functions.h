typedef struct {

    u8 board[10][18];//8 wide by 16 tall. [0][0] not used.
    u8 flag_status;

    Sprite* fallingPiece[2];
    u16 spriteX;
    s16 spriteY;
    u8 xPosition,yPosition;
    u8 numColors;

    u8 leftright[4][16];
    u8 updown[7][8];
    u8 topbot[4][2];
    u8 innerconnect[4][7][2];

    u8 fallingIncrement;
    
    //u8 rotation;//0=spawn. secondary piece above. 1=secondary piece on the right. 2=secondary piece below. 3=to the left
    //u8 fallingPieceArea[3][3];//[0][0] is top left, [2][2] is bot right

    u8 moveDelay;
    u8 fallDelay;

    u8 newPiece[2];//used in createPiece

} Player;

Player P1;
Player P2;

u8 randomRange(u8 rangeStart, u8 rangeEnd);
void loadTiles();
void drawTile(u8 xPos, u8 yPos);
void printBoard();
void clearBoard(Player* player);

char debug_string[40] = "";

#define maxX 8
#define maxY 17 //because 0 is the top of the spawning piece

#define xOffset 5
#define yOffset 1

#define TILESIZE 12

enum status
{
    nothing,
    redraw,
    needPiece,
    toppedOut
};

enum direction
{
    TOP = 1,
    BOTTOM = 2,
    LEFT = 3,
    RIGHT = 4
};

#define false 0
#define true 1

#define MOVE_DELAY_AMOUNT 6
#define FALL_DELAY_AMOUNT 4

u8 randomRange(u8 rangeStart, u8 rangeEnd)//general use function
{
    return (random() % (rangeEnd + 1 - rangeStart)) + rangeStart;
}

#define ADDAMOUNT 4 //#colors + 1 ? 3colors+1=4, 5colors+1=6
#define EXTRA_TILES_BEGIN 13

void loadTiles()
{
    VDP_loadFontData(tileset_Font_Namco.tiles, 96, CPU);
    
    VDP_loadTileSet(fullblock_color1.tileset,1,CPU);
    VDP_loadTileSet(fullblock_color2.tileset,1+ADDAMOUNT,CPU);
    VDP_loadTileSet(fullblock_color3.tileset,1+ADDAMOUNT+ADDAMOUNT,CPU);

    VDP_loadTileSet(topblock_color1.tileset,2,CPU);
    VDP_loadTileSet(topblock_color2.tileset,2+ADDAMOUNT,CPU);
    VDP_loadTileSet(topblock_color3.tileset,2+ADDAMOUNT+ADDAMOUNT,CPU);
    
    VDP_loadTileSet(rightblock_color1.tileset,3,CPU);
    VDP_loadTileSet(rightblock_color2.tileset,3+ADDAMOUNT,CPU);
    VDP_loadTileSet(rightblock_color3.tileset,3+ADDAMOUNT+ADDAMOUNT,CPU);

    VDP_loadTileSet(cornerblock_color1.tileset,4,CPU);
    VDP_loadTileSet(cornerblock_color2.tileset,4+ADDAMOUNT,CPU);
    VDP_loadTileSet(cornerblock_color3.tileset,4+ADDAMOUNT+ADDAMOUNT,CPU);

    VDP_loadTileSet(leftright12.tileset,EXTRA_TILES_BEGIN+0,CPU);//1,2
    VDP_loadTileSet(leftright13.tileset,EXTRA_TILES_BEGIN+1,CPU);//1,3
    VDP_loadTileSet(leftright23.tileset,EXTRA_TILES_BEGIN+2,CPU);//2,3

    VDP_loadTileSet(updown12.tileset,EXTRA_TILES_BEGIN+3,CPU);//1,2
    VDP_loadTileSet(updown13.tileset,EXTRA_TILES_BEGIN+4,CPU);//1,3
    VDP_loadTileSet(updown23.tileset,EXTRA_TILES_BEGIN+5,CPU);//2,3

    VDP_loadTileSet(topbot12.tileset,EXTRA_TILES_BEGIN+6,CPU);//1,2
    VDP_loadTileSet(topbot13.tileset,EXTRA_TILES_BEGIN+7,CPU);//1,3
    VDP_loadTileSet(topbot23.tileset,EXTRA_TILES_BEGIN+8,CPU);//2,3
}

#define innerSectionsVRAM EXTRA_TILES_BEGIN+8//22//56 tiles worth of VRAM (28 per player)

void clearBoard(Player* player)
{
    for (u8 boardX=1;boardX<maxX+1;boardX++)
    {
        for (u8 boardY=1;boardY<maxY+1;boardY++)
        {
            player->board[boardX][boardY]=0;
        }
    }
}

void loadDebugFieldData()
{
    for (u8 x=1;x<maxX+1;x++)
    {
        for (u8 y=1;y<maxY+1;y++)
        {
            P1.board[x][y]=randomRange(1,P1.numColors);
        }
    }

/*
    P1.board[1][maxY]=randomRange(1,P1.numColors);
    P1.board[2][maxY]=randomRange(1,P1.numColors);

    P1.board[4][maxY]=randomRange(1,P1.numColors);
    P1.board[5][maxY]=randomRange(1,P1.numColors);

    P1.board[maxX-1][maxY]=randomRange(1,P1.numColors);
    P1.board[maxX][maxY]=randomRange(1,P1.numColors);
*/
}

void drawTile(u8 xPos, u8 yPos)//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
{
    u8 colorAdd=0;//4 tiles for each color. so color 2 is adding 4, color 3 is adding 8

    //if we are drawing a tile other than color 1, we need to increase the tile index    
    if(P1.board[xPos][yPos]>1)colorAdd=(P1.board[xPos][yPos]-1)<<2;//multiply by 4

    u8 drawingxPos=xPos+(xPos>>1);
    u8 drawingyPos=yPos+(yPos>>1);

    if(xPos%2!=0 && yPos%2!=0)//odd column, odd row (1,1)
    {
        //top left: bottom half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
        //top right: bottom left corner
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 4+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
        //bottom left: full square
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
        //bottom right: left half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);
    }
    else if(xPos%2==0 && yPos%2!=0)//even column, odd row (2,1)
    {
        //top left: bottom right corner
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 4+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
        //top right: bottom half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
        //bottom left: right half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);
        //bottom right: full square
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
    }
    else if(xPos%2!=0 && yPos%2==0)//odd column, even row (1,2)
    {
        //top left: full square
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
        //top right: left half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, TRUE, 3+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset-1, 1, 1);
        //bottom left: top half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
        //bottom right: top left corner
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, TRUE, 4+colorAdd), drawingxPos+xOffset+1, drawingyPos+yOffset, 1, 1);//was FALSE,TRUE,5
    }
    else if(xPos%2==0 && yPos%2==0)//even column, even row (2,2)
    {
        //top left: right half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 3+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset-1, 1, 1);
        //top right: full square
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 1+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset-1, 1, 1);
        //bottom left: top right corner
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, TRUE, FALSE, 4+colorAdd), drawingxPos+xOffset-1, drawingyPos+yOffset, 1, 1);//was FALSE,FALSE,5
        //bottom right: top half
        VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 2+colorAdd), drawingxPos+xOffset, drawingyPos+yOffset, 1, 1);
    }
}