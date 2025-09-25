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

//  FUNCIONES LZ78

struct EntradaDiccionario {
    unsigned char* cadena;
    int longitud;
};

void descomprimirLZ78(const unsigned char* comprimido, int longComp,
                      unsigned char* salida, int& longSalida) {
    EntradaDiccionario diccionario[65536];
    int tamDiccionario = 0;
    longSalida = 0;
    int i = 0;

    // Entrada vacía para índice 0
    diccionario[0].cadena = new unsigned char[1];
    diccionario[0].cadena[0] = '\0';
    diccionario[0].longitud = 0;
    tamDiccionario++;

    while (i < longComp - 2) {
        int indice = (comprimido[i] << 8) | comprimido[i + 1];
        i += 2;

        if (i >= longComp) break;

        unsigned char nuevoChar = comprimido[i];
        i++;

        if (indice == 0) {
            salida[longSalida++] = nuevoChar;

            diccionario[tamDiccionario].cadena = new unsigned char[2];
            diccionario[tamDiccionario].cadena[0] = nuevoChar;
            diccionario[tamDiccionario].cadena[1] = '\0';
            diccionario[tamDiccionario].longitud = 1;
            tamDiccionario++;
        } else if (indice < tamDiccionario) {
            int longPrefijo = diccionario[indice].longitud;

            for (int j = 0; j < longPrefijo; j++) {
                salida[longSalida++] = diccionario[indice].cadena[j];
            }

            salida[longSalida++] = nuevoChar;
