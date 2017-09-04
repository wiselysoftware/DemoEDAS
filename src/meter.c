#include <stdio.h>
#include "../i2c-tools/include/linux/i2c-dev.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <asm/ioctl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
/*

Matar todos los que no estas usando.
Configurar bien el limit register.
Obtener direcciones de los otros monos.
*/


// Must check with the board, values from schematic
enum ADCS   
  {
    U33 = 0x1F, //VOLTAGE
    U38 = 0x37  //CURRENT
  };

enum defChanRead
  {
    CHAN1 = 0x20,
    CHAN2 = 0x21,
    CHAN3 = 0x22,
    CHAN4 = 0x23,
    CHAN5 = 0x24,
    CHAN6 = 0x25
  };

enum defHighLim
  {
    CHANHL1 = 0x2A,
    CHANHL2 = 0x2C,
    CHANHL3 = 0x2E,
    CHANHL4 = 0x30,
    CHANHL5 = 0x32,
    CHANHL6 = 0x34
  };

enum defLowLim
  {
    CHANLL1 = 0x2B,
    CHANLL2 = 0x2D,
    CHANLL3 = 0x2F,
    CHANLL4 = 0x31,
    CHANLL5 = 0x33,
    CHANLL6 = 0x35
  };


int getChannel (char *stringValue)
{
  if(strcmp(stringValue,"1")==0)
    return 1;
  else if(strcmp(stringValue,"2")==0)
    return 2;
  else if(strcmp(stringValue,"3")==0)
    return 3;
  else if(strcmp(stringValue,"4")==0)
    return 4;
  else if(strcmp(stringValue,"5")==0)
    return 5;
  else if(strcmp(stringValue,"6")==0)
    return 6;
  else 
    return -1;
}

int main(int argc, char *argv[])
{
  int goodInputFlag=0;
  int i2cAddr;
  int fd;
  int res3, res4,res5;
  unsigned long res2;
  __s32 res;
  int chanReadReg;
  int highLimReg, lowLimReg;
  int chann;
  enum ADCS ADCSInstance[] = {U33, U38};
  enum defHighLim highLimRegs[] = {CHANHL1,CHANHL2,CHANHL3,CHANHL4,CHANHL5,CHANHL6};
  enum defLowLim lowLimRegs[] =  {CHANLL1,CHANLL2,CHANLL3,CHANLL4,CHANLL5,CHANLL6};
  enum defChanRead chanReadRegs[] = {CHAN1,CHAN2,CHAN3,CHAN4,CHAN5,CHAN6}; 

  if (argc == 3)
    {
      if(strcmp(argv[1],"v")==0)
	{
	  chann=getChannel(argv[2]);
	  printf("VOLTAGE\n");
	  if(chann == -1)
	    {
	      printf("Channel not defined\n");
	      return -1; //exit(1)???
	    }
	  else
	    {
	      goodInputFlag=1;
	      i2cAddr = ADCSInstance[0];
	      printf("Channel %d\n",chann);
	      chanReadReg =  chanReadRegs [chann-1];
	      highLimReg = highLimRegs[chann-1];
	      lowLimReg = lowLimRegs[chann-1];
	    }
	}
      else if (strcmp(argv[1],"a")==0)
	{
	  chann=getChannel(argv[2]);
	  printf("CURRENT\n");
	  if(getChannel(argv[2]) == -1)
	    {
	      printf("Channel not defined\n");
	      return -1; //exit(1)???
	    }
	  else
	    {
	      goodInputFlag=1;
	      i2cAddr = ADCSInstance[1];
	      printf("Channel %d\n",getChannel(argv[2]));
	      chanReadReg =  chanReadRegs [chann-1];
	      highLimReg = highLimRegs[chann-1];
	      lowLimReg = lowLimRegs[chann-1];
	    }
      }
      else
	printf("Argument invalid\n");
    }
  else if (argc > 3)
    {
    printf("Too many arguments supplied.\n");
    return -1; //exit(1)???
    }
  else
    {
    printf("One argument expected.\n");
    return -1; //exit(1)???
    }
  if (goodInputFlag==1)
    {
      fd = open("/dev/i2c-1", O_RDWR);
      if (fd < 0)
	{
	  printf("ERROR: open(%d) failed\n", fd);
	  exit(1); //return -1???
	}
      if (ioctl(fd, I2C_SLAVE, i2cAddr) < 0)
	{
	  printf("ERROR: Couldn't connect to slave\n");
	  exit(1);  //return -1???
	}


      /*###########################################################################
      Here we should read and check the configuration of the ADC, for that we should use
      something like setting a global variable:

      __s32 registers[12]={0,0,0,0,0,0,0,0,0,0,0,0};

      and read like:
      
      registers[0]= i2c_smbus_read_byte_data(fd,  0x00);//00: Configuration Register
      registers[1]= i2c_smbus_read_byte_data(fd,  0x01);//01: Interrupt status Reg
      registers[2]= i2c_smbus_read_byte_data(fd,  0x03);//03:Interrupt mask Reg
      registers[3]= i2c_smbus_read_byte_data(fd,  0x07);//07: Conversion rate reg
      registers[4]= i2c_smbus_read_byte_data(fd,  0x08);//08: Channel disable reg 
      registers[5]= i2c_smbus_read_byte_data(fd,  0x0A);//0A: Deep shutdown reg 
      registers[6]= i2c_smbus_read_byte_data(fd,  0x0B);//0B: Advanced config reg 
      registers[7]= i2c_smbus_read_byte_data(fd,  0x0C);//0C: Busy status reg
      registers[8]= i2c_smbus_read_byte_data(fd,  highLimReg);//XX: IN1 high limit
      registers[9]= i2c_smbus_read_byte_data(fd,  lowLimReg);//XX: IN1 low limit
      
      then compare the array contents to the desired, change them if not.

      Insetead of that, we are just writing the registers, without knowing what 
      they have originally.
      ###########################################################################*/
      res= i2c_smbus_write_byte_data(fd,  0x00, 0x00);
      res= i2c_smbus_write_byte_data(fd,  0x07, 0x01);
      res= i2c_smbus_write_byte_data(fd,  0x0B, 0x02);
      usleep(50000); // 50 ms
      res= i2c_smbus_write_byte_data(fd,  0x00, 0x01);
      /*##########################################################################
      In case you need to write the limit registers.
      //res= i2c_smbus_write_byte_data(fd,  0x2C, 0xFA);
      //res= i2c_smbus_write_byte_data(fd,  0x2E, 0xFA);
      ###########################################################################*/
      usleep(50000); //50 ms
      //Now we read the data
      res2= i2c_smbus_read_word_data(fd,  chanReadReg);
      res3=res2&0x00FF;
      res4=res2>>12;
      res5=(res3<<4)|res4;
      printf ("Voltage: %f [V?]\n",(res5/4096.0)*2.56);
      //Must close the communication:
      close(fd);
    }
}




/*
    The I2C address 
      1D ->  ADC on main board
      1E -> U33 on meter board A1 en mid, A0 en low, 2D.
      2D -> U38 on meter board   
      U33 A0 high A1 LOW ----> 1F!! WORKS
      U38 A0 high  A1 HIGH ---> 37 !?

time out

valid answer
*/

