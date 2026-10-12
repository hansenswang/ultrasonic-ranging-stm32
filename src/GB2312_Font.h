#ifndef GB2312_Font
#define GB2312_Font
//请在使用字库的C文件中包含头文件 #include "GB2312_Font.h"
typedef struct
{
char hz[2];//汉字字符
unsigned char Data[30];//汉字点阵
}HZ_Struct;
extern unsigned int code HZK_length;//汉字长度，数量
extern HZ_Struct code hz_library[];
#endif
/*
点阵数据格式（最后一列数据为0，省略）
B0  B1  B2  B3  B4  B5  B6  B7  B8  B9  B10 B11 B12 B13 B14 
B15 B16 B17 B18 B19 B20 B21 B22 B23 B24 B25 B16 B27 B28 B29
//数据竖直，低位在上
b0
b1
b2
b3
b4
b5
b6
b7
*/
