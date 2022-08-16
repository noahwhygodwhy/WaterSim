//stolen from https://www.programiz.com/dsa/quick-sort cause I don't want to write quicksort over again
//and opencl doesn't come with that feature of C99

#ifndef QUICK_SORT_H
#define QUICK_SORT_H

void swap(__local unsigned int *a, __local unsigned int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(__local unsigned int* array, int low, int high) {
    int pivot = array[high];
    int i = (low - 1);
    for (int j = low; j < high; j++) {
        if (array[j] <= pivot) {
        i++;
        swap(&array[i], &array[j]);
        }
    }
    swap(&array[i + 1], &array[high]);
    return (i + 1);
}

void quickSort(__local unsigned int* array, int low, int high) {
    if (low < high) {
        int pi = partition(array, low, high);
        quickSort(array, low, pi - 1);
        quickSort(array, pi + 1, high);
    }
}
#endif