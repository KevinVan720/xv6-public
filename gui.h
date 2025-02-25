#ifndef GUI_H
#define GUI_H

#define GUI_BUF 0x9000

#ifndef __ASSEMBLER__

extern ushort SCREEN_WIDTH;
extern ushort SCREEN_HEIGHT;
extern int screen_size;

// 24 bit RGB. used in GUI Utility
typedef struct RGB {
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGB;

// 32 bit RGBA. used above GUI Utility
typedef struct RGBA {
    unsigned char A;
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGBA;


#endif
#endif