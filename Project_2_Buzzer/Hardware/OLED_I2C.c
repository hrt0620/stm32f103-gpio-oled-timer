
#include "main.h"
#include "OLED_I2C.h"
#include "codetab.h"


// 延时函数实现
void DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

// GPIO配置函数 - 使用HAL库
void IIC_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIO时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 配置SCL引脚
    GPIO_InitStruct.Pin = OLED_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 开漏输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // 上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStruct);
    
    // 配置SDA引脚
    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    HAL_GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStruct);
    
    // 给一个停止信号, 复位IIC总线上的所有设备到待机模式
    IIC_Stop();
}

static void IIC_Delay(void)
{
    volatile uint8_t i;  // 加 volatile！
    for (i = 0; i < 20; i++);
}

void IIC_Start(void)
{
    /* 当SCL高电平时，SDA出现一个下跳沿表示IIC总线启动信号 */
    OLED_SDA_HIGH();
    OLED_SCL_HIGH();
    IIC_Delay();
    OLED_SDA_LOW();
    IIC_Delay();
    OLED_SCL_LOW();
    IIC_Delay();
}

void IIC_Stop(void)
{
    /* 当SCL高电平时，SDA出现一个上跳沿表示IIC总线停止信号 */
    OLED_SDA_LOW();
    OLED_SCL_HIGH();
    IIC_Delay();
    OLED_SDA_HIGH();
}

uint8_t IIC_WaitAck(void)
{
    uint8_t re;
    uint16_t timeout = 0; // 增加一个超时计数器
    
    OLED_SDA_HIGH();	/* CPU释放SDA总线 */
    IIC_Delay();
    OLED_SCL_HIGH();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
    IIC_Delay();
    
    // 【核心修改】加上超时判断，防止死机
    while (OLED_SDA_READ()) 
    {
        timeout++;
        if (timeout > 1000) // 等太久了就不等了，直接退出
        {
            OLED_SCL_LOW();
            return 1; // 返回1表示没收到应答
        }
    }
    re = 0; // 收到应答
    OLED_SCL_LOW();
    IIC_Delay();
    return re;
}

void Write_IIC_Byte(uint8_t _ucByte)
{
    uint8_t i;
    
    /* 先发送字节的高位bit7 */
    for (i = 0; i < 8; i++)
    {		
        if (_ucByte & 0x80)
        {
            OLED_SDA_HIGH();
        }
        else
        {
            OLED_SDA_LOW();
        }
        IIC_Delay();
        OLED_SCL_HIGH();
        IIC_Delay();	
        OLED_SCL_LOW();
        if (i == 7)
        {
             OLED_SDA_HIGH(); // 释放总线
        }
        _ucByte <<= 1;	/* 左移一个bit */
        IIC_Delay();
    }
}

void WriteCmd(unsigned char I2C_Command)//写命令
{
	IIC_Start();
	Write_IIC_Byte(OLED_ADDRESS);//OLED地址
	IIC_WaitAck();
	Write_IIC_Byte(0x00);//寄存器地址
	IIC_WaitAck();
	Write_IIC_Byte(I2C_Command);
	IIC_WaitAck();
	IIC_Stop();
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	IIC_Start();
	Write_IIC_Byte(OLED_ADDRESS);//OLED地址
	IIC_WaitAck();
	Write_IIC_Byte(0x40);//寄存器地址
	IIC_WaitAck();
	Write_IIC_Byte(I2C_Data);
	IIC_WaitAck();
	IIC_Stop();
}

void OLED_Init(void)
{
	DelayMs(100); //这里的延时很重要
	IIC_GPIO_Config();
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	//DelayMs(2);
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr_S(unsigned char x, unsigned char y, unsigned char ch[],  char POS,unsigned char len,unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	//DelayMs(2);
	POS = POS-1;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
				{
					if((j>=POS)&(j<(POS+len)))
					{
						WriteDat(~F6x8[c][i]);
					}
					else
					{
						WriteDat(F6x8[c][i]);
					}
				}
					
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
				{
					if((j>POS-1)&(j<(POS+len)))
					{
						WriteDat(~F8X16[c*16+i]);
					}
					else
					{
						WriteDat(F8X16[c*16+i]);
					}
				}
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
				{
					if((j>=POS)&(j<(POS+len)))
					{
						WriteDat(~F8X16[c*16+i+8]);
					}
					else
					{
						WriteDat(F8X16[c*16+i+8]);
					}
				}
				x += 8;
				j++;
			}
		}break;
	}
}
//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{

		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{

		adder += 1;
	}
}


