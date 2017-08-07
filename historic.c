/*
 * historic.c
 * 
 * Copyright 2017 christian <christian@coolermaster>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circularBuffer.h"
#include "historic.h"

//~ char* getStr(char * path, char out_buf[], int size) {
	//~ FILE * f = fopen(path, "rt");
	//~ if(f!=NULL) {
		//~ size_t readSize = fread(out_buf, 1, size-1, f);
		//~ out_buf[readSize+1]='\0';
		//~ fclose(f);
		//~ return out_buf;
	//~ }
	//~ return NULL;
//~ }

char* getStr(char * path, char out_buf[], int size) {
	FILE * f = fopen(path, "rt");
	if(f!=NULL) {
		fgets(out_buf, size-1, f);
		out_buf[size-1]='\0';
		fclose(f);
	}
	else
	{
		out_buf[0]='\0';
	}
	return out_buf;
}

float getFloat(char * path) {
	char buff[50];
	if(getStr(path, buff, sizeof(buff))!=NULL) {
		return atof(buff);
	}
	return 0;
}

int getInt(char * path) {
	char buff[50];
	if(getStr(path, buff, sizeof(buff))!=NULL) {
		return atoi(buff);
	}
	return 0;
}

tData * retrieveDynamicData(tData *data) {
	data->timestamp = time(NULL);
	data->bmp280_pressure    = getFloat("/sys/bus/i2c/devices/i2c-1/1-0076/iio:device1/in_pressure_input");
	data->bmp280_temperature = getInt(  "/sys/bus/i2c/devices/i2c-1/1-0076/iio:device1/in_temp_input");
	data->htu21_temperature  = getInt(  "/sys/bus/i2c/devices/i2c-1/1-0040/iio:device0/in_temp_input");
	data->htu21_humidity     = getInt(  "/sys/bus/i2c/devices/i2c-1/1-0040/iio:device0/in_humidityrelative_input");	
	return data;
}

void reduceHistoric(tHistoricItem historics[], int nbHistorics) {
	for(int i = 0; i < nbHistorics; i++) {
		if(queue_getNbItems(&historics[i].circularBuffer) > historics[i].limit) {
			double fields[5] = {0};
			tData writeData;
			int nbElementsToSum = historics[i].limit / 2;
			//printf("Reduction queue %d nbElementsToSum = %d\n", i, nbElementsToSum);
			for(int j=0; j < nbElementsToSum; j++)
			{
				tData readData;
				if(queue_getItem(&historics[i].circularBuffer, &readData)) {
					fields[0] += readData.timestamp;
					fields[1] += readData.bmp280_pressure;
					fields[2] += readData.bmp280_temperature;
					fields[3] += readData.htu21_temperature;
					fields[4] += readData.htu21_humidity;
				}
			}
			writeData.timestamp          = (time_t)(fields[0] / nbElementsToSum);
			writeData.bmp280_pressure    = (float) (fields[1] / nbElementsToSum);
			writeData.bmp280_temperature = (int)   (fields[2] / nbElementsToSum);
			writeData.htu21_temperature  = (int)   (fields[3] / nbElementsToSum);
			writeData.htu21_humidity     = (int)   (fields[4] / nbElementsToSum);
			//{
				//char json[240];
				//char *pJson = json;
				//int remainingSize = sizeof(json);
				//json[0]= '\0';
				//printf("Reducted value : %s\n", tData_json(&writeData, &pJson, &remainingSize));
			//}
			if(i+1<nbHistorics) {
				queue_putItem(&historics[i+1].circularBuffer, &writeData);
			}
		}
		else {
			//printf("No value to reduce\n");
			break;
		}
	}
}
