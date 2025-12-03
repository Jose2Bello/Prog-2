#include "GestorArchivos.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <direct.h>
#include <sys/stat.h> 
#include "../Hospital/hospital.hpp"
#include "../Doctores/Doctor.hpp"
#include "../Citas/Citas.hpp"
#include "../Pacientes/Pacientes.hpp"

using namespace std;

bool GestorArchivos::verificarIntegridad(const char* nombreArchivo) {
    cout << "\n=== VERIFICANDO INTEGRIDAD: " << nombreArchivo << " ===" << endl;
    
    if (!archivoExiste(nombreArchivo)) {
        cout << " Archivo no existe: " << nombreArchivo << endl;
        return false;
    }
    
    // Verificar header
    ArchivoHeader header = leerHeader(nombreArchivo, false);
    if (!validarHeader(header)) {
        cout << " Header corrupto" << endl;
        return false;
    }
    cout << " Header válido" << endl;
    
    // Verificar tamaño del archivo
    ifstream archivo(nombreArchivo, ios::binary | ios::ate);
    streampos tamanio = archivo.tellg();
    archivo.close();
    
    // Calcular tamaño esperado basado en el tipo de archivo
    streampos tamanioEsperado = sizeof(ArchivoHeader);
    const char* tipo = determinarTipoArchivo(nombreArchivo);
    
    if (strcmp(tipo, "PACIENTES") == 0) {
        tamanioEsperado += header.cantidadRegistros * sizeof(Paciente);
    } else if (strcmp(tipo, "DOCTORES") == 0) {
        tamanioEsperado += header.cantidadRegistros * sizeof(Doctor);
    } else if (strcmp(tipo, "CITAS") == 0) {
        tamanioEsperado += header.cantidadRegistros * sizeof(Cita);
    } else if (strcmp(tipo, "HOSPITAL") == 0) {
        tamanioEsperado += sizeof(Hospital);
    }
    
    if (tamanio != tamanioEsperado) {
        cout << "    Tamaño del archivo incorrecto" << endl;
        cout << "   Esperado: " << tamanioEsperado << " bytes" << endl;
        cout << "   Encontrado: " << tamanio << " bytes" << endl;
        return false;
    }
    cout << " Tamaño del archivo correcto: " << tamanio << " bytes" << endl;
    
    // Verificar consistencia de contadores
    if (header.registrosActivos > header.cantidadRegistros) {
        cout << "   Inconsistencia en contadores" << endl;
        cout << "   Registros activos: " << header.registrosActivos << endl;
        cout << "   Total registros: " << header.cantidadRegistros << endl;
        return false;
    }
    cout << " Contadores consistentes" << endl;
    
    cout << " Archivo verificado correctamente" << endl;
    return true;
}

bool GestorArchivos::archivoExiste(const char* nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary);
    bool existe = archivo.is_open();
    if (existe) {
        archivo.close();
    }
    return existe;
}

bool GestorArchivos::restaurarRespaldo(const char* nombreRespaldo, const char* nombreDestino) {
    cout << "\n=== RESTAURANDO RESPALDO ===" << endl;
    cout << "Desde: " << nombreRespaldo << endl;
    cout << "Hacia: " << nombreDestino << endl;
    
    if (!archivoExiste(nombreRespaldo)) {
        cout << " Archivo de respaldo no existe: " << nombreRespaldo << endl;
        return false;
    }
    
    // Verificar que el respaldo sea válido
    if (!verificarArchivo(nombreRespaldo)) {
        cout << " El archivo de respaldo está corrupto" << endl;
        return false;
    }
    
    // Crear respaldo del archivo actual si existe
    if (archivoExiste(nombreDestino)) {
        char respaldoActual[250];
        sprintf(respaldoActual, "%s.pre_restauracion", nombreDestino);
        if (copiarArchivo(nombreDestino, respaldoActual)) {
            cout << " Respaldo del archivo actual creado: " << respaldoActual << endl;
        }
    }
    
    // Restaurar el respaldo
    if (copiarArchivo(nombreRespaldo, nombreDestino)) {
        cout << " Respaldo restaurado exitosamente" << endl;
        
        // Actualizar fecha de modificación en el header
        ArchivoHeader header = leerHeader(nombreDestino, false);
        header.fechaUltimaModificacion = time(nullptr);
        actualizarHeader(nombreDestino, header);
        
        return true;
    }
    
    cout << " Error al restaurar respaldo" << endl;
    return false;
}

bool GestorArchivos::crearDirectorioSiNoExiste(const char* directorio) {
    // Verificar parámetro
    if (!directorio || directorio[0] == '\0') {
        return false;
    }
    
    #ifdef _WIN32
        // Windows
        if (_mkdir(directorio) == 0 || errno == EEXIST) {
            return true;
        }
    #else
        // Linux/Mac
        if (mkdir(directorio, 0755) == 0 || errno == EEXIST) {
            return true;
        }
    #endif
    
    // Si llegamos aquí, hubo un error
    cout << "Error: No se pudo crear/acceder al directorio '" 
         << directorio << "' (Error: " << errno << ")" << endl;
    return false;
}


bool GestorArchivos::copiarArchivo(const char* origen, const char* destino) {
    ifstream src(origen, ios::binary);
    ofstream dst(destino, ios::binary);
    
    if (!src.is_open() || !dst.is_open()) {
        return false;
    }
    
    dst << src.rdbuf();
    return src.good() && dst.good();
}

bool GestorArchivos::limpiarRegistrosEliminados(const char* nombreArchivo) {
    cout << "\n=== LIMPIANDO REGISTROS ELIMINADOS: " << nombreArchivo << " ===" << endl;
    
    if (!archivoExiste(nombreArchivo)) {
        cout << " Archivo no existe" << endl;
        return false;
    }
    
    // Crear archivo temporal
    char archivoTemp[250];
    sprintf(archivoTemp, "%s.temp", nombreArchivo);
    ofstream salida(archivoTemp, ios::binary);
    
    if (!salida.is_open()) {
        cout << " Error al crear archivo temporal" << endl;
        return false;
    }
    
    // Leer header original
    ArchivoHeader header = leerHeader(nombreArchivo, false);
    int registrosOriginales = header.cantidadRegistros;
    int registrosActivosOriginales = header.registrosActivos;
    
    // Reiniciar contadores para el nuevo archivo
    header.cantidadRegistros = 0;
    header.registrosActivos = 0;
    header.fechaUltimaModificacion = time(nullptr);
    
    // Escribir nuevo header
    salida.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    
    // Copiar solo registros activos
    const char* tipo = determinarTipoArchivo(nombreArchivo);
    ifstream entrada(nombreArchivo, ios::binary);
    entrada.seekg(sizeof(ArchivoHeader)); // Saltar header
    
    int registrosCopiados = 0;
    
    if (strcmp(tipo, "PACIENTES") == 0) {
        Paciente paciente;
        for (int i = 0; i < registrosOriginales; i++) {
            entrada.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente));
            if (entrada.gcount() == sizeof(Paciente) && !paciente.isEliminado()) {
                salida.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
                registrosCopiados++;
            }
        }
    } else if (strcmp(tipo, "DOCTORES") == 0) {
        Doctor doctor;
        for (int i = 0; i < registrosOriginales; i++) {
            entrada.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor));
            if (entrada.gcount() == sizeof(Doctor) && !doctor.isEliminado()) {
                salida.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
                registrosCopiados++;
            }
        }
    } else if (strcmp(tipo, "CITAS") == 0) {
        Cita cita;
        for (int i = 0; i < registrosOriginales; i++) {
            entrada.read(reinterpret_cast<char*>(&cita), sizeof(Cita));
            if (entrada.gcount() == sizeof(Cita) && !cita.isEliminado()) {
                salida.write(reinterpret_cast<const char*>(&cita), sizeof(Cita));
                registrosCopiados++;
            }
        }
    }
    
    entrada.close();
    salida.close();
    
    // Actualizar header con nuevos contadores
    header.cantidadRegistros = registrosCopiados;
    header.registrosActivos = registrosCopiados;
    
    fstream archivoTempActualizar(archivoTemp, ios::binary | ios::in | ios::out);
    if (archivoTempActualizar.is_open()) {
        archivoTempActualizar.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
        archivoTempActualizar.close();
    }
    
    // Reemplazar archivo original
    remove(nombreArchivo);
    if (rename(archivoTemp, nombreArchivo) != 0) {
        cout << " Error al reemplazar archivo original" << endl;
        return false;
    }
    
    cout << "   Registros eliminados limpiados" << endl;
    cout << "   Registros antes: " << registrosOriginales << " (" << registrosActivosOriginales << " activos)" << endl;
    cout << "   Registros después: " << registrosCopiados << " (todos activos)" << endl;
    cout << "   Espacio liberado: " << (registrosOriginales - registrosCopiados) << " registros" << endl;
    
    return true;
}

bool GestorArchivos::realizarRespaldoCompleto() {
    cout << "\n=== REALIZANDO RESPALDO COMPLETO ===" << endl;
    
    // Crear directorio de respaldos (usar backslash para Windows)
    crearDirectorioSiNoExiste("backups");
    
    // Timestamp para el respaldo
    time_t ahora = time(nullptr);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&ahora));
    
    char directorioRespaldo[100];
    sprintf(directorioRespaldo, "backups\\respaldo_%s", timestamp);  // Usar backslash
    crearDirectorioSiNoExiste(directorioRespaldo);
    
    // Lista de archivos a respaldar
    const char* archivos[] = {
        RUTA_PACIENTES,
        RUTA_DOCTORES,
        RUTA_CITAS,
        RUTA_HISTORIALES,
        RUTA_HOSPITAL
    };
    const int numArchivos = 5;
    
    bool todosExitosos = true;
    int respaldados = 0;
    
    for (int i = 0; i < numArchivos; i++) {
        const char* archivo = archivos[i];
        if (archivoExiste(archivo)) {
            // Obtener solo el nombre del archivo (sin ruta)
            // Buscar tanto backslash como slash
            const char* nombreArchivo = archivo;
            const char* ultimoBackslash = strrchr(archivo, '\\');
            const char* ultimoSlash = strrchr(archivo, '/');
            
            // Tomar el último separador encontrado
            if (ultimoBackslash != nullptr) {
                nombreArchivo = ultimoBackslash + 1;
            } else if (ultimoSlash != nullptr) {
                nombreArchivo = ultimoSlash + 1;
            }
            // Si no hay separador, nombreArchivo ya tiene el valor correcto
            
            char nombreDestino[150];
            sprintf(nombreDestino, "%s\\%s", directorioRespaldo, nombreArchivo);  // Usar backslash
            
            if (copiarArchivo(archivo, nombreDestino)) {
                cout << "  Respaldado: " << archivo << " -> " << nombreDestino << endl;
                respaldados++;
            } else {
                cout << "  Error al respaldar: " << archivo << endl;
                todosExitosos = false;
            }
        } else {
            cout << "  No existe: " << archivo << " (omitido)" << endl;
        }
    }
    
    if (respaldados > 0) {
        cout << "\n  Respaldo completado: " << respaldados 
             << " archivos guardados en: " << directorioRespaldo << endl;
    } else {
        cout << "\n  No se respaldo ningun archivo" << endl;
    }
    
    return todosExitosos;
}


void GestorArchivos::mostrarInformacionArchivos() {
    cout << "\n=== INFORMACION DE ARCHIVOS DEL SISTEMA ===" << endl;
    
    const char* archivos[] = {
        RUTA_PACIENTES,
        RUTA_DOCTORES,
        RUTA_CITAS,
        RUTA_HISTORIALES,
        RUTA_HOSPITAL
    };
    const int numArchivos = 5;
    
    for (int i = 0; i < numArchivos; i++) {
        const char* archivo = archivos[i];
        
        // Obtener nombre del archivo sin ruta
        const char* nombreArchivo = archivo;
        const char* ultimoBackslash = strrchr(archivo, '\\');
        const char* ultimoSlash = strrchr(archivo, '/');
        
        if (ultimoBackslash != nullptr) {
            nombreArchivo = ultimoBackslash + 1;
        } else if (ultimoSlash != nullptr) {
            nombreArchivo = ultimoSlash + 1;
        }
        
        cout << "\n--- " << nombreArchivo << " ---" << endl;
        
        if (archivoExiste(archivo)) {
            ArchivoHeader header = leerHeader(archivo, false);
            
            cout << "Tipo: " << header.tipoArchivo << endl;
            cout << "Estado: " << (validarHeader(header) ? "VALIDO" : "CORRUPTO") << endl;
            cout << "Registros: " << header.registrosActivos << " activos de " 
                 << header.cantidadRegistros << " totales" << endl;
            cout << "Proximo ID: " << header.proximoID << endl;
            
            // Calcular tamaño
            ifstream file(archivo, ios::binary | ios::ate);
            streampos tamanio = file.tellg();
            cout << "Tamaño: " << tamanio << " bytes" << endl;
            file.close();
            
            // Mostrar fechas
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&header.fechaCreacion));
            cout << "Creado: " << buffer << endl;
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&header.fechaUltimaModificacion));
            cout << "Modificado: " << buffer << endl;
            
            // Eficiencia de almacenamiento
            if (header.cantidadRegistros > 0) {
                double eficiencia = (double)header.registrosActivos / header.cantidadRegistros * 100;
                cout << "Eficiencia: " << eficiencia << "%" << endl;
            }
            
            // Espacio disponible
            if (tamanio > 0) {
                long tamanioEsperado = sizeof(ArchivoHeader);
                const char* tipo = determinarTipoArchivo(archivo);
                
                if (strcmp(tipo, "PACIENTES") == 0) {
                    tamanioEsperado += header.cantidadRegistros * sizeof(Paciente);
                } else if (strcmp(tipo, "DOCTORES") == 0) {
                    tamanioEsperado += header.cantidadRegistros * sizeof(Doctor);
                } else if (strcmp(tipo, "CITAS") == 0) {
                    tamanioEsperado += header.cantidadRegistros * sizeof(Cita);
                } else if (strcmp(tipo, "HOSPITAL") == 0) {
                    tamanioEsperado += sizeof(Hospital);
                }
                
                if (tamanio == tamanioEsperado) {
                    cout << "Integridad: OK" << endl;
                } else {
                    cout << "Integridad: INCONSISTENTE (esperado: " 
                         << tamanioEsperado << " bytes)" << endl;
                }
            }
        } else {
            cout << "Estado: NO EXISTE" << endl;
        }
    }
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

