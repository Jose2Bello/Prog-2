#include "GestorArchivos.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/stat.h>  // Para crear directorios en diferentes sistemas

using namespace std;

bool GestorArchivos::archivoExiste(const char* nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary);
    bool existe = archivo.is_open();
    if (existe) {
        archivo.close();
    }
    return existe;
}

bool GestorArchivos::inicializarArchivo(const char* nombreArchivo, const char* tipo) {
    // Crear directorio si no existe
    crearDirectorioSiNoExiste("datos");
    
    ofstream archivo(nombreArchivo, ios::binary | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se pudo crear el archivo " << nombreArchivo << endl;
        return false;
    }

    // Crear header con valores iniciales
    ArchivoHeader header;
    header.cantidadRegistros = 0;
    header.proximoID = 1;
    header.registrosActivos = 0;
    header.version = VERSION_ACTUAL;
    header.fechaCreacion = time(nullptr);
    header.fechaUltimaModificacion = header.fechaCreacion;

    // Copiar tipo de archivo
    strncpy(header.tipoArchivo, tipo, sizeof(header.tipoArchivo) - 1);
    header.tipoArchivo[sizeof(header.tipoArchivo) - 1] = '\0';

    // Escribir header al archivo
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se pudo escribir el header en " << nombreArchivo << endl;
        archivo.close();
        return false;
    }

    archivo.close();

    cout << "Archivo inicializado: " << nombreArchivo << " (Tipo: " << tipo << ")" << endl;
    return true;
}

bool GestorArchivos::verificarArchivo(const char* nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo " << nombreArchivo << endl;
        return false;
    }

    // Verificar que el archivo tenga al menos el tamaño del header
    archivo.seekg(0, ios::end);
    streampos fileSize = archivo.tellg();
    archivo.seekg(0, ios::beg);

    if (fileSize < static_cast<streampos>(sizeof(ArchivoHeader))) {
        cout << "Error: Archivo " << nombreArchivo
             << " está corrupto (tamaño insuficiente)" << endl;
        archivo.close();
        return false;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se pudo leer el header de " << nombreArchivo << endl;
        archivo.close();
        return false;
    }

    archivo.close();

    // Validar header
    return validarHeader(header);
}

ArchivoHeader GestorArchivos::leerHeader(const char* nombreArchivo, bool mostrarInfo) {
    ArchivoHeader header;

    // Valores por defecto para header inválido
    memset(&header, 0, sizeof(header));
    header.proximoID = 1;
    header.version = VERSION_ACTUAL;
    header.fechaCreacion = time(nullptr);
    header.fechaUltimaModificacion = header.fechaCreacion;
    strcpy(header.tipoArchivo, "INVALIDO");

    // Intentar leer archivo
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo.is_open()) {
        if (mostrarInfo) {
            cout << "No existe: " << nombreArchivo << endl;
        }
        return header;
    }

    // Verificar tamaño
    archivo.seekg(0, ios::end);
    long tamanio = archivo.tellg();
    archivo.seekg(0, ios::beg);

    if (tamanio < static_cast<long>(sizeof(ArchivoHeader))) {
        if (mostrarInfo) {
            cout << "Tamaño insuficiente: " << nombreArchivo << endl;
        }
        archivo.close();
        return header;
    }

    // Leer header
    if (!archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader))) {
        if (mostrarInfo) {
            cout << "Error de lectura: " << nombreArchivo << endl;
        }
        archivo.close();
        return header;
    }

    archivo.close();

    // Mostrar información si se solicita
    if (mostrarInfo) {
        cout << "HEADER de " << nombreArchivo << ":" << endl;
        cout << "  Tipo: " << header.tipoArchivo << endl;
        cout << "  Versión: " << header.version << endl;
        cout << "  Registros: " << header.registrosActivos << "/"
             << header.cantidadRegistros << endl;
        cout << "  Próximo ID: " << header.proximoID << endl;

        // Formatear fechas
        char fechaCreacion[20], fechaModificacion[20];
        strftime(fechaCreacion, sizeof(fechaCreacion), "%Y-%m-%d %H:%M",
                 localtime(&header.fechaCreacion));
        strftime(fechaModificacion, sizeof(fechaModificacion), "%Y-%m-%d %H:%M",
                 localtime(&header.fechaUltimaModificacion));

        cout << "  Creado: " << fechaCreacion << endl;
        cout << "  Modificado: " << fechaModificacion << endl;
    }

    return header;
}

bool GestorArchivos::actualizarHeader(const char* nombreArchivo, ArchivoHeader header) {
    // Verificar que el archivo existe
    if (!archivoExiste(nombreArchivo)) {
        cout << "Error: " << nombreArchivo << " no existe" << endl;
        return false;
    }

    // Abrir en modo lectura/escritura
    fstream archivo(nombreArchivo, ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir " << nombreArchivo << " para escritura" << endl;
        return false;
    }

    // Validar header antes de escribir
    if (header.proximoID < 1) {
        cout << "Advertencia: próximoID inválido, ajustando a 1" << endl;
        header.proximoID = 1;
    }

    if (header.registrosActivos < 0) {
        cout << "Advertencia: registrosActivos negativo, ajustando a 0" << endl;
        header.registrosActivos = 0;
    }

    if (header.registrosActivos > header.cantidadRegistros) {
        cout << "Advertencia: registrosActivos > cantidadRegistros, ajustando" << endl;
        header.registrosActivos = header.cantidadRegistros;
    }

    // Actualizar metadata
    header.fechaUltimaModificacion = time(nullptr);

    // Posicionarse al inicio y escribir
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));

    // Forzar escritura a disco
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (exito) {
        cout << "Header actualizado: " << nombreArchivo << endl;
    } else {
        cout << "Error crítico: No se pudo escribir header en " << nombreArchivo << endl;
    }

    return exito;
}

const char* GestorArchivos::determinarTipoArchivo(const char* nombreArchivo) {
    if (strstr(nombreArchivo, "pacientes") != nullptr) return "PACIENTES";
    if (strstr(nombreArchivo, "doctores") != nullptr) return "DOCTORES";
    if (strstr(nombreArchivo, "citas") != nullptr) return "CITAS";
    if (strstr(nombreArchivo, "historiales") != nullptr) return "HISTORIALES";
    if (strstr(nombreArchivo, "hospital") != nullptr) return "HOSPITAL";
    return "GENERAL";
}


bool GestorArchivos::realizarRespaldo(const char* nombreArchivo) {
    if (!archivoExiste(nombreArchivo)) {
        cout << "Error: No se puede hacer respaldo de " << nombreArchivo << " (no existe)" << endl;
        return false;
    }

    // Crear nombre de respaldo con timestamp
    time_t ahora = time(nullptr);
    char nombreResaldo[100];
    strftime(nombreResaldo, sizeof(nombreResaldo), "backups/%Y%m%d_%H%M%S_", localtime(&ahora));
    strcat(nombreResaldo, strrchr(nombreArchivo, '/') ? strrchr(nombreArchivo, '/') + 1 : nombreArchivo);

    // Crear directorio de backups si no existe
    crearDirectorioSiNoExiste("backups");

    // Copiar archivo
    ifstream origen(nombreArchivo, ios::binary);
    ofstream destino(nombreResaldo, ios::binary);

    if (!origen.is_open() || !destino.is_open()) {
        cout << "Error: No se pudo crear respaldo" << endl;
        return false;
    }

    destino << origen.rdbuf();

    bool exito = destino.good();
    origen.close();
    destino.close();

    if (exito) {
        cout << "Respaldo creado: " << nombreResaldo << endl;
    } else {
        cout << "Error al crear respaldo" << endl;
    }

    return exito;
}

bool GestorArchivos::validarHeader(const ArchivoHeader& header) {
    bool valido = true;
    string errores;

    // Validar versión
    if (header.version != VERSION_ACTUAL) {
        errores += "Versión inválida. ";
        valido = false;
    }

    // Validar contadores no negativos
    if (header.cantidadRegistros < 0) {
        errores += "cantidadRegistros negativo. ";
        valido = false;
    }

    if (header.proximoID < 1) {
        errores += "proximoID inválido. ";
        valido = false;
    }

    if (header.registrosActivos < 0 || header.registrosActivos > header.cantidadRegistros) {
        errores += "registrosActivos inconsistente. ";
        valido = false;
    }

    // Validar fechas (no pueden ser futuras)
    time_t ahora = time(nullptr);
    if (header.fechaCreacion > ahora) {
        errores += "fechaCreación en futuro. ";
        valido = false;
    }

    if (header.fechaUltimaModificacion > ahora) {
        errores += "fechaModificación en futuro. ";
        valido = false;
    }

    // Validar tipo de archivo
    if (strlen(header.tipoArchivo) == 0 || strlen(header.tipoArchivo) >= sizeof(header.tipoArchivo)) {
        errores += "tipoArchivo inválido. ";
        valido = false;
    }

    if (!valido) {
        cout << "Header inválido: " << errores << endl;
    }

    return valido;
}