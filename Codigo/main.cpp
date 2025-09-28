#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>

using namespace std;

//  CONSTANTES
const int MAX_BUFFER = 1000000;

//  ROTACIONES
unsigned char rotarIzquierda(unsigned char byte, int n) {
    unsigned int x = static_cast<unsigned int>(byte);
    unsigned int res = ((x << n) | (x >> (8 - n))) & 0xFFu;
    return static_cast<unsigned char>(res);
}
unsigned char rotarDerecha(unsigned char byte, int n) {
    unsigned int x = static_cast<unsigned int>(byte);
    unsigned int res = ((x >> n) | (x << (8 - n))) & 0xFFu;
    return static_cast<unsigned char>(res);
}

//  DESENCRIPTAR (XOR K - rotación derecha n)
void desencriptar(const unsigned char* entrada, int longitud,
                  unsigned char* salida, int n, unsigned char k) {
    if (!entrada || !salida || longitud <= 0) return;
    if (n <= 0 || n >= 8) return; // n en 1..7
    for (int i = 0; i < longitud; i++) {
        unsigned char temp = static_cast<unsigned char>(entrada[i] ^ k);
        salida[i] = rotarDerecha(temp, n);
    }
}

// RLE ([basura][cantidad][caracter])
void descomprimirRLE(const unsigned char* comprimido, int longComp,
                     unsigned char* salida, int& longSalida) {
    longSalida = 0;
    if (!comprimido || !salida || longComp <= 0) return;
    if (longComp % 3 != 0) return; // debe ser múltiplo de 3

    for (int i = 0; i + 2 < longComp; i += 3) {
        // unsigned char basura = comprimido[i]; // se ignora
        unsigned char count = comprimido[i + 1];  // segundo byte = cantidad
        unsigned char c     = comprimido[i + 2];  // tercer byte = carácter
        if (count == 0) continue;

        for (int j = 0; j < count; j++) {
            if (longSalida >= MAX_BUFFER) return; // evitar overflow de salida
            salida[longSalida++] = c;
        }
    }
}

// LZ78 ([prefijo alto][prefijo bajo][caracter])
void descomprimirLZ78(const unsigned char* comprimido, int longComp,
                      unsigned char* salida, int& longSalida) {
    longSalida = 0;
    if (!comprimido || !salida || longComp <= 0) return;
    if (longComp % 3 != 0) return; // debe ser múltiplo de 3 (ternas)

    // Reservar diccionario en HEAP (evita stack grande)
    unsigned char** dicadenas = new unsigned char*[65536];
    int*            dilong    = new int[65536];

    // Inicializar a valores seguros
    for (int i = 0; i < 65536; ++i) { dicadenas[i] = nullptr; dilong[i] = 0; }

    int  tamDiccionario = 0; // entradas reales: 1..tamDiccionario (0 = vacío)
    bool ok = true;

    for (int i = 0; i + 2 < longComp; i += 3) {
        unsigned short prefijo = (static_cast<unsigned short>(comprimido[i]) << 8) |
                                 static_cast<unsigned short>(comprimido[i + 1]);
        unsigned char  nuevo   = comprimido[i + 2];

        // prefijo válido: 0..tamDiccionario
        if (prefijo > static_cast<unsigned short>(tamDiccionario)) { ok = false; break; }

        int longPref = (prefijo == 0) ? 0 : dilong[prefijo];
        int nuevaLen = longPref + 1;

        // Emitir prefijo (si existe) controlando MAX_BUFFER
        if (prefijo != 0) {
            for (int j = 0; j < longPref; ++j) {
                if (longSalida >= MAX_BUFFER) { ok = false; goto liberar; }
                salida[longSalida++] = dicadenas[prefijo][j];
            }
        }
        // Emitir nuevo carácter
        if (longSalida >= MAX_BUFFER) { ok = false; goto liberar; }
        salida[longSalida++] = nuevo;

        // Agregar nueva entrada al diccionario (si hay espacio)
        if (tamDiccionario < 65535) {
            int idx = tamDiccionario + 1;      // primera libre real
            dilong[idx] = nuevaLen;
            dicadenas[idx] = new unsigned char[nuevaLen]; // sin '\0'

            // Copiar prefijo (si existe)
            if (prefijo != 0) {
                for (int j = 0; j < longPref; ++j) {
                    dicadenas[idx][j] = dicadenas[prefijo][j];
                }
            }
            // Añadir carácter final
            dicadenas[idx][nuevaLen - 1] = nuevo;

            tamDiccionario++;
        }
        // Si se alcanza 65535, se puede seguir sin agregar nuevas entradas.
    }

liberar:
    // Liberar TODAS las cadenas reales (1..tamDiccionario). La 0 nunca reservó.
    for (int j = 1; j <= tamDiccionario; ++j) {
        delete[] dicadenas[j];
        dicadenas[j] = nullptr;
        dilong[j] = 0;
    }
    // Liberar tablas del diccionario
    delete[] dicadenas;
    delete[] dilong;

    (void)ok; // opcional: puedes usar 'ok' para reportar error si lo deseas
}

