#pragma once
#include "Data.h"
#include <vector>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#include <omp.h>
#endif

class ResultsBuffer {
private:
    std::vector<Data> container;
public:
    void addItemSorted(const Data &item);

    std::vector<Data> getItems();
};