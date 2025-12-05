#ifndef GESTOR_ARCHIVOS_HPP
#define GESTOR_ARCHIVOS_HPP

#include "Constantes.hpp"
#include <string>

class GestorArchivos {
public:
	
    // Operaciones avanzadas de archivos
    static bool verificarIntegridad(const char* nombreArchivo);
    static bool reconstruirArchivo(const char* nombreArchivo);
    static bool realizarRespaldoCompleto();
    static bool restaurarRespaldo(const char* nombreRespaldo, const char* nombreDestino);
    static bool limpiarRegistrosEliminados(const char* nombreArchivo);
    static void mostrarInformacionArchivos();
    static bool reindexarArchivo(const char* nombreArchivo);

    // Operaciones básicas de archivos
    static bool archivoExiste(const char* nombreArchivo);
    static bool inicializarArchivo(const char* nombreArchivo, const char* tipo);
    static bool verificarArchivo(const char* nombreArchivo);
    
    // Manejo de headers
    static ArchivoHeader leerHeader(const char* nombreArchivo, bool mostrarInfo = false);
    static bool actualizarHeader(const char* nombreArchivo, ArchivoHeader header);
    
    // Utilidades
    static const char* determinarTipoArchivo(const char* nombreArchivo);
    static bool realizarRespaldo(const char* nombreArchivo);
    static bool crearDirectorioSiNoExiste(const char* directorio);
    
    // Validaciones
    static bool validarHeader(const ArchivoHeader& header);
   
private:
    static bool escribirHeader(const char* nombreArchivo, const ArchivoHeader& header);
    static bool copiarArchivo(const char* origen, const char* destino);
  
};

#endif
