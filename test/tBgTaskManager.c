/**
 * \file tBgTaskManager.c
 * \date Feb 01, 2017
 */

#include <servkit/bgtaskmanager.h>
#include "unittest.h"

#include <servkit/asserts.h>

#include <unistd.h>

#define NUM_THREADS 4

typedef struct
{
    int idx;
    int* pNumDone;
} consumerData;

static consumerData data[NUM_THREADS];
static int numInit = 0;
static int numDone = 0;

static
skBgConsumerDataPtr consumerDataCreate()
{
    int curr = numInit++;
    data[curr].idx = curr;
    data[curr].pNumDone = &numDone;
    return &data[curr];
}

static
void consumerDataDestroy()
{
}

static
void consumer(skBgConsumerDataPtr data, skBgConsumerTaskPtr task)
{
    consumerData* cdata = (consumerData*)data;
    int taskIdx = (int)(size_t)task;
    skDbgTraceF(SK_LVL_INFO, "Thread %d consumed task %d.", cdata->idx, taskIdx);
    (void)taskIdx;
    __sync_add_and_fetch(cdata->pNumDone, 1);
}

TEST_FUNC( CreateAndDestroy )
{
  skBgTaskManager mgr;
  numInit = 0;
  TEST_TRUE(skBgTaskManagerInit(&mgr, NUM_THREADS, consumerDataCreate,
                                consumerDataDestroy, consumer) == 0);
  TEST_TRUE(numInit == NUM_THREADS);
  TEST_TRUE(skBgTaskManagerDestroy(&mgr) == 0);
}

#define NUM_TASKS 1000

TEST_FUNC( TenTasks )
{
  skBgTaskManager mgr;
  numInit = 0;
  numDone = 0;
  TEST_TRUE(skBgTaskManagerInit(&mgr, NUM_THREADS, consumerDataCreate,
                                consumerDataDestroy, consumer) == 0);
  TEST_TRUE(numInit == NUM_THREADS);
  for (int i = 0; i < NUM_TASKS; ++i) {
      skBgTaskManagerAddTask(&mgr, (void*)(size_t)i);
  }
  while (numDone != NUM_TASKS) { }
  TEST_TRUE(skBgTaskManagerDestroy(&mgr) == 0);
}

void SetupTests(void)
{
    REG_TEST( CreateAndDestroy );
    REG_TEST( TenTasks );
}