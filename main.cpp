#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <omp.h>
#include <openssl/sha.h>
#include <cstring>
using namespace std;

struct Data {
    string Name;
    int Age{};
    double Salary{};
    string Computed;
};

vector<Data> readData(string filename);
string getHash(Data item);

int main() {
    for (int fileN = 1; fileN <= 3; fileN++) {
        cout << "Started file  #" << fileN << endl;

        char buffer[50];
        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_dat_%d.txt", fileN);
        string dataFilename = buffer;

        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_rez_%d.txt", fileN);
        string resultsFilename = buffer;

        vector<Data> items = readData(dataFilename);
        vector<Data> dataBuffer, resultsBuffer;
        int maxBufferSize = 5;
        bool finished = false;

        int n = items.size() / 4;

        #pragma omp parallel num_threads(n+1)
        {
            if (omp_get_thread_num() == 0) {
                // "Main" thread, kuris idedineja duomenis
                for (int i = 0; i < items.size();) {
                    #pragma omp critical (data_critical)
                    {
                        if (dataBuffer.size() < maxBufferSize) {
//                            cout << "Inserted item\n";
                            dataBuffer.push_back(items[i]);
                            i++;
                        }
                    }
                }
//                cout << "Finished inserting\n";
                finished = true;
            }
            else {
                // Worker threads
                while (true) {
                    Data item;

                    bool received = false;
                    while (!received) {
                        #pragma omp critical (data_critical)
                        {
                            if (dataBuffer.size() > 0) {
//                                cout << "Removed item\n";
                                item = dataBuffer[0];
                                dataBuffer.erase(dataBuffer.begin());
                                received = true;
                            }
                        }

                        if (!received && finished) {
                            break;
                        }
                    }

                    if (!received && finished) {
//                        cout << "Thread exit\n";
                        break;
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
//                            cout << "Pushed item\n";
                            resultsBuffer.push_back(d);
                            pushed = true;
                        }
                    }
                }
            }
        };

        cout << "Finished file  #" << fileN << endl;
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

    return "thisIsAHash";
}