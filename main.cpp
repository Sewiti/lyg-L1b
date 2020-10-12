#include "Data.h"
#include "DataBuffer.h"
#include "ResultsBuffer.h"
#include "json.hpp"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <openssl/sha.h>
#include <string>
#include <vector>

using namespace std;


void executeFile(const string &dataFilename, const string &resultsFilename);

vector<Data> readJSON(const string &filename);

void outputResults(const string &filename, vector<Data> initData, vector<Data> results);

void putIntoBuffer(DataBuffer *dataBuffer, vector<Data> items);

void worker(DataBuffer *dataBuffer, ResultsBuffer *resultsBuffer);

string getHash(const Data &item);

string base64_encode(const unsigned char *src, size_t len);


int main() {
    for (int i = 1; i <= 3; i++) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "IFF-8-5_BernotasM_L1b_dat_%d.json", i);
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
    vector<Data> items = readJSON(dataFilename);

    auto dataBuffer = new DataBuffer(8);
    auto resultsBuffer = new ResultsBuffer();

    unsigned int n = items.size() / 4;

#pragma omp parallel num_threads(n+1) default(none) shared(dataBuffer, resultsBuffer, items, n)
    {
        if (omp_get_thread_num() == n) {
            // "Main" thread
            putIntoBuffer(dataBuffer, items);
        } else {
            // Worker threads
            worker(dataBuffer, resultsBuffer);
        }
    }

    outputResults(resultsFilename, items, resultsBuffer->getItems());
}

vector<Data> readJSON(const string &filename) {
    ifstream file(filename);
    vector<Data> v;

    if (file.is_open()) {
        auto json = nlohmann::json::parse(file);

        for (const auto &item : json) {
            Data d{
                    item["name"],
                    item["age"],
                    item["salary"]
            };

            v.push_back(d);
        }

        file.close();
    }

    return v;
}

void outputResults(const string &filename, vector<Data> initData, vector<Data> results) {
    ofstream file(filename);

    if (file.is_open()) {
        file << "Initial data:\n"
             << "  # | Name                          | Age | Salary\n"
             << "----+-------------------------------+-----+----------\n";

        if (initData.empty()) {
            file << "  - | -                             |   - |    --.--\n";
        } else {
            for (int i = 0; i < initData.size(); i++) {
                Data v = initData[i];
                file << setw(3) << i + 1 << " | "
                     << setw(30) << left << v.Name << "|"
                     << setw(4) << right << v.Age << " |"
                     << setw(9) << fixed << setprecision(2) << v.Salary << endl;
            }
        }

        file << "\nSorted results (age >= 18):\n"
             << "  # | Name                          | Age | Salary   | Hash\n"
             << "----+-------------------------------+-----+----------+--------------\n";

        if (results.empty()) {
            file << "  - | -                             |   - |    --.-- | -\n";
        } else {
            for (int i = 0; i < results.size(); i++) {
                Data v = results[i];
                file << setw(3) << i + 1 << " | "
                     << setw(30) << left << v.Name << "|"
                     << setw(4) << right << v.Age << " |"
                     << setw(9) << fixed << setprecision(2) << v.Salary << " | "
                     << v.Computed << endl;
            }
        }

        file.close();
    }
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

//    string hash(reinterpret_cast<char *>(buf));

    return base64_encode(buf, sizeof(buf));
}

static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

string base64_encode(const unsigned char *src, size_t len) {
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return std::string(); /* integer overflow */

    std::string outStr;
    outStr.resize(olen);
    out = (unsigned char *) &outStr[0];

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                                  (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}

#pragma clang diagnostic pop