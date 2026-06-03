#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "sh1106oled.h"
#include "sh1106oledfont.h"

//声明变量
volatile uint8_t s_chDispalyBuffer[128][8];

//定义全局变量
volatile uint8_t Flag =0;
volatile uint8_t clock_sec = 0;
volatile uint8_t clock_min = 0;
volatile uint8_t clock_hour=0; 


//声明静态函数，不调用的函数
static void sh1106_write_cmd(uint8_t chdata);
static void sh1106_write_data(uint8_t chdata);


//时钟初始化
void CLOCK_Init(void)
{
    clock_sec  = 0;
    clock_min  = 0;
    clock_hour = 0;
    Flag = 0;
}

//时钟自增
void CLOCK_TimeIncrement(void)
{
    clock_sec++;
    if (clock_sec > 59)
    {
        clock_sec = 0;
        clock_min++;
        if (clock_min > 59)
        {
            clock_min = 0;
            clock_hour++;
            if (clock_hour > 23)
            {
                clock_hour = 0;
            }
        }
    }
    Flag = 1;//设一个标志位
}


//显示屏复位
void sh1106_oled_rest()
{
	HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_SET);
	HAL_Delay(1000);
}

//发送命令
void sh1106_write_cmd(uint8_t chdata)
{	
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin,GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3,&chdata,1,0xff);
}
//发送数据的函数,发完指令发数据
void sh1106_write_data(uint8_t chdata)
{	
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin,GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi3,&chdata,1,0xff);
}
//初始化
void sh1106_init()
{
sh1106_oled_rest();//让芯片复位
	
sh1106_write_cmd(0xAE);//关闭显示，外设不同控制地址可能不同
	
sh1106_write_cmd(0x00);//设置列地址初始化
sh1106_write_cmd(0x10);
	
sh1106_write_cmd(0x40);//设置屏幕从哪一行开始显示（0x00~0x3F）。这里设为 0x00，即从第 0 行开始。
	
sh1106_write_cmd(0x81);//0x81 是双字节命令，后一个字节为对比度值（0x00~0xFF）。0xCF 接近最大值，屏幕会较亮。
sh1106_write_cmd(0xCF);
	
sh1106_write_cmd(0xA1);//列地址 0 对应 SEG0（左）；  0xA1：列地址 0 对应 SEG131（右）→ 左右镜像。    
sh1106_write_cmd(0xC0);//COM0 对应第一行（正常）；0xC8：COM63 对应第一行（上下翻转）
	
sh1106_write_cmd(0xA6);//-正常显示（1 点亮，0 熄灭）。反色显示。
	
sh1106_write_cmd(0xA8);
sh1106_write_cmd(0x3f);//告诉驱动芯片屏幕实际物理行数为 64。A8-3F=64
	
sh1106_write_cmd(0xD3);//-set display offset Shift Mapping RAM Counter (0x00~0x3F)
sh1106_write_cmd(0x00);//-not offset，可让显示内容整体上下平移（0~63）。这里不偏移。
	
sh1106_write_cmd(0xd5);
sh1106_write_cmd(0x80);//手册给出 0x80 可得到约 100 FPS 刷新率。
	
sh1106_write_cmd(0xD9);//影响显示质量，通常按手册推荐值设置。
sh1106_write_cmd(0xF1);
	
sh1106_write_cmd(0xDA);
sh1106_write_cmd(0x12);//具体值需对照屏幕硬件（常见为 0x12 或 0x02）。
				
sh1106_write_cmd(0xDB);//设置 VCOM 电压值（0x00~0xFF），影响对比度微调，一般为 0x40。
sh1106_write_cmd(0x40);
				
sh1106_write_cmd(0x20);//-Set Page Addressing Mode (0x00水平寻址模式/0x01垂直寻址模式/0x02页寻址模式)
sh1106_write_cmd(0x02);//页寻址模式（最常用，每次写入一页 8 行后需手动换页）。
				
sh1106_write_cmd(0x8D);//-SH1106 内部需要升压产生驱动电压，必须使能电荷泵，否则屏幕完全不亮。
sh1106_write_cmd(0x14);//0x14：使能；0x10：关闭。
				
sh1106_write_cmd(0xA4);// 正常显示 RAM 内容。
				
sh1106_write_cmd(0xA6);// 这里是重复设置，确保为正常显示。
				
sh1106_write_cmd(0xAF);//完成所有配置后，最终让屏幕亮起。
}

//清屏函数
void clr_oled_screen()
{
	//定义数组把所有二极管都算进去，8页，每页8位，共64行，128列
	uint8_t i,j;

	for(i=0;i<8;i++)
	{
		//底层逻辑就是按页，先将一页熄灭，再熄灭其余页，嵌套循环就能实现
		sh1106_write_cmd(0xB0 + i);
		//再发列，通过高四位和低四位,每个页的列是一样的
		sh1106_write_cmd(0x02);
		sh1106_write_cmd(0x10);
		for(j=0;j<128;j++)
		{
			s_chDispalyBuffer[j][i]=0;	
			sh1106_write_data(s_chDispalyBuffer[j][i]);
		}
	}
}


//刷新函数

void sh1106_refresh_gram(void)
{
   uint8_t i, j;
    for (i = 0; i < 8; i ++) {  
        sh1106_write_cmd(0xB0 + i); //设置页码从0xB0开始到0xB7   
        sh1106_write_cmd(0x02); //列起始地址低四位
        sh1106_write_cmd(0x10); //列起始地址高四位      
        for (j = 0; j < 128; j ++) {
					sh1106_write_data(s_chDispalyBuffer[j][i]); 
                }
        }   
}
//画点函数
void sh1106_draw_point(uint8_t chXpos, uint8_t chYpos, uint8_t chPoint)
{
        uint8_t chPos, chBx, chTemp = 0;
        //判断有没有超出OLED范围
        if (chXpos > 127 || chYpos > 63) {
                return;
        }
        //chYpos坐标转换，因为我们用8个字节管理了64个bit,所以需要把y坐标转换到对应的字节bit位置
        chPos = 7 - chYpos / 8;   //找出那一页，屏幕从下到上，0-7，倒置处理
        chBx = chYpos % 8;        //找出哪一位
        chTemp = 1 << (7 - chBx); //生7-chBx也是倒置处理。成的 chTemp 是一个掩码，把对应位置1，8位二进制
				//这里只是ifelse语句，说明一次点亮一个
        if (chPoint) {
					//就是判断要不要亮，如果要亮，就与更新数组，初始数组全0，这里是列与哦，手动指定
            s_chDispalyBuffer[chXpos][chPos] |= chTemp;//数组[列][页] |= 掩码
        } else {
					s_chDispalyBuffer[chXpos][chPos] &= ~chTemp;//数组[列][页] &= ~掩码，直接全为0了
					//这个模块是从下往上的，，因此上面做了倒置处理，已经过倒置处理。
					//     掩码：   0b0010 0000
					//状态为1时：		0b0010 0000，得到的是1（初始页对应的列全为0，8位，0010 0000或0010 0000）
					//状态为0时：		0b1101 1111，得到的是0（1101 1111与0010 0000）
        }
}

//画位图函数
//时钟显示用的位图绘制
void sh1106_draw_bitmap(uint8_t chXpos, uint8_t chYpos, const uint8_t *pchBmp, uint8_t chWidth, uint8_t chHeight)
{
    uint16_t i, j, byteWidth = (chWidth + 7) / 8;
    //遍历图片的宽高，取出每一点，判断为1的位，为需要点亮的点，通过画点函数绘制到屏幕
    for(j = 0; j < chHeight; j ++){ //这里外层是行选
        for(i = 0; i < chWidth; i ++ ) {
					//j * byteWidth行偏移，i / 8列偏移，(i & 7)右移位数
            if(*(pchBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
							//这里就是点亮，调用了画点的函数，相当于这里的x变化，列就变化，外层的是宽度，相当于行的变化？
                sh1106_draw_point(chXpos + i, chYpos + j, 1);
            }
        }
    }

}
//数字显示
void OLED_ShowNum(uint8_t x, uint8_t y, uint8_t num, uint8_t fontSize)
{
    if(num > 9) return;
    
    if(fontSize == 5) {
        // 16x16像素字体（需要16字节字模）
        sh1106_draw_bitmap(x, y, num_5x7[num], 5, 7);
    } else if(fontSize == 8) {
        // 8x16像素字体（需要16字节字模）
        sh1106_draw_bitmap(x, y, num_8x16[num], 8, 16);
    }
}

//冒号显示
void OLED_ShowColon(uint8_t x, uint8_t y)
{
    sh1106_draw_bitmap(x, y, colon_4x16, 4, 16);
}

//局部擦除
// 画一个实心矩形，color: 1点亮，0熄灭
void sh1106_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
    uint8_t i, j;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            sh1106_draw_point(x + i, y + j, color);
        }
    }
}
//时间封装,这里可以说是个小时计时器，从0开始
void OLED_ShowTime(uint8_t clock_hour, uint8_t clock_min, uint8_t clock_sec)
{
    uint8_t sec_units   = clock_sec % 10;
    uint8_t sec_tens    = clock_sec / 10;
    uint8_t min_units   = clock_min % 10;
    uint8_t min_tens    = clock_min / 10;
    uint8_t hour_units  = clock_hour % 10;
    uint8_t hour_tens   = clock_hour / 10;

    sh1106_fill_rect(0, 0, 48, 16, 0);
    // 时十位
    OLED_ShowNum(0,   0, hour_tens, 8);
    // 时个位
    OLED_ShowNum(8,  0, hour_units, 8);
    // 冒号（需要单独制作冒号位图）
    OLED_ShowColon(16, 0);  // 自定义函数
    // 分十位
    OLED_ShowNum(20,  0, min_tens, 8);
    // 分个位
    OLED_ShowNum(28,  0, min_units, 8);
    // 秒十位
    OLED_ShowNum(36,  9, sec_tens , 5);
    // 秒个位
    OLED_ShowNum(42,  9,sec_units, 5);
    // 刷新显示（draw_bitmap 内部已调用 refresh）
	sh1106_refresh_gram();
}

//小数点
void OLED_Showfloat(uint8_t x, uint8_t y)
{
    sh1106_draw_point(x,  y, 1);
    sh1106_draw_point(x+1,y, 1);
    sh1106_draw_point( x, y+1, 1);
    sh1106_draw_point(x+1, y+1, 1);
}


//温湿度显示
void value_show(uint8_t x, uint8_t y, float value)
{
    int int_part = (int)value;                      // 整数部分
    //整数部分
    uint8_t tem_units = int_part % 10;
    uint8_t tem_tens = int_part  / 10;
    //小数部分
    uint8_t temp_float = (int)((value-int_part)*10);
    if(value<0){
        sh1106_draw_bitmap(x, y, minus_sign_8x16, 8, 16);
        x+=8;
        int_part=-int_part;
    }
    else if(value>=10){
        OLED_ShowNum(x,  y, tem_tens, 8);
        x+=8;
        OLED_ShowNum(x,  y, tem_units, 8);
        x+=8;
    }
    else{
        OLED_ShowNum(x,  y, tem_units , 8);
    }
    OLED_Showfloat(x, y-4);//小数点
    OLED_Showfloat(x, y+12);//小数点
    x+=4;
    OLED_ShowNum(x,  y+8, temp_float, 5);
    x+=8;
        //sh1106_fill_rect(80, 0, 40, 16, 0);
    if(y==0){
        sh1106_draw_bitmap(x, y,ascii_8x16[95], 8, 16);//°
        x+=8;
        sh1106_draw_bitmap(x, y,ascii_8x16[35], 8, 16); //C	
    }
    else if(y==16){
        x+=8;
        sh1106_draw_bitmap(x, y,ascii_8x16[5], 8, 16);//%
    }
    
}
void OLED_ShowTEM_HUM(uint8_t x, uint8_t y, float value1,float value2)
{
    //这里以图标起点位x起点，图标48+32=80，图标的宽度32，图标的宽度32，数值的起点x+32
    sh1106_fill_rect(x, y, y+80, 32, 0);
    sh1106_draw_bitmap(x, y,gImage_222, 32, 32);//图标

    value_show(x+32, y, value1);
    value_show(x+32,y+16, value2);
    sh1106_refresh_gram();
}






