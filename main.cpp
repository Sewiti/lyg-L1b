#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <omp.h>
#include <openssl/sha.h>
#include <cstring>
using namespace std;

//#define DEBUG

struct Data {
    string Name;
    int Age{};
    double Salary{};
    string Computed;
};

vector<Data> readData(string filename);
string getHash(Data item);
void putIntoBuffer(vector<Data> *dataBuffer, int maxBufferSize, vector<Data> items, bool *finished);
void worker(vector<Data> *dataBuffer, vector<Data> *resultsBuffer, bool *finished);

int main() {
    for (int fileN = 1; fileN <= 1; fileN++) {
        cout << "Started file  #" << fileN << endl;

        char buffer[50];
        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_dat_%d.txt", fileN);
        string dataFilename = buffer;

        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_rez_%d.txt", fileN);
        string resultsFilename = buffer;

        vector<Data> items = readData(dataFilename);
        vector<Data> dataBuffer, resultsBuffer;

        int n = items.size() / 4;
        bool finished = false;

        #pragma omp parallel num_threads(n+1) default(none) shared(dataBuffer, resultsBuffer, items, finished)
        {
            if (omp_get_thread_num() == 0) {
                // "Main" thread
                putIntoBuffer(&dataBuffer, 8, items, &finished);
            }
            else {
                // Worker threads
                worker(&dataBuffer, &resultsBuffer, &finished);
            }
        };

        cout << "Finished file #" << fileN << endl;
    }

//        outputResults(resultsFilename, items, resultMonitor.getItems())
//        fmt.Printf("Finished %d\n", i)

    return 0;
}

vector<Data> readData(string filename) {
    ifstream file(filename);

    vector<Data> v;

    if (file.is_open()) {
        int n;

        file >> n;

        for (int i = 0; i < n; i++) {
            string fname, lname;

            Data d;
            file >> fname >> lname >> d.Age >> d.Salary;

            char buffer[30];
            snprintf(buffer, sizeof(buffer), "%s %s", fname.c_str(), lname.c_str());
            d.Name = buffer;

            v.push_back(d);
        }

        file.close();
    }

    return v;
}

string getHash(Data item) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%s:%d:%f", item.Name.c_str(), item.Age, item.Salary);
    string str = buffer;

    unsigned char *buf = new unsigned char[str.length()+1];
    strcpy((char *)buf, str.c_str());

    unsigned char obuf[SHA_DIGEST_LENGTH];
    for (int i = 0; i < 2000000; i++) {
        SHA512(buf, sizeof(buf) -1, obuf);

        buf = obuf;
    }

    // TODO: convert to string and return

    return "thisIsAHash";
}

void putIntoBuffer(vector<Data> *dataBuffer, int maxBufferSize, vector<Data> items, bool *finished) {
    for (int i = 0; i < items.size();) {
        #pragma omp critical(data_critical)
        {
            if (dataBuffer->size() < maxBufferSize) {
#ifdef DEBUG
                cout << omp_get_thread_num() << ": Inserted item\n";
#endif
                dataBuffer->push_back(items[i]);
                i++;
            }
        }
    }
#ifdef DEBUG
    cout << omp_get_thread_num() << ": Finished inserting\n";
#endif
    *finished = true;
}

void worker(vector<Data> *dataBuffer, vector<Data> *resultsBuffer, bool *finished) {
    while (true) {
        Data item;

        bool received = false;
        while (!received) {
            #pragma omp critical(data_critical)
            {
                if (dataBuffer->size() > 0) {
#ifdef DEBUG
                    cout << omp_get_thread_num() << ": Removed item\n";
#endif
                    item = dataBuffer->at(0);
                    dataBuffer->erase(dataBuffer->begin());
                    received = true;
                }
            }

            if (!received && *finished) {
#ifdef DEBUG
                cout << omp_get_thread_num() << ": Thread exit\n";
#endif
                return;
            }
        }

        Data d;
        d.Name = item.Name;
        d.Age = item.Age;
        d.Salary = item.Salary;
        d.Computed = getHash(item);

        bool pushed = false;
        while (!pushed) {
            #pragma omp critical (data_critical)
            {
#ifdef DEBUG
                cout << omp_get_thread_num() << ": Pushed item\n";
#endif
                resultsBuffer->push_back(d);
                pushed = true;
            }
        }
    }
}