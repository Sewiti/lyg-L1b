#include <string>
#include <vector>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#include <omp.h>
#endif

struct Data {
    std::string Name;
    int Age = 0;
    double Salary = 0;
    std::string Computed;
    bool Valid = true;
};

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

class ResultsBuffer {
private:
    std::vector<Data> container;
public:
    void addItemSorted(const Data &item);

    std::vector<Data> getItems();
};