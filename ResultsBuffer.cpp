#include "ResultsBuffer.h"

void ResultsBuffer::addItemSorted(const Data &item) {
    if (item.Age < 18) {
        return;
    }

    bool added = false;
    while (!added) {
#pragma omp critical (results_critical)
        {
            int i;
            for (i = 0; i < container.size(); i++) {
                if (container[i].Name.compare(item.Name) >= 0) {
                    break;
                }
            }

            container.insert(container.begin() + i, item);
            added = true;

#ifdef DEBUG
            std::cout << omp_get_thread_num() << ": Added item to results\n";
#endif
        }
    }
}

std::vector<Data> ResultsBuffer::getItems() {
    return container;
}