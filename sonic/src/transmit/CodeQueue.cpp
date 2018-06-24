#include <math.h>
#include <assert.h>
#include <deque>
#include <algorithm>
#include <iterator>

#include "freq_util/bb_freq_util.h"
#include "queue/queue.h"
#include "CodeQueue.h"




//#define SONIC_DEBUG

#ifdef SONIC_DEBUG
#define DBG(x)  x
#else
#define DBG(x)
#endif // SONIC_DEBUG



#define QUEUE_COUNT     32
#define QUEUE_LENGTH    20


struct CodeResult
{
    std::vector<int> res;
    std::vector<int> rrr;
};

struct CCodeQueue::Impl
{
    queue savedBuffer[QUEUE_COUNT];
    std::deque<CodeResult> resultQueue;
};


CCodeQueue::CCodeQueue()
{
    mImpl = new Impl;

    for (int i = 0; i < QUEUE_COUNT; ++i) {
        init_queue(&mImpl->savedBuffer[i], QUEUE_LENGTH);
    }
}

CCodeQueue::~CCodeQueue()
{
    delete mImpl;
}

inline void dump_queue(queue* q17, queue* q19)
{
    for (int i = 0; i < QUEUE_LENGTH; ++i) {
        printf("[%02d] %f %f\n", i, queue_item_at_index(q17, i), queue_item_at_index(q19, i));
    }
}

bool resultWithTimeSlice(queue* savedBuffer, std::vector<int>& res, std::vector<int>& rrr)
{
    queue *q17 = &savedBuffer[17];
    queue *q19 = &savedBuffer[19];

    if (!(queue_item_at_index(q17, 0) > 0.0
            && queue_item_at_index(q19, 1) > 0.0)) {
        return false;
    }

    float value1 = queue_item_at_index(q17, 0) + queue_item_at_index(q19, 1);
    float value2 = queue_item_at_index(q17, 1) + queue_item_at_index(q19, 2);
    if (value1 < value2) {
        return false;
    }

    float minValue = fmin(queue_item_at_index(q17, 2), queue_item_at_index(q19, 3));
    minValue = fmax(minValue, queue_item_at_index(q17, 0) * 0.7);
    float maxValue = fmax(queue_item_at_index(q17, 0), queue_item_at_index(q19, 1)) * 1.85;
    DBG(printf("minValue(%f) maxValue(%f)\n", minValue, maxValue));
    //printf("\n================= start:(19[0]=%f), (17[2]=%f), (19[3]=%f), (17[0]*0.7=%f), (minValue=%f) ==================\n\n",
    //    queue_item_at_index(q19, 0), queue_item_at_index(q17, 2), queue_item_at_index(q19, 3), queue_item_at_index(q17, 0) * 0.7, minValue);
    DBG(dump_queue(q17, q19));

    //if (minValue > maxValue || minValue < 0.4)
    //{
    //    return false;
    //}

    res.resize(QUEUE_LENGTH);
    rrr.resize(QUEUE_LENGTH);
    generate_data(savedBuffer, QUEUE_COUNT, &res[0], &rrr[0], QUEUE_LENGTH, minValue, maxValue);

#if 0
    printf("res:\n");
    for (int i=0; i<(int)res.size(); i++) 
    {
        printf("%02d ", res[i]);
    }
    printf("\n");
#endif

    // swap rrr
    std::reverse(rrr.begin(), rrr.end());

#if 0
    printf("rrr:\n");
    for (int i=0; i<(int)rrr.size(); i++) 
    {
        printf("%02d ", rrr[i]);
    }
    printf("\n\n");
#endif

    return true;
}

bool CCodeQueue::putFreqValues(std::vector<double> const& freqVaules)
{
    assert(freqVaules.size() == QUEUE_COUNT);

    for (int i = 0; i < (int)freqVaules.size(); ++i) {
        enqueue(&mImpl->savedBuffer[i], freqVaules[i]);
    }

    DBG(printf("mImpl->savedBuffer[0].count=%d\n", mImpl->savedBuffer[0].count));
    if (mImpl->savedBuffer[0].count >= mImpl->savedBuffer[0].length) {
#if 0
        for (int i = 0; i < QUEUE_COUNT; ++i)
        {
            print_queue(&mImpl->savedBuffer[i]);
        }
#endif

        std::vector<int> res, rrr;
        if (resultWithTimeSlice(mImpl->savedBuffer, res, rrr)) {
            //printf("res size(%d), rrr size(%d)\n", res.size(), rrr.size());
            mImpl->resultQueue.push_back(CodeResult());
            CodeResult& back = mImpl->resultQueue.back();
            std::copy(&res[2], &res.back() + 1, std::back_inserter(back.res));
            std::copy(&rrr[2], &rrr.back() + 1, std::back_inserter(back.rrr));
            //printf("back res size(%d), rrr size(%d)\n", back.res.size(), back.rrr.size());
        }
    }

    return true;
}

bool CCodeQueue::getResult(std::vector<int>& res, std::vector<int>& rrr)
{
    if (!mImpl->resultQueue.empty()) {
        DBG(printf("not empty\n"));
        CodeResult& front = mImpl->resultQueue.front();
        res.swap(front.res);
        rrr.swap(front.rrr);
        mImpl->resultQueue.pop_front();

        return true;
    }

    return false;
}

bool CCodeQueue::clearQueue()
{
    DBG(printf("clear_all_queue\n"));

    for (int i = 0; i < QUEUE_COUNT; ++i) {
        while (!queue_is_empty(&mImpl->savedBuffer[i])) {
            dequeue(&mImpl->savedBuffer[i]);
        }
    }

    return true;
}


