#pragma once
#include "Data.h"
#include <vector>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#include <omp.h>
#endif

class DataBuffer {
private:
    bool finished = false;
    unsigned int maxSize;
    std::vector<Data> container;
public:
    explicit DataBuffer(unsigned maxBufferSize);

    void addItem(const Data &item);

    Data removeItem();

    bool isFull();

    bool isEmpty();

    void setFinished();
};