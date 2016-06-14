/*
** =============================================================================
** Copyright (c) 2016  Texas Instruments Inc.
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the Free Software 
** Foundation; version 2.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
** Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
** File:
**     main.c
**
** Description:
**     test program for TAS2555 Android Linux drivers
**
** =============================================================================
*/

#include <sys/cdefs.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include "ti_audio.h"

static void usage(){

	fprintf(stderr, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
		"usage: ti_audio -r book page register [count]               (read, hexdecimal)",
		"       ti_audio -w book page register value1 [value2 ...]  (write, hexdecimal)",
		"       ti_audio -d on                            (turn on the debug msg, bool)",
		"       ti_audio -p [n]                        (get/[set] DSP program, decimal)",
		"       ti_audio -c [n]                  (get/[set] DSP configuration, decimal)",
		"       ti_audio -s [n]                        (get/[set] sample rate, decimal)",
		"       ti_audio -b [n]                           (get/[set] bit rate, decimal)",
		"       ti_audio -v [n]                             (get/[set] volume, decimal)",
		"       ti_audio -f                                   (trigger firmware reload)",		
		"       ti_audio -o on                              (turn on/off TAS2555, bool)",
		"       ti_audio -t                                    (get firmware timestamp)",
		TIAUDIO_VERSION);	
}

static int str2hexchar(char *argv, unsigned char *pUInt8_Val){
	int str_len = strlen(argv), i, result = -1, j;
	unsigned char val[2] = {0};

	if(str_len > 2){
		fprintf(stderr, "invalid parameters\n");
		goto err;
	}

	for(i = (str_len-1), j=0; i >= 0; i--, j++){
		if((argv[i] <= '9')&&(argv[i] >= '0')){
			val[j] = argv[i] - 0x30;
		}else if((argv[i] <='f')&&(argv[i]>= 'a')){
			val[j] = argv[i] - 0x57;
		}else if((argv[i] <='F')&&(argv[i]>= 'A')){
			val[j] = argv[i] - 0x37;
		}else{
			fprintf(stderr, "reg/data out of range\n");
			goto err;
		}
	}
	
	*pUInt8_Val = (unsigned char)(val[0]|(val[1]<<4));
	result = 0;
	
err:
	return result;
}

/*
static int str2hexshort(char *argv, unsigned short *pUInt16_Val){
	int str_len = strlen(argv), i, j, result = -1;
	unsigned char val[4] = {0};

	if(str_len > 4){
		fprintf(stderr, "invalid parameters\n");
		goto err;
	}

	for(i = (str_len-1), j=0; i >= 0; i--, j++){
		if((argv[i] <= '9')&&(argv[i] >= '0')){
			val[j] = argv[i] - 0x30;
		}else if((argv[i] <='f')&&(argv[i]>= 'a')){
			val[j] = argv[i] - 0x57;
		}else if((argv[i] <='F')&&(argv[i]>= 'A')){
			val[j] = argv[i] - 0x37;
		}else{
			fprintf(stderr, "reg/data out of range\n");
			goto err;
		}
	}
	
	*pUInt16_Val = (unsigned short)(val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12));
	result = 0;

err:
	return result;
}
*/

static int str2decimal(char *argv, unsigned int *pUInt32_Val){
	int max_len = strlen(MAX_INT_STR), i, result = -1, j;
	int str_len = strlen(argv);
	unsigned int nValue = 0;
	unsigned char temp;

	if(str_len > max_len){
		fprintf(stderr, "invalid parameters\n");
		goto err;
	}
	
	for(i = (str_len-1), j=0; i >= 0; i--, j++){
		if((argv[i] <= '9')&&(argv[i] >= '0')){
			temp = argv[i] - 0x30;
			nValue += (temp * pow(10, j));
		}else{
			fprintf(stderr, "reg/data out of range\n");
			goto err;
		}
	}
	
	*pUInt32_Val = nValue;
	result = 0;
	
err:
	return result;
}