//  BUSCAR PARÁMETROS
bool buscarParametros(const unsigned char* encriptado, int longEnc,
                      const char* fragmento, int longFrag,
                      int& n_out, unsigned char& k_out, int& metodo_out) {
    if (!encriptado || longEnc <= 0 || !fragmento || longFrag <= 0) return false;

    // Buffers grandes en HEAP (evitar stack overflow)
    unsigned char* desencriptado = new unsigned char[longEnc]; // igual al cifrado
    unsigned char* descomprimido = new unsigned char[MAX_BUFFER];

    for (int n = 1; n < 8; n++) {
        for (int k = 0; k < 256; k++) {
            // Desencriptar
            desencriptar(encriptado, longEnc, desencriptado, n, (unsigned char)k);

            // Intento RLE
            int longDesc = 0;
            descomprimirRLE(desencriptado, longEnc, descomprimido, longDesc);
            if (longDesc >= longFrag) {
                bool coincide = false;
                for (int pos = 0; pos <= longDesc - longFrag && !coincide; pos++) {
                    bool ok = true;
                    for (int j = 0; j < longFrag; j++) {
                        if (descomprimido[pos + j] != (unsigned char)fragmento[j]) { ok = false; break; }
                    }
                    if (ok) coincide = true;
                }
                if (coincide) {
                    n_out = n;
                    k_out = (unsigned char)k;
                    metodo_out = 1;
                    delete[] desencriptado;
                    delete[] descomprimido;
                    return true;
                }
            }

            // Intento LZ78
            longDesc = 0;
            descomprimirLZ78(desencriptado, longEnc, descomprimido, longDesc);
            if (longDesc >= longFrag) {
                bool coincide = false;
                for (int pos = 0; pos <= longDesc - longFrag && !coincide; pos++) {
                    bool ok = true;
                    for (int j = 0; j < longFrag; j++) {
                        if (descomprimido[pos + j] != (unsigned char)fragmento[j]) { ok = false; break; }
                    }
                    if (ok) coincide = true;
                }
                if (coincide) {
                    n_out = n;
                    k_out = (unsigned char)k;
                    metodo_out = 2;
                    delete[] desencriptado;
                    delete[] descomprimido;
                    return true;
                }
            }
        }
    }

    delete[] desencriptado;
    delete[] descomprimido;
    return false;
}

//  LECTURA DE ARCHIVOS
bool leerArchivoEncriptado(const char* nombreArchivo, unsigned char* buffer, int& longitud) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo) {
        cout << "Error: No se pudo abrir " << nombreArchivo << endl;
        return false;
    }
    archivo.seekg(0, ios::end);
    longitud = (int)archivo.tellg();
    archivo.seekg(0, ios::beg);

    if (longitud <= 0 || longitud > MAX_BUFFER) {
        cout << "Error: Archivo vacio o demasiado grande" << endl;
        return false;
    }
    archivo.read((char*)buffer, longitud);
    archivo.close();
    return true;
}
bool leerArchivoPista(const char* nombreArchivo, char* buffer, int& longitud) {
    ifstream archivo(nombreArchivo);
    if (!archivo) {
        cout << "Error: No se pudo abrir " << nombreArchivo << endl;
        return false;
    }
    archivo.getline(buffer, MAX_BUFFER);
    longitud = (int)strlen(buffer);
    archivo.close();

    return true;
}

