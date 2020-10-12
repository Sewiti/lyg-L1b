#include "DataBuffer.h"


DataBuffer::DataBuffer(unsigned int maxBufferSize) {
    maxSize = maxBufferSize;
}

void DataBuffer::addItem(const Data &item) {
    bool added = false;
    while (!added) {
#pragma omp critical(data_critical)
        {
            if (!isFull()) {
                container.push_back(item);
                added = true;
#ifdef DEBUG
                std::cout << omp_get_thread_num() << ": Added item to data\n";
#endif
            }
        }
    }
}

Data DataBuffer::removeItem() {
    Data item;
    bool removed = false;

    while (!removed) {
        if (finished && isEmpty()) {
#ifdef DEBUG
            std::cout << omp_get_thread_num() << ": Thread exit\n";
#endif
            Data data;
            data.Valid = false;
            return data;
        }

#pragma omp critical(data_critical)
        {
            if (!isEmpty()) {
                item = container.at(0);
                container.erase(container.begin());
                removed = true;
#ifdef DEBUG
                std::cout << omp_get_thread_num() << ": Removed item\n";
#endif
            }
        }
    }

    return item;
}

bool DataBuffer::isFull() {
    return container.size() >= maxSize;
}

bool DataBuffer::isEmpty() {
    return container.empty();
}

void DataBuffer::setFinished() {
    finished = true;
#ifdef DEBUG
    std::cout << omp_get_thread_num() << ": Finished adding\n";
#endif
}