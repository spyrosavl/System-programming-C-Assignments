//
// Created by spyros on 9/3/2018.
//
#include "util.h"


/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
int partition (List<SearchResult *> * list, int low, int high)
{
    double pivot = list->value(high)->score;    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (list->value(j)->score > pivot)
        {
            i++;    // increment index of smaller element
            SearchResult * temp = list->value(i);
            list->update(i, list->value(j));
            list->update(j, temp);
        }
    }
    //swap(&arr[i + 1], &arr[high]);
    SearchResult * temp = list->value(i + 1);
    list->update(i+1, list->value(high));
    list->update(high, temp);

    return (i + 1);
}

/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void quickSort(List<SearchResult *> * list, int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(list, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(list, low, pi - 1);
        quickSort(list, pi + 1, high);
    }
}