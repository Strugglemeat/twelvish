#define fallingPieceNumberOfTiles 3

typedef struct {

    u8 board[9][18];//7 wide by 16 tall. [0][0] not used. array is [9] because [8] gets fucky with the inner tiles
    u8 flag_status;

    Sprite* fallingPiece[fallingPieceNumberOfTiles];
    u8 newPiece[fallingPieceNumberOfTiles];//used in createPiece
    u16 spriteX;
    s16 spriteY;
    u8 xPosition,yPosition;

    u8 numColors;

    u8 leftright[4][16];
    u8 updown[7][8];
    u8 innerconnect[4][7][2];

    u8 fallingIncrement;

    u8 moveDelay;
    u8 fallDelay;

    u8 drawStartX,drawStartY,drawEndX,drawEndY;

} Player;

Player P1;
Player P2;

u8 randomRange(u8 rangeStart, u8 rangeEnd);
void initialize();
void loadTiles();
void loadDebugFieldData();
void clearBoard(Player* player);
void drawFallingSprite(Player* player);
void printBoardAll();
void drawTile(Player* player, u8 xPos, u8 yPos);

char debug_string[40] = "";

#define TILESIZE 12

#define maxX 7
#define maxY 17 //because 0 is the top of the spawning piece

#define xOffset 0//6
#define yOffset 1

#define ySpawn 0

#define spriteXorigin 44//48+36+8
#define spriteYorigin -24

#define player2offset 27
#define p2spriteXcreate (18*TILESIZE)

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

#define globalNumColors 6
#define ADDAMOUNT 4
#define ADDAMOUNT2 8
#define ADDAMOUNT3 12
#define ADDAMOUNT4 16
#define ADDAMOUNT5 20
#define extra_tiles_start  (4 + (ADDAMOUNT*(globalNumColors-1)) + 1)