bool GestorArchivos::reconstruirArchivo(const char* nombreArchivo) {
    cout << "\n=== RECONSTRUYENDO ARCHIVO: " << nombreArchivo << " ===" << endl;
    
    if (!archivoExiste(nombreArchivo)) {
        cout << "Creando nuevo archivo: " << nombreArchivo << endl;
        const char* tipo = determinarTipoArchivo(nombreArchivo);
        return inicializarArchivo(nombreArchivo, tipo);
    }
    
    // Crear respaldo antes de reconstruir
    char nombreRespaldo[200];
    strcpy(nombreRespaldo, nombreArchivo);
    strcat(nombreRespaldo, ".backup_reconstruccion");
    
    if (!copiarArchivo(nombreArchivo, nombreRespaldo)) {
        cout << " No se pudo crear respaldo de seguridad" << endl;
        return false;
    }
    
    // Leer header actual
    ArchivoHeader header = leerHeader(nombreArchivo, false);
    
    if (!validarHeader(header)) {
        cout << "Header corrupto, reconstruyendo..." << endl;
        
        // Reconstruir header
        const char* tipo = determinarTipoArchivo(nombreArchivo);
        strncpy(header.tipoArchivo, tipo, sizeof(header.tipoArchivo) - 1);
        header.tipoArchivo[sizeof(header.tipoArchivo) - 1] = '\0';
        header.version = VERSION_ACTUAL;
        header.fechaCreacion = time(nullptr);
        header.fechaUltimaModificacion = header.fechaCreacion;
        
        // Para archivos de datos, reiniciar contadores
        if (strcmp(tipo, "HOSPITAL") != 0) {
            header.cantidadRegistros = 0;
            header.registrosActivos = 0;
            header.proximoID = 1;
        }
    }
    
    // Actualizar header reconstruido
    header.fechaUltimaModificacion = time(nullptr);
    
    if (actualizarHeader(nombreArchivo, header)) {
        cout << " Archivo reconstruido exitosamente" << endl;
        cout << " Respaldo guardado como: " << nombreRespaldo << endl;
        return true;
    }
    
    cout << " Error al reconstruir archivo" << endl;
    return false;
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


bool GestorArchivos::reindexarArchivo(const char* nombreArchivo) {
    cout << "\n=== REINDEXANDO: " << nombreArchivo << " ===" << endl;
    
    if (!archivoExiste(nombreArchivo)) {
        cout << " Archivo no existe" << endl;
        return false;
    }
    
    ArchivoHeader header = leerHeader(nombreArchivo, false);
    
    if (strcmp(header.tipoArchivo, "HOSPITAL") == 0) {
        cout << "  El archivo hospital no necesita reindexación" << endl;
        return true;
    }
    
    cout << "Reindexando " << header.cantidadRegistros << " registros..." << endl;
    
    // Para archivos de datos, reasignar IDs secuenciales
    // Esta es una implementación básica que actualiza el próximo ID

    int maxID = 0;
    
    ifstream entrada(nombreArchivo, ios::binary);
    entrada.seekg(sizeof(ArchivoHeader)); // Saltar header
    
    if (strcmp(header.tipoArchivo, "PACIENTES") == 0) {
        Paciente paciente;
        for (int i = 0; i < header.cantidadRegistros; i++) {
            entrada.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente));
            if (entrada.gcount() == sizeof(Paciente) && paciente.getId() > maxID) {
                maxID = paciente.getId();
            }
        }
    } else if (strcmp(header.tipoArchivo, "DOCTORES") == 0) {
        Doctor doctor;
        for (int i = 0; i < header.cantidadRegistros; i++) {
            entrada.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor));
            if (entrada.gcount() == sizeof(Doctor) && doctor.getId() > maxID) {
                maxID = doctor.getId();
            }
        }
    } else if (strcmp(header.tipoArchivo, "CITAS") == 0) {
        Cita cita;
        for (int i = 0; i < header.cantidadRegistros; i++) {
            entrada.read(reinterpret_cast<char*>(&cita), sizeof(Cita));
            if (entrada.gcount() == sizeof(Cita) && cita.getId() > maxID) {
                maxID = cita.getId();
            }
        }
    }
    
    entrada.close();
    
    // Actualizar próximo ID en el header
    header.proximoID = maxID + 1;
    header.fechaUltimaModificacion = time(nullptr);
    
    if (actualizarHeader(nombreArchivo, header)) {
        cout << "   Archivo reindexado exitosamente" << endl;
        cout << "   Nuevo próximo ID: " << header.proximoID << endl;
        return true;
    }
    
    cout << "Error al reindexar archivo" << endl;
    return false;
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

bool GestorArchivos::realizarRespaldo(const char* nombreArchivo) {
    if (!archivoExiste(nombreArchivo)) {
        cout << "Error: No se puede hacer respaldo de " << nombreArchivo << " (no existe)" << endl;
        return false;
    }

    // Crear directorio de backups si no existe
    crearDirectorioSiNoExiste("backups");

    // Crear nombre de respaldo con timestamp
    time_t ahora = time(nullptr);
    char nombreRespaldo[100];
    strftime(nombreRespaldo, sizeof(nombreRespaldo), "backups\\%Y%m%d_%H%M%S_", localtime(&ahora));  // Backslash
    
    // Obtener nombre del archivo sin ruta
    const char* nombreArchivoSimple = nombreArchivo;
    const char* ultimoBackslash = strrchr(nombreArchivo, '\\');
    const char* ultimoSlash = strrchr(nombreArchivo, '/');
    
    if (ultimoBackslash != nullptr) {
        nombreArchivoSimple = ultimoBackslash + 1;
    } else if (ultimoSlash != nullptr) {
        nombreArchivoSimple = ultimoSlash + 1;
    }
    
    strcat(nombreRespaldo, nombreArchivoSimple);

    // Copiar archivo
    ifstream origen(nombreArchivo, ios::binary);
    ofstream destino(nombreRespaldo, ios::binary);

    if (!origen.is_open() || !destino.is_open()) {
        cout << "Error: No se pudo crear respaldo" << endl;
        return false;
    }

    destino << origen.rdbuf();

    bool exito = destino.good();
    origen.close();
    destino.close();

    if (exito) {
        cout << "Respaldo creado: " << nombreRespaldo << endl;
    } else {
        cout << "Error al crear respaldo" << endl;
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
