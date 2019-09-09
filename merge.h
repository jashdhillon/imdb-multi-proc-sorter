#include "data.h"

//Merges 2 lists in sorted order
//Core of the merge sort sorting fucntion
void merge(Movie* movies, int cat_index, int l, int m, int r);

//Helper function for the merge sort sorting algorithm
void sort(Movie* movies, int cat_index, int l, int r);

//Pulls the elements with a value of null for the data at the index cat_index to the front in the original order
int bring_null_forward(Movie* movies, int cat_index, int size);
