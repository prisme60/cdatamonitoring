#ifndef __HISTORIC_H__
#define __HISTORIC_H__

#include "circularBuffer.h"

typedef struct {
	circularQueue_t circularBuffer;
	int limit;
} tHistoricItem;

tData * retrieveDynamicData(tData *data);

void reduceHistoric(tHistoricItem historics[], int nbHistorics);

#endif // __HISTORIC_H__