static int TiAudio_Reg_Write(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char *pBuff = NULL, book, page, reg,value;
	unsigned int whole_reg = 0;
	unsigned int temp_reg = 0;
	int i=0, reg_count = 0;
	
	if(argc < 6){
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	reg_count = argc - 5;

	pBuff = (unsigned char *)malloc(reg_count + 5);
	if(pBuff == NULL){
		fprintf(stderr, "not enough mem\n");
		goto err;		
	}
	
	pBuff[0] = TIAUDIO_CMD_REG_WITE;
	
	err = str2hexchar(argv[2], &book);
	if(err < 0){
		goto err;
	}
	
	err = str2hexchar(argv[3], &page);
	if(err < 0){
		goto err;
	}
	
	err = str2hexchar(argv[4], &reg);
	if(err < 0){
		goto err;
	}
	
	whole_reg = TAS2555_REG(book, page, reg);
	pBuff[1] = (whole_reg & 0xff000000) >> 24;
	pBuff[2] = (whole_reg & 0x00ff0000) >> 16;
	pBuff[3] = (whole_reg & 0x0000ff00) >> 8;
	pBuff[4] = (whole_reg & 0x000000ff) ;
	
	for(i=0; i< reg_count; i++){
		err = str2hexchar(argv[i+5], &value);
		if(err < 0){
			goto err;
		}
		pBuff[i + 5] = value;
	}
	
	err = write(fileHandle, pBuff, reg_count+5);
	if(err != (reg_count+5)){
		fprintf(stderr, "write err=%d\n", err);
	}else{
		for(i=0; i< reg_count; i++){
			temp_reg = whole_reg + i;
			fprintf(stderr, "W B[%d]P[%d]R[%d]=0x%x\n", TAS2555_BOOK_ID(temp_reg), TAS2555_PAGE_ID(temp_reg), TAS2555_PAGE_REG(temp_reg), pBuff[i + 5] );
		}
	}	

err:
	if(pBuff != NULL)
		free(pBuff);	
	
	return err;
}

static int TiAudio_Reg_Read(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char *pBuff = NULL, book, page, reg, len;
	unsigned int whole_reg = 0;
	unsigned int temp_reg = 0;
	int i=0, reg_count = 0;
	
	if((argc != 5) &&(argc != 6))  {
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	if(argc == 5)
		reg_count = 1;
	else{
		err = str2hexchar(argv[5], &len);
		if(err < 0){
			goto err;
		}
		reg_count = len;
	}
	
	pBuff = (unsigned char *)malloc(reg_count + 5);
	if(pBuff == NULL){
		fprintf(stderr, "not enough mem\n");
		goto err;		
	}
	
	pBuff[0] = TIAUDIO_CMD_REG_READ;
	
	err = str2hexchar(argv[2], &book);
	if(err < 0){
		goto err;
	}
	
	err = str2hexchar(argv[3], &page);
	if(err < 0){
		goto err;
	}
	
	err = str2hexchar(argv[4], &reg);
	if(err < 0){
		goto err;
	}
	
	whole_reg = TAS2555_REG(book, page, reg);
	pBuff[1] = (whole_reg & 0xff000000) >> 24;
	pBuff[2] = (whole_reg & 0x00ff0000) >> 16;
	pBuff[3] = (whole_reg & 0x0000ff00) >> 8;
	pBuff[4] = (whole_reg & 0x000000ff) ;
		
	err = write(fileHandle, pBuff, 5);
	if(err != 5){
		fprintf(stderr, "read err=%d\n", err);
		goto err;
	}
	
	err = read(fileHandle, pBuff, reg_count);
	if(err != reg_count){
		fprintf(stderr, "read err=%d\n", err);
		goto err;
	}else{
		for(i=0; i< reg_count; i++){
			temp_reg = whole_reg + i;
			fprintf(stderr, "R B[%d]P[%d]R[%d]=0x%x\n", TAS2555_BOOK_ID(temp_reg), TAS2555_PAGE_ID(temp_reg), TAS2555_PAGE_REG(temp_reg), pBuff[i]);
		}
	}	

err:
	if(pBuff != NULL)
		free(pBuff);	
	
	return err;
}

static int TiAudio_Debug_On(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[2], on;
	
	if(argc != 3) {
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_DEBUG_ON;
	
	err = str2hexchar(argv[2], &on);
	if(err < 0){
		goto err;
	}
	
	pBuff[1] = on; 
	
	err = write(fileHandle, pBuff, 2);
	if(err != 2){
		fprintf(stderr, "set err=%d\n", err);
		goto err;
	}
	
	if(on == 0){
		fprintf(stderr, "DBG msg Off\n");
	}else{
		fprintf(stderr, "DBG msg On\n");
	}
	
err:

	return err;
}

static int TiAudio_Program(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[PROGRAM_BUF_SIZE], bSet = 0;
	unsigned int nProgram;
	
	if(argc == 2){
		bSet = 0;
	}else if(argc == 3){
		bSet = 1;
	}else{
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_PROGRAM;
	
	if(bSet == 1){
		err = str2decimal(argv[2], &nProgram);
		if(err < 0){
			fprintf(stderr, "invalid para numbers\n");		
			goto err;
		}
		
		pBuff[1] = nProgram;
	}
	
	err = write(fileHandle, pBuff, (1+bSet));
	if(err != (bSet+1)){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	if(bSet == 1){
		fprintf(stderr, "Program Set to %d\n", nProgram);
	}else{
		err = read(fileHandle, pBuff, PROGRAM_BUF_SIZE);
		if(err != PROGRAM_BUF_SIZE){
			fprintf(stderr, "read err=%d\n", err);
			goto err;
		}else{
			unsigned char nPrograms = pBuff[0];
			unsigned char nCurProgram = pBuff[1];
			unsigned char *pName = &pBuff[2];
			unsigned char *pDescription = &pBuff[2 + FW_NAME_SIZE];
			fprintf(stderr, "Total Programs   : %d\n", nPrograms);
			fprintf(stderr, "Current Programs : %d\n", nCurProgram);
			fprintf(stderr, "\t Name: %s\n", pName);
			fprintf(stderr, "\t Description : %s\n", pDescription);
		}
	}

err:

	return err;
}

static int TiAudio_Configuration(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[CONFIGURATION_BUF_SIZE], bSet = 0;
	unsigned int nConfiguration;
	
	if(argc == 2){
		bSet = 0;
	}else if(argc == 3){
		bSet = 1;
	}else{
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_CONFIGURATION;
	
	if(bSet == 1){
		err = str2decimal(argv[2], &nConfiguration);
		if(err < 0){
			goto err;
		}
		
		pBuff[1] = nConfiguration;
	}
	
	err = write(fileHandle, pBuff, (1+bSet));
	if(err != (bSet+1)){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	if(bSet == 1){
		fprintf(stderr, "Configuration Set to %d\n", nConfiguration);
	}else{
		err = read(fileHandle, pBuff, CONFIGURATION_BUF_SIZE);
		if(err != CONFIGURATION_BUF_SIZE){
			fprintf(stderr, "read err=%d\n", err);
			goto err;
		}else{
			unsigned char nConfigurations = pBuff[0];
			unsigned char nCurConfiguration = pBuff[1];
			unsigned char *pName = &pBuff[2];
			unsigned char nProgram = pBuff[2 + FW_NAME_SIZE];
			unsigned char nPLL = pBuff[3 + FW_NAME_SIZE];
			unsigned int nSampleRate = pBuff[4 + FW_NAME_SIZE] + 
				((unsigned int)pBuff[5 + FW_NAME_SIZE] << 8) +
				((unsigned int)pBuff[6 + FW_NAME_SIZE] << 16) +
				((unsigned int)pBuff[7 + FW_NAME_SIZE] << 24);			
			unsigned char *pDescription = &pBuff[8 + FW_NAME_SIZE];
			fprintf(stderr, "Total Configurations : %d\n", nConfigurations);
			fprintf(stderr, "Current Configuration: %d\n", nCurConfiguration);
			fprintf(stderr, "\t Name: %s\n", pName);
			fprintf(stderr, "\t Description : %s\n", pDescription);
			fprintf(stderr, "\t nProgram: %d\n", nProgram);
			fprintf(stderr, "\t nPLL: %d\n", nPLL);
			fprintf(stderr, "\t nSampleRate: %d\n", nSampleRate);
		}
	}

err:

	return err;
}

static int TiAudio_SampleRate(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[5];
	unsigned char bSet = 0;
	unsigned int nSampleRate;
	int nLen;
	
	if(argc == 2){
		bSet = 0;
	}else if(argc == 3){
		bSet = 1;
	}else{
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_SAMPLERATE;
	
	if(bSet == 1){
		if(strcmp(argv[2], SR_STR_48K) == 0)
			nSampleRate = SR_NUM_48K;
		else if(strcmp(argv[2], SR_STR_44K) == 0)
			nSampleRate = SR_NUM_44K;
		else if(strcmp(argv[2], SR_STR_16K) == 0)
			nSampleRate = SR_NUM_16K;
		else{
			fprintf(stderr, "invalid para numbers\n");
			goto err;
		}
		
		pBuff[1] = (nSampleRate&0xff000000)>>24;
		pBuff[2] = (nSampleRate&0x00ff0000)>>16;
		pBuff[3] = (nSampleRate&0x0000ff00)>>8;
		pBuff[4] = (nSampleRate&0x000000ff);
	}
	
	if(bSet) nLen = 5; 
	else nLen = 1;
	
	err = write(fileHandle, pBuff, nLen);
	if(err != nLen){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	if(bSet == 1){
		fprintf(stderr, "Sample Rate Set to %d\n", nSampleRate);
	}else{
		err = read(fileHandle, pBuff, 4);
		if(err != 4){
			fprintf(stderr, "read err=%d\n", err);
			goto err;
		}else{
			nSampleRate = pBuff[0] + 
				((unsigned int)pBuff[1] << 8) +
				((unsigned int)pBuff[2] << 16) +
				((unsigned int)pBuff[3] << 24);			
			fprintf(stderr, "\t nSampleRate: %d\n", nSampleRate);
		}
	}

err:

	return err;
}

static int TiAudio_BitRate(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[5];
	unsigned char bSet = 0;
	unsigned int nBitRate;
	
	if(argc == 2){
		bSet = 0;
	}else if(argc == 3){
		bSet = 1;
	}else{
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_BITRATE;
	
	if(bSet == 1){
		if(strcmp(argv[2], "32") == 0)
			nBitRate = 32;
		else if(strcmp(argv[2], "24") == 0)
			nBitRate = 24;
		else if(strcmp(argv[2], "20") == 0)
			nBitRate = 20;
		else if(strcmp(argv[2], "16") == 0)
			nBitRate = 16;			
		else{
			fprintf(stderr, "invalid para numbers\n");
			goto err;
		}
		
		pBuff[1] = nBitRate;
	}
	
	err = write(fileHandle, pBuff, (1+bSet));
	if(err != (1+bSet)){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	if(bSet == 1){
		fprintf(stderr, "BitRate Set to %d\n", nBitRate);
	}else{
		err = read(fileHandle, pBuff, 1);
		if(err != 1){
			fprintf(stderr, "read err=%d\n", err);
			goto err;
		}else{
			nBitRate = pBuff[0];
			fprintf(stderr, "\t BitRate: %d\n", nBitRate);
		}
	}

err:

	return err;
}

static int TiAudio_DACVolume(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[5];
	unsigned char bSet = 0;
	unsigned int nVol;
	
	if(argc == 2){
		bSet = 0;
	}else if(argc == 3){
		bSet = 1;
	}else{
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_DACVOLUME;
	
	if(bSet == 1){
		err = str2decimal(argv[2], &nVol);
		if(err < 0){
			goto err;
		}
		
		pBuff[1] = nVol;
	}
	
	err = write(fileHandle, pBuff, (1+bSet));
	if(err != (1+bSet)){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	if(bSet == 1){
		fprintf(stderr, "DAC Volume Set to %d\n", nVol);
	}else{
		err = read(fileHandle, pBuff, 1);
		if(err != 1){
			fprintf(stderr, "read err=%d\n", err);
			goto err;
		}else{
			nVol = pBuff[0];		
			fprintf(stderr, "\t DAC Volume: %d\n", nVol);
		}
	}

err:

	return err;
}

static int TiAudio_SpeakerOn(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[2], on;
	
	if(argc != 3) {
		fprintf(stderr, "invalid para numbers\n");
		goto err;
	}
	
	pBuff[0] = TIAUDIO_CMD_SPEAKER;
	
	err = str2hexchar(argv[2], &on);
	if(err < 0){
		goto err;
	}
	
	pBuff[1] = on; 
	
	err = write(fileHandle, pBuff, 2);
	if(err != 2){
		fprintf(stderr, "set err=%d\n", err);
		goto err;
	}
	
	if(on == 0){
		fprintf(stderr, "TAS2555 Power Off\n");
	}else{
		fprintf(stderr, "TAS2555 Power On\n");
	}
	
err:

	return err;
}

static int TiAudio_Timestamp(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[4];
	
	pBuff[0] = TIAUDIO_CMD_FW_TIMESTAMP;
	
	err = write(fileHandle, pBuff, 1);
	if(err != 1){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	err = read(fileHandle, pBuff, 4);
	if(err != 4){
		fprintf(stderr, "read err=%d\n", err);
		goto err;
	}else{
		unsigned int nTimestamp = pBuff[0] + 
			((unsigned int)pBuff[1] << 8) +
			((unsigned int)pBuff[2] << 16) +
			((unsigned int)pBuff[3] << 24);
	
		time_t t = (time_t)nTimestamp;  
		struct tm *p;  
		p=localtime(&t);  
		char s[100];  
		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", p);  
		fprintf(stderr, "FW Timestamp : %d: %s\n", (int)t, s); 
	}

err:

	return err;
}

static int TiAudio_TriggerFWReload(int fileHandle, int argc, char **argv){
	int err = -1;
	unsigned char pBuff[4];	
	pBuff[0] = TIAUDIO_CMD_FW_RELOAD;
	
	err = write(fileHandle, pBuff, 1);
	if(err != 1){
		fprintf(stderr, "write err=%d\n", err);
		goto err;
	}
	
	fprintf(stderr, "Firmware Reload Triggered\n");	
	
err:

	return err;
}

static int getDevHandle(){
	int fileHandle = -1;
	fileHandle = open(TI_AUDIO_NAME, O_RDWR);
	if(fileHandle < 0 ){
		fprintf(stderr, "[ERROR]file(%s) open_RDWR error\n", TI_AUDIO_NAME);
	}		

	return fileHandle;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int ch;
	int fileHandle = -1;

	fileHandle = getDevHandle();
	if(fileHandle < 0 ){
		fprintf(stderr, " file handle err=%d\n", fileHandle);
		return ret;
	}
	
	if(argc == 1){
		usage();
		return 0;
	}

	while ((ch = getopt(argc, argv, "wrdpcsbvotf")) != -1) {
		switch (ch) {
		case 'w': 
			ret = TiAudio_Reg_Write(fileHandle, argc, argv);			
			break;
		case 'r':
			ret = TiAudio_Reg_Read(fileHandle, argc, argv);
			break;
		case 'd':
			ret = TiAudio_Debug_On(fileHandle, argc, argv);
			break;		
		case 'p':
			ret = TiAudio_Program(fileHandle, argc, argv);
			break;					
		case 'c':
			ret = TiAudio_Configuration(fileHandle, argc, argv);
			break;		
		case 's':
			ret = TiAudio_SampleRate(fileHandle, argc, argv);
			break;		
		case 'b':
			ret = TiAudio_BitRate(fileHandle, argc, argv);
			break;
		case 'v':
			ret = TiAudio_DACVolume(fileHandle, argc, argv);
			break;		
		case 'o':
			ret = TiAudio_SpeakerOn(fileHandle, argc, argv);
			break;				
		case 't':
			ret = TiAudio_Timestamp(fileHandle, argc, argv);
			break;		
		case 'f':
			ret = TiAudio_TriggerFWReload(fileHandle, argc, argv);
			break;
		default:
			usage();
			break;
		}
	}

	if(fileHandle > 0 )
		close(fileHandle);
	
	return ret;
}
