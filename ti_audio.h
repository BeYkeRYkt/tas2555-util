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
**     ti_audio.h
**
** Description:
**     header file for the test program of TAS2555 Android Linux drivers
**
** =============================================================================
*/
#include <stdlib.h>
#include <string.h>

#define	TIAUDIO_VERSION		"Version : 3.1 (20th, May, 2016)"
#define	TI_AUDIO_NAME		"/dev/tas2555"

#define MAX_INT_STR	"4294967295"

#define	SR_STR_48K	"48000"
#define	SR_STR_44K	"44100"
#define	SR_STR_16K	"16000"
#define	SR_NUM_48K	48000
#define	SR_NUM_44K	44100
#define	SR_NUM_16K	16000

#define	FW_NAME_SIZE			64
#define	FW_DESCRIPTION_SIZE		256
#define	PROGRAM_BUF_SIZE 		(2 + FW_NAME_SIZE + FW_DESCRIPTION_SIZE)
#define	CONFIGURATION_BUF_SIZE 	(8 + FW_NAME_SIZE + FW_DESCRIPTION_SIZE)

#define	TIAUDIO_CMD_REG_WITE			1
#define	TIAUDIO_CMD_REG_READ			2
#define	TIAUDIO_CMD_DEBUG_ON			3
#define	TIAUDIO_CMD_PROGRAM				4
#define	TIAUDIO_CMD_CONFIGURATION		5
#define	TIAUDIO_CMD_FW_TIMESTAMP		6
#define	TIAUDIO_CMD_CALIBRATION			7
#define	TIAUDIO_CMD_SAMPLERATE			8
#define	TIAUDIO_CMD_BITRATE				9
#define	TIAUDIO_CMD_DACVOLUME			10
#define	TIAUDIO_CMD_SPEAKER				11
#define	TIAUDIO_CMD_FW_RELOAD			12

#define TAS2555_REG(book, page, reg)		(((book * 256 * 128) + \
						 (page * 128)) + reg)

#define TAS2555_BOOK_ID(reg)			(reg / (256 * 128))
#define TAS2555_PAGE_ID(reg)			((reg % (256 * 128)) / 128)
#define TAS2555_BOOK_REG(reg)			(reg % (256 * 128))
#define TAS2555_PAGE_REG(reg)			((reg % (256 * 128)) % 128)

#define ARRAY_LEN(x) ((int)(sizeof(x)/sizeof((x)[0])))
