#include <stdio.h>
#include <stdlib.h>
#include <time.h>


//This Program calculates the scalar Product of two random Vectors and uses Quicksort Algorithm to sort the resulting Vector, that is the component wise product of the two random Vectors.
//We do this in order to increase accuracy of the scalar product calculation


void UniformArray(double arr[], int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (double)rand() / RAND_MAX;
    }
}

// Eine Funktion, um zwei Elemente zu tauschen
void swap(double *a, double *b) {
    double temp = *a; // Wir machen einen tempörären Zwischenspeicher von a in temp
    *a = *b; //Wir überschreiben a mit b
    *b = temp; //Wir überschreiben b mit temp
}

//Das ist eine Funktion die einen int zurückgibt und einen Array mit zwei Parametern bekommt. Sie soll eine Partition erstellen vom Array arr[], wobei alle Elemente die kleiner als das Pivot Element sind, links vom Pivot Element stehen und alle Elemente die größer sind, rechts vom Pivot Element stehen.
//Es gibt dann die Position des Pivot Elements aus
int partition(double arr[], int low, int high) {
    double pivot = arr[high]; //Wähle high als den Index des Pivot Elements
    int i = (low - 1); // Wir machen eine Liste von low zu woach immer i geht, welche die Elemente enthält die kleiner als das Pivot Element sind

    // Durchlaufe das Array von low bis high und vergleiche die Elemente mit dem Pivot
    for (int j = low; j < high; j++) {

        if (arr[j] <= pivot) {
            i++; // Erhöhe den rechten Rand der Liste der Elemente die kleiner als das Pivot Element sind um 1
            swap(&arr[i], &arr[j]);// Tuhe das Element arr[j] in die Liste der Elemente die kleiner als das Pivot Element sind
        }
    }
    swap(&arr[i + 1], &arr[high]); // Mach das Pivot Element arr[j] an die rechte Stelle der Liste der Elemente die kleiner als das Pivot Element sind 
    return (i + 1); //Gib die Position des Pivot Elements
}

// Quicksort Funktion. Kein Autput notwedig, da wir nur die Adressen der Elemente ändern
void quickSort(double arr[], int low, int high) {
    if (low < high) { //Saftey Check, sodass wir nicht in eine Endlosschleife geraten
        //Mach die geordnete Partition und gib die Position des Pivot Elements
        int pi = partition(arr, low, high);

        //Mach die geordnete Partition für die Elemente links und rechts vom Pivot Element nochmal ....
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Printe die Elemente vom Array
// arr ist nur ein Pointer auf die Adresse des ersten Elements des Arrays. Deswegen ist das printen so umständlich
void printArray(double arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%f ", arr[i]);
    }
    printf("\n");
}


int main() {
    int n = 10; 
    double arr[n]; 
    double arrOne[n];
    double arrTwo[n];
    double Sum;

    srand(time(NULL)); // Zufallsgenerator initialisieren
    UniformArray(arrOne, n); //zwei Zufallsvektoren machen
    UniformArray(arrTwo, n);
    for (int i = 0; i < n; i++) {
        arr[i] = arrOne[i] * arrTwo[i]; //Elementweise Multiplikation
    }


    printf("Originaler Array: ");
    printArray(arr, n);

    quickSort(arr, 0, n - 1);

    printf("Sortierter Array: ");
    printArray(arr, n);
    for(int i=0; i<n;i++){
        Sum += arr[i];
    }
    printf("Summe des Arrays %f ", Sum);
    return 0;
} 

// Run this Programm in the Terminal using
// gcc -c Quicksort.c && gcc Quicksort.o -o Quicksort && time ./Quicksort