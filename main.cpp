#include "Data.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <openssl/sha.h>
#include <string>
#include <vector>

using namespace std;


void executeFile(const string &dataFilename, const string &resultsFilename);

vector<Data> readData(const string &filename);

void putIntoBuffer(DataBuffer *dataBuffer, vector<Data> items);

void worker(DataBuffer *dataBuffer, ResultsBuffer *resultsBuffer);

string getHash(const Data &item);


int main() {
    for (int i = 1; i <= 3; i++) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_dat_%d.txt", i);
        string dataFilename = buffer;

        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_rez_%d.txt", i);
        string resultsFilename = buffer;


        cout << "Executing '" << dataFilename << "'...\n";

        executeFile(dataFilename, resultsFilename);
    }

    cout << "Done! Have a nice day:)\n";
    return 0;
}

void executeFile(const string &dataFilename, const string &resultsFilename) {
    vector<Data> items = readData(dataFilename);

    auto dataBuffer = new DataBuffer(8);
    auto resultsBuffer = new ResultsBuffer();

    unsigned int n = items.size() / 4;

#pragma omp parallel num_threads(n+1) default(none) shared(dataBuffer, resultsBuffer, items)
    {
        if (omp_get_thread_num() == 0) {
            // "Main" thread
            putIntoBuffer(dataBuffer, items);
        } else {
            // Worker threads
            worker(dataBuffer, resultsBuffer);
        }
    }

    // TODO: Print out results
}

vector<Data> readData(const string &filename) {
    ifstream file(filename);

    vector<Data> v;

    if (file.is_open()) {
        int n;

        file >> n;

        for (int i = 0; i < n; i++) {
            string firstname, lastname;

            Data d;
            file >> firstname >> lastname >> d.Age >> d.Salary;

            char buffer[30];
            snprintf(buffer, sizeof(buffer), "%s %s", firstname.c_str(), lastname.c_str());
            d.Name = buffer;

            v.push_back(d);
        }

        file.close();
    }

    return v;
}

void putIntoBuffer(DataBuffer *dataBuffer, vector<Data> items) {
    for (int i = 0; i < items.size();) {
        dataBuffer->addItem(items[i++]);
    }

    dataBuffer->setFinished();
}

void worker(DataBuffer *dataBuffer, ResultsBuffer *resultsBuffer) {
    while (true) {
        Data item = dataBuffer->removeItem();

        if (!item.Valid) {
            break;
        }

        item.Computed = getHash(item);
        resultsBuffer->addItemSorted(item);
    }
}

string getHash(const Data &item) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%s:%d:%f", item.Name.c_str(), item.Age, item.Salary);
    string str = buffer;

    auto buf = new unsigned char[str.length() + 1];
    strcpy((char *) buf, str.c_str());

    unsigned char outputBuffer[SHA_DIGEST_LENGTH];
    for (int i = 0; i < 2000000; i++) {
        SHA512(buf, sizeof(buf) - 1, outputBuffer);

        buf = outputBuffer;
    }

    string hash(reinterpret_cast<char *>(buf));

    return hash;
}
