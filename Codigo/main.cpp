#include <iostream>
#include <cstring>
#include <fstream>
#include <string>

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

    // Procesar de a 3 en 3 bytes
    for (int i = 0; i + 2 < longComp; i += 3) {
        unsigned char count = comprimido[i+1];  // segundo byte = cantidad
        unsigned char c     = comprimido[i+2];  // tercer byte = carácter

        for (int j = 0; j < count; j++) {
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

    // Entrada vacía para índice 0
    diccionario[0].cadena = new unsigned char[1];
    diccionario[0].cadena[0] = '\0';
    diccionario[0].longitud = 0;
    tamDiccionario++;

    // Procesar de a 3 en 3 bytes (TERNAS)
    for (int i = 0; i + 2 < longComp; i += 3) {
        // Primeros 2 bytes = índice, tercer byte = carácter
        int indice = (comprimido[i] << 8) | comprimido[i + 1];
        unsigned char nuevoChar = comprimido[i + 2];

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

            int nuevaLong = longPrefijo + 1;
            diccionario[tamDiccionario].cadena = new unsigned char[nuevaLong + 1];
            for (int j = 0; j < longPrefijo; j++) {
                diccionario[tamDiccionario].cadena[j] = diccionario[indice].cadena[j];
            }
            diccionario[tamDiccionario].cadena[longPrefijo] = nuevoChar;
            diccionario[tamDiccionario].cadena[nuevaLong] = '\0';
            diccionario[tamDiccionario].longitud = nuevaLong;
            tamDiccionario++;
        }
    }

    for (int j = 0; j < tamDiccionario; j++) {
        delete[] diccionario[j].cadena;
    }
}
