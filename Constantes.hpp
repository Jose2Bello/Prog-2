#ifndef CONSTANTES_HPP
#define CONSTANTES_HPP

#include <ctime>

// Estructura del Header de archivos (para todos)
struct ArchivoHeader {
    int cantidadRegistros;        // Total de registros físicos
    int proximoID;                // Siguiente ID disponible
    int registrosActivos;         // Registros no eliminados
    int version;                  // Versión del formato
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
    char tipoArchivo[20];         // "PACIENTES", "DOCTORES", "CITAS", "HISTORIALES"
};


const char* const RUTA_PACIENTES = "datos\\pacientes.bin";
const char* const RUTA_DOCTORES = "datos\\doctores.bin";
const char* const RUTA_CITAS = "datos\\citas.bin";
const char* const RUTA_HISTORIALES = "datos\\historiales.bin";
const char* const RUTA_HOSPITAL = "datos\\hospital.bin";


const int VERSION_ACTUAL = 1;

#endif