//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char *s)
// Calls          : 
// Parameters     : x:起始列坐标,y -- 起始点行坐标(x:0~127, y:0~3);
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCC(unsigned char x, unsigned char y, unsigned char *s)
{
	uint16_t HZnum;
	uint16_t k,i,c;
	uint8_t wm=0;
	uint8_t adder=0;	
	 while(*s!=0)//数据未结束
	// for (l=0;l<len;l++)//数据未结束
   { 
		 if(*s<=0x80)
		 {
				c = *s - 32;
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				s+=1;
		 }
		 else
		 {
			HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//自动统计汉字数目
			for (k=0;k<HZnum;k++) 
			{
					adder = 0;
				if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
				{
					//unsigned char wm=0;
					//unsigned int  adder=32*k;
					OLED_SetPos(x , y);
					for(wm = 0;wm < 16;wm++)
					{
						WriteDat(tfont16[k].Msk[adder]);
						adder += 1;
					}
					OLED_SetPos(x,y + 1);
					for(wm = 0;wm < 16;wm++)
					{
						WriteDat(tfont16[k].Msk[adder]);
						adder += 1;
					}
				}
				continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
			}
					 s+=2; 
						x+=16;//下一个汉字偏移	
		}
	}
}



//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCC_S(unsigned char x, unsigned char y, unsigned char *s , char POS,unsigned char len)
{
	uint16_t HZnum;
	uint16_t k,i,c;
	uint8_t wm=0,j=0;
	uint8_t adder=0;	
		POS = POS-1;
		if((int)POS<0) POS = 0;
	
	 while(*s!=0)//数据未结束
	// for (l=0;l<len;l++)//数据未结束
   { 
		 if(*s<=0x80)
		 {				
				c = *s - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
				{
					if((j>POS-1)&(j<(POS+len)))
					{
						WriteDat(~F8X16[c*16+i]);
					}
					else
					{
						WriteDat(F8X16[c*16+i]);
					}
				}
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
				{
					if((j>=POS)&(j<(POS+len)))
					{
						WriteDat(~F8X16[c*16+i+8]);
					}
					else
					{
						WriteDat(F8X16[c*16+i+8]);
					}
				}
				x += 8;
				s+=1;
				j+=1;
		 }
		 else
		 {
			HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//自动统计汉字数目
			for (k=0;k<HZnum;k++) 
			{
					adder = 0;
				if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
				{
					//unsigned char wm=0;
					//unsigned int  adder=32*k;
					OLED_SetPos(x , y);
					for(wm = 0;wm < 16;wm++)
					{
						if((j>POS-1)&(j<(POS+len)))
						{
							WriteDat(~tfont16[k].Msk[adder]);
						}
						else
						{
							WriteDat(tfont16[k].Msk[adder]);
						}						
						adder += 1;
					}
					OLED_SetPos(x,y + 1);
					for(wm = 0;wm < 16;wm++)
					{
						if((j>POS-1)&(j<(POS+len)))
						{
							WriteDat(~tfont16[k].Msk[adder]);
						}
						else
						{
							WriteDat(tfont16[k].Msk[adder]);
						}	
						adder += 1;
					}
				}
				continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
			}
					 s+=2; 
					 j+=2;
						x+=16;//下一个汉字偏移	
		}
	}
}
//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}


void OLED_Set_Pos(uint8_t x, uint8_t y) 
{ 
	WriteCmd(0xb0+y);
	WriteCmd((((x+2)&0xf0)>>4)|0x10);
	WriteCmd(((x+2)&0x0f)); 
}
