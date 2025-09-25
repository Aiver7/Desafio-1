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
