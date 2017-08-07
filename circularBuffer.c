/*
 * circularBuffer.c
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
#include <stdbool.h>
#include <time.h>

#include "circularBuffer.h"

#define SEPARATOR_JSON_FIELD ",\n"

void queue_init(circularQueue_t *theQueue, tData data[], int maxItems)
{
    memset(theQueue, 0, sizeof(circularQueue_t));
    memset(data, 0, maxItems * sizeof(tData));
    theQueue->data = data;
    theQueue->maxItems = maxItems;
}

bool queue_isEmpty(circularQueue_t *theQueue)
{
    return theQueue->validItems==0;
}

int queue_getNbItems(circularQueue_t *theQueue)
{
    return theQueue->validItems;
}

bool queue_putItem(circularQueue_t *theQueue, const tData *theItemValue)
{
    if(theQueue->validItems>=theQueue->maxItems)
    {
        printf("The queue is full\n");
        printf("You cannot add items\n");
        return false;
    }
    else
    {
        theQueue->validItems++;
        theQueue->data[theQueue->last] = *theItemValue;
        theQueue->last = (theQueue->last+1)%theQueue->maxItems;
        return true;
    }
}

bool queue_getItem(circularQueue_t *theQueue, tData *theItemValue)
{
    if(queue_isEmpty(theQueue))
    {
        printf("isempty\n");
        return false;
    }
    else
    {
        *theItemValue=theQueue->data[theQueue->first];
        theQueue->first=(theQueue->first+1)%theQueue->maxItems;
        theQueue->validItems--;
        return true;
    }
}

bool queue_peekItem(circularQueue_t *theQueue, int index, tData *theItemValue) {
	if(index < theQueue->validItems) {
		int internalIndex = (theQueue->first + index) % theQueue->maxItems;
		*theItemValue = theQueue->data[internalIndex];
		return true;
	}
	return false;
}

void queue_print(circularQueue_t *theQueue)
{
    for(int aux = theQueue->first, aux1=theQueue->validItems; aux1>0; aux=(aux+1)%theQueue->maxItems, aux1--) {
        printf("Element #%d timestamp = %ld\n", aux, theQueue->data[aux].timestamp);
        printf("Element #%d pressure  = %f\n", aux, theQueue->data[aux].bmp280_pressure);
        printf("Element #%d bmp280Temp= %d\n", aux, theQueue->data[aux].bmp280_temperature);
        printf("Element #%d htu21Temp = %d\n", aux, theQueue->data[aux].htu21_temperature);
        printf("Element #%d humidity  = %d\n", aux, theQueue->data[aux].htu21_humidity);
    }
}

char * queue_json(circularQueue_t *theQueue, char *buffer, int bufSize)
{
	int remainingSize = bufSize;
	char *pBufferStart = buffer;
	bool firstSeparator = true;
	if(remainingSize > 0) {
		strcat(pBufferStart, "[");
		pBufferStart++;
		remainingSize--;
	}
    for(int aux = theQueue->first, aux1=theQueue->validItems;
		aux1 > 0 && remainingSize > 0;
		aux=(aux+1)%theQueue->maxItems, aux1--) {
		
		if(!firstSeparator) {
			strcat(pBufferStart, SEPARATOR_JSON_FIELD);
			remainingSize -= sizeof(SEPARATOR_JSON_FIELD) - 1;
			pBufferStart += sizeof(SEPARATOR_JSON_FIELD) - 1;
		}
        tData_json(&(theQueue->data[aux]), &pBufferStart, &remainingSize);
        firstSeparator = false;
    }
    if(remainingSize > 0) {
		strcat(pBufferStart, "]");
		pBufferStart++;
		remainingSize--;
	}
	return remainingSize>0?buffer:NULL;
}

char * tData_json(tData * data, char* pBufferStart[], int *remainingSize) {
	char *pInitBuffer = *pBufferStart;
	int nbCharWritten = snprintf(*pBufferStart, *remainingSize,
		"{\"timestamp\": %ld,\n"
		"\"pressure\"  : %.2f,\n"
		"\"bmp280Temp\": %.3f,\n"
		"\"htu21Temp\" : %.3f,\n"
		"\"humidity\"  : %.2f}\n",
		data->timestamp,
		data->bmp280_pressure * 10,
		data->bmp280_temperature / 1000.0f,
		data->htu21_temperature / 1000.0f,
		data->htu21_humidity / 1000.0f
	);
	*remainingSize -= nbCharWritten;
	*pBufferStart += nbCharWritten;
	return *remainingSize>0?pInitBuffer:NULL;
}
