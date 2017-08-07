#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <stdbool.h>
#include <time.h>

typedef struct {
	time_t timestamp;
	float bmp280_pressure;
	int bmp280_temperature;
	int htu21_temperature;
	int htu21_humidity;
} tData;

typedef struct
{
    int     first;
    int     last;
    int     validItems;
    tData   *data;
    int     maxItems;
} circularQueue_t;

void queue_init(circularQueue_t *theQueue, tData data[], int maxItems);
bool queue_isEmpty(circularQueue_t *theQueue);
int  queue_getNbItems(circularQueue_t *theQueue);
bool queue_putItem(circularQueue_t *theQueue, const tData *theItemValue);
bool queue_getItem(circularQueue_t *theQueue, tData *theItemValue);
bool queue_peekItem(circularQueue_t *theQueue, int index, tData *theItemValue);
void queue_print(circularQueue_t *theQueue);
char * queue_json(circularQueue_t *theQueue, char *buffer, int bufSize);
char * tData_json(tData * data, char* pBufferStart[], int *remainingSize);

#endif // __CIRCULAR_BUFFER_H__