void loadTiles()
{
    VDP_loadFontData(tileset_Font_Namco.tiles, 96, CPU);
    
    VDP_loadTileSet(fullblock_color1.tileset,1,CPU);
    VDP_loadTileSet(fullblock_color2.tileset,1+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(fullblock_color3.tileset,1+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(fullblock_color4.tileset,1+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(fullblock_color5.tileset,1+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(fullblock_color6.tileset,1+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(topblock_color1.tileset,2,CPU);
    VDP_loadTileSet(topblock_color2.tileset,2+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(topblock_color3.tileset,2+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(topblock_color4.tileset,2+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(topblock_color5.tileset,2+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(topblock_color6.tileset,2+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(rightblock_color1.tileset,3,CPU);
    VDP_loadTileSet(rightblock_color2.tileset,3+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(rightblock_color3.tileset,3+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(rightblock_color4.tileset,3+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(rightblock_color5.tileset,3+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(rightblock_color6.tileset,3+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(cornerblock_color1.tileset,4,CPU);
    VDP_loadTileSet(cornerblock_color2.tileset,4+(ADDAMOUNT*1),CPU);
    VDP_loadTileSet(cornerblock_color3.tileset,4+(ADDAMOUNT*2),CPU);
    VDP_loadTileSet(cornerblock_color4.tileset,4+(ADDAMOUNT*3),CPU);
    VDP_loadTileSet(cornerblock_color5.tileset,4+(ADDAMOUNT*4),CPU);
    VDP_loadTileSet(cornerblock_color6.tileset,4+(ADDAMOUNT*5),CPU);

    VDP_loadTileSet(leftright12.tileset,extra_tiles_start+0,CPU);//1,2
    VDP_loadTileSet(leftright13.tileset,extra_tiles_start+1,CPU);//1,3
    VDP_loadTileSet(leftright14.tileset,extra_tiles_start+2,CPU);
    VDP_loadTileSet(leftright15.tileset,extra_tiles_start+3,CPU);
    VDP_loadTileSet(leftright16.tileset,extra_tiles_start+4,CPU);

    VDP_loadTileSet(leftright23.tileset,extra_tiles_start+5,CPU);//2,3
    VDP_loadTileSet(leftright24.tileset,extra_tiles_start+6,CPU);
    VDP_loadTileSet(leftright25.tileset,extra_tiles_start+7,CPU);
    VDP_loadTileSet(leftright26.tileset,extra_tiles_start+8,CPU);

    VDP_loadTileSet(leftright34.tileset,extra_tiles_start+9,CPU);
    VDP_loadTileSet(leftright35.tileset,extra_tiles_start+10,CPU);
    VDP_loadTileSet(leftright36.tileset,extra_tiles_start+11,CPU);

    VDP_loadTileSet(leftright45.tileset,extra_tiles_start+12,CPU);
    VDP_loadTileSet(leftright46.tileset,extra_tiles_start+13,CPU);

    VDP_loadTileSet(leftright56.tileset,extra_tiles_start+14,CPU);


    VDP_loadTileSet(updown12.tileset,extra_tiles_start+15,CPU);//1,2
    VDP_loadTileSet(updown13.tileset,extra_tiles_start+16,CPU);//1,3
    VDP_loadTileSet(updown14.tileset,extra_tiles_start+17,CPU);
    VDP_loadTileSet(updown15.tileset,extra_tiles_start+18,CPU);
    VDP_loadTileSet(updown16.tileset,extra_tiles_start+19,CPU);

    VDP_loadTileSet(updown23.tileset,extra_tiles_start+20,CPU);//2,3
    VDP_loadTileSet(updown24.tileset,extra_tiles_start+21,CPU);
    VDP_loadTileSet(updown25.tileset,extra_tiles_start+22,CPU);
    VDP_loadTileSet(updown26.tileset,extra_tiles_start+23,CPU);

    VDP_loadTileSet(updown34.tileset,extra_tiles_start+24,CPU);
    VDP_loadTileSet(updown35.tileset,extra_tiles_start+25,CPU);
    VDP_loadTileSet(updown36.tileset,extra_tiles_start+26,CPU);

    VDP_loadTileSet(updown45.tileset,extra_tiles_start+27,CPU);
    VDP_loadTileSet(updown46.tileset,extra_tiles_start+28,CPU);

    VDP_loadTileSet(updown56.tileset,extra_tiles_start+29,CPU);
}

#define innerSectionsVRAM extra_tiles_start+30// 64 tiles worth of VRAM (32 per player)
#define endOfInnerSectionsVRAM innerSectionsVRAM+64

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

u8 randomRange(u8 rangeStart, u8 rangeEnd)//general use function
{
    return (random() % (rangeEnd + 1 - rangeStart)) + rangeStart;
}

void loadDebugFieldData()
{
    for (u8 x=1;x<maxX+1;x++)
    {
        for (u8 y=7;y<maxY+1;y++)
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

void initialize()
{
    P1.numColors=globalNumColors;
    P2.numColors=P1.numColors;

    clearBoard(&P1);
    clearBoard(&P2);

    P1.fallingPiece[0] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    P1.fallingPiece[1] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    P1.fallingPiece[2] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));

    P2.fallingPiece[0] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    P2.fallingPiece[1] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    P2.fallingPiece[2] = SPR_addSpriteSafe(&fallingSingleAll, -TILESIZE, -TILESIZE, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));

    SPR_setVisibility(P1.fallingPiece[0],HIDDEN);
    SPR_setVisibility(P1.fallingPiece[1],HIDDEN);
    SPR_setVisibility(P1.fallingPiece[2],HIDDEN);

    SPR_setVisibility(P2.fallingPiece[0],HIDDEN);
    SPR_setVisibility(P2.fallingPiece[1],HIDDEN);
    SPR_setVisibility(P2.fallingPiece[2],HIDDEN);

    P1.flag_status=needPiece;
    P2.flag_status=needPiece;

    P1.fallingIncrement=0;
    P2.fallingIncrement=0;
}

void drawTile(Player* player, u8 xPos, u8 yPos)//TILE_ATTR_FULL(pal, prio, flipV, flipH, index)
{
    u8 colorAdd=0;//4 tiles for each color. so color 2 is adding 4, color 3 is adding 8

    //if we are drawing a tile other than color 1, we need to increase the tile index    
    if(player->board[xPos][yPos]>1)colorAdd=(player->board[xPos][yPos]-1)<<2;//multiply by 4

    u8 p2offsetX=0;
    if(player==&P2)p2offsetX=player2offset;

    u8 drawingxPos=xPos+(xPos>>1)+p2offsetX;
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