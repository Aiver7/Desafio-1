#include <iostream>
#include <cstring>
#include <fstream>

using namespace std;

//  CONSTANTES
const int MAX_BUFFER = 1000000;

//  FUNCIONES DE ENCRIPTACIÓN

unsigned char rotarIzquierda(unsigned char byte, int n) {
    return (byte << n) | (byte >> (8 - n));
}

unsigned char rotarDerecha(unsigned char byte, int n) {
    return (byte >> n) | (byte << (8 - n));
}

void desencriptar(const unsigned char* entrada, int longitud,
                  unsigned char* salida, int n, unsigned char k) {
    for (int i = 0; i < longitud; i++) {
        unsigned char temp = entrada[i] ^ k;
        salida[i] = rotarDerecha(temp, n);
    }
}

//  FUNCIONES RLE

void descomprimirRLE(const unsigned char* comprimido, int longComp,
                     unsigned char* salida, int& longSalida) {
    longSalida = 0;
    int i = 0;

    while (i < longComp) {
        int num = 0;
        while (i < longComp && comprimido[i] >= '0' && comprimido[i] <= '9') {
            num = num * 10 + (comprimido[i] - '0');
            i++;
        }

        if (i >= longComp) break;

        unsigned char c = comprimido[i];
        i++;

        for (int j = 0; j < num; j++) {
            salida[longSalida++] = c;
        }
    }
}
