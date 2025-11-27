#ifndef GESTOR_ARCHIVOS_HPP
#define GESTOR_ARCHIVOS_HPP

#include "Constantes.hpp"
#include <string>

class GestorArchivos {
public:
    // Operaciones básicas de archivos
    static bool archivoExiste(const char* nombreArchivo);
    static bool inicializarArchivo(const char* nombreArchivo, const char* tipo);
    static bool verificarArchivo(const char* nombreArchivo);
    
    // Manejo de headers
    static ArchivoHeader leerHeader(const char* nombreArchivo, bool mostrarInfo = false);
    static bool actualizarHeader(const char* nombreArchivo, ArchivoHeader header);
    
    // Utilidades
    static const char* determinarTipoArchivo(const char* nombreArchivo);
    static bool crearDirectorioSiNoExiste(const char* ruta);
    static bool realizarRespaldo(const char* nombreArchivo);
    
    // Validaciones
    static bool validarHeader(const ArchivoHeader& header);
    
private:
    static bool escribirHeader(const char* nombreArchivo, const ArchivoHeader& header);
};

#endif