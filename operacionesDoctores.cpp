#include "operacionesDoctores.hpp"
#include "../persistencia/GestorArchivos.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <iomanip>

using namespace std;

bool OperacionesDoctores::agregarDoctor(Doctor& doctor) {
    
    if (strlen(doctor.getNombre()) == 0 || 
        strlen(doctor.getApellido()) == 0) {
        cout << "Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }

    if (strlen(doctor.getCedula()) == 0) {
        cout << "Error: La cédula es obligatoria" << endl;
        return false;
    }

    if (strlen(doctor.getEspecialidad()) == 0) {
        cout << "Error: La especialidad es obligatoria" << endl;
        return false;
    }

    // Verificar cédula única
    if (existeDoctorConCedula(doctor.getCedula())) {
        cout << "Error: Ya existe un doctor con la cédula " << doctor.getCedula() << endl;
        return false;
    }

    // Obtener próximo ID
    ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "Error: No se puede leer header de doctores" << endl;
        return false;
    }

    doctor.setId(header.proximoID);

    // Guardar en archivo
    ofstream archivo("doctores.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de doctores" << endl;
        return false;
    }

    // Si archivo vacío, escribir header primero
    archivo.seekp(0, ios::end);
    if (archivo.tellp() == 0) {
        ArchivoHeader nuevoHeader;
        strcpy(nuevoHeader.tipoArchivo, "DOCTORES");
        nuevoHeader.version = 1;
        nuevoHeader.cantidadRegistros = 1;
        nuevoHeader.registrosActivos = 1;
        nuevoHeader.proximoID = doctor.getId() + 1;
        nuevoHeader.fechaCreacion = time(nullptr);
        nuevoHeader.fechaUltimaModificacion = nuevoHeader.fechaCreacion;

        archivo.seekp(0);
        archivo.write(reinterpret_cast<const char*>(&nuevoHeader), sizeof(ArchivoHeader));
    }

    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    bool exito = archivo.good();
    archivo.close();

    if (exito) {
        // Actualizar header
        header.cantidadRegistros++;
        header.registrosActivos++;
        header.proximoID++;
        header.fechaUltimaModificacion = time(nullptr);
        GestorArchivos::actualizarHeader("doctores.bin", header);

        cout << "DOCTOR AGREGADO EXITOSAMENTE" << endl;
        cout << "ID: " << doctor.getId() << endl;
        cout << "Nombre: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
        cout << "Especialidad: " << doctor.getEspecialidad() << endl;
        return true;
    }

    cout << "ERROR: No se pudo agregar el doctor" << endl;
    return false;
}

bool OperacionesDoctores::actualizarDoctor(const Doctor& doctor) {
    if (doctor.getId() <= 0) {
        cout << "Error: ID de doctor inválido" << endl;
        return false;
    }

    // Buscar posición del doctor
    int indice = buscarIndicePorID(doctor.getId());
    if (indice == -1) {
        cout << "Error: No se puede encontrar doctor con ID " << doctor.getId() << endl;
        return false;
    }

    // Abrir archivo en modo lectura/escritura
    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir doctores.bin para actualización" << endl;
        return false;
    }

    // Posicionarse en la ubicación exacta
    long posicion = calcularPosicion(indice);
    archivo.seekp(posicion);

    if (archivo.fail()) {
        cout << "Error: No se puede posicionar en archivo" << endl;
        archivo.close();
        return false;
    }

    // Escribir estructura completa
    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (exito) {
        cout << "Doctor ID " << doctor.getId() << " actualizado exitosamente" << endl;
    } else {
        cout << "Error al actualizar doctor ID " << doctor.getId() << endl;
    }

    return exito;
}

bool OperacionesDoctores::eliminarDoctor(int id, bool confirmar) {
    // Validaciones iniciales
    if (id <= 0) {
        cout << "Error: ID de doctor inválido" << endl;
        return false;
    }

    // Buscar doctor para mostrar información
    Doctor doctor = buscarPorID(id);
    if (doctor.getId() == -1) {
        cout << "Error: Doctor ID " << id << " no existe" << endl;
        return false;
    }

    if (doctor.isEliminado()) {
        cout << "Doctor ID " << id << " ya está eliminado" << endl;
        return false;
    }

    // Confirmación de eliminación
    if (confirmar) {
        cout << "ELIMINACIÓN DE DOCTOR - CONFIRMACIÓN REQUERIDA" << endl;
        cout << "Datos del doctor:" << endl;
        cout << "ID: " << doctor.getId() << endl;
        cout << "Nombre: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
        cout << "Cédula: " << doctor.getCedula() << endl;
        cout << "Especialidad: " << doctor.getEspecialidad() << endl;
        cout << "Pacientes asignados: " << doctor.getCantidadPacientes() << endl;

        cout << "¿Está seguro de eliminar este doctor? (s/n): ";
        char respuesta;
        cin >> respuesta;
        cin.ignore();

        if (tolower(respuesta) != 's') {
            cout << "Eliminación cancelada por el usuario" << endl;
            return false;
        }

        // Doble confirmación si tiene pacientes o citas
        if (doctor.getCantidadPacientes() > 0 || doctor.getCantidadCitas() > 0) {
            cout << "ADVERTENCIA: Este doctor tiene " << doctor.getCantidadPacientes() 
                 << " pacientes y " << doctor.getCantidadCitas() << " citas" << endl;
            cout << "¿Continuar con la eliminación? (s/n): ";
            cin >> respuesta;
            cin.ignore();

            if (tolower(respuesta) != 's') {
                cout << "Eliminación cancelada" << endl;
                return false;
            }
        }
    }

    // Proceder con eliminación lógica
    int indice = buscarIndicePorID(id);
    if (indice == -1) return false;

    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;

    long posicion = calcularPosicion(indice);

    // Leer y modificar
    Doctor temp;
    archivo.seekg(posicion);
    archivo.read(reinterpret_cast<char*>(&temp), sizeof(Doctor));

    temp.setEliminado(true);
    temp.setFechaModificacion(time(nullptr));

    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&temp), sizeof(Doctor));
    archivo.flush();

    bool exito = archivo.good();
    archivo.close();

    if (exito) {
        // Actualizar header
        ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.registrosActivos = max(0, header.registrosActivos - 1);
            header.fechaUltimaModificacion = time(nullptr);
            GestorArchivos::actualizarHeader("doctores.bin", header);
        }

        cout << "ELIMINACIÓN COMPLETADA" << endl;
        cout << "Doctor: Dr. " << temp.getNombre() << " " << temp.getApellido() << endl;
    } else {
        cout << "ERROR: No se pudo completar la eliminación" << endl;
    }

    return exito;
}

bool OperacionesDoctores::restaurarDoctor(int id) {
    int indice = buscarIndicePorID(id);
    if (indice == -1) {
        cout << "Error: No se puede encontrar doctor con ID " << id << endl;
        return false;
    }

    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;

    long posicion = calcularPosicion(indice);

    // Leer doctor
    Doctor doctor;
    archivo.seekg(posicion);
    archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor));

    if (!doctor.isEliminado()) {
        cout << "Doctor ID " << id << " no está eliminado" << endl;
        archivo.close();
        return false;
    }

    // Restaurar
    doctor.setEliminado(false);
    doctor.setFechaModificacion(time(nullptr));

    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    archivo.flush();

    bool exito = archivo.good();
    archivo.close();

    if (exito) {
        // Actualizar header
        ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.registrosActivos++;
            header.fechaUltimaModificacion = time(nullptr);
            GestorArchivos::actualizarHeader("doctores.bin", header);

            cout << "DOCTOR RESTAURADO" << endl;
            cout << "Doctor ID " << id << " reactivado exitosamente" << endl;
        } else {
            cout << "DOCTOR RESTAURADO" << endl;
            cout << "Doctor ID " << id << " reactivado exitosamente" << endl;
            cout << "Nota: No se pudo actualizar el header del archivo" << endl;
        }
    }

    return exito;
}

Doctor OperacionesDoctores::buscarPorID(int id) {
    Doctor doctor;
    
    if (id <= 0) {
        cout << "Error: ID de doctor inválido" << endl;
        return doctor;
    }

    // Abrir doctores.bin
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir doctores.bin" << endl;
        return doctor;
    }

    // Leer header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se puede leer header de doctores.bin" << endl;
        archivo.close();
        return doctor;
    }

    // Verificar si hay registros
    if (header.cantidadRegistros == 0) {
        cout << "No hay doctores registrados" << endl;
        archivo.close();
        return doctor;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);

    // Búsqueda secuencial
    bool encontrado = false;
    int doctoresLeidos = 0;

    while (doctoresLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor))) {
        if (doctor.getId() == id && !doctor.isEliminado()) {
            encontrado = true;
            break;
        }
        doctoresLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        Doctor vacio;
        cout << "Doctor con ID " << id << " no encontrado" << endl;
        return vacio;
    } else {
        cout << "Doctor encontrado: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
    }

    return doctor;
}

Doctor OperacionesDoctores::buscarPorCedula(const char* cedula) {
    Doctor doctor;
    
    if (!cedula || strlen(cedula) == 0) {
        cout << "Error: Cédula inválida" << endl;
        return doctor;
    }

    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de doctores" << endl;
        return doctor;
    }

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se puede leer header de doctores" << endl;
        archivo.close();
        return doctor;
    }

    // Buscar doctor por cédula
    bool encontrado = false;
    int doctoresLeidos = 0;

    while (doctoresLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor))) {
        if (strcmp(doctor.getCedula(), cedula) == 0 && !doctor.isEliminado()) {
            encontrado = true;
            break;
        }
        doctoresLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        Doctor vacio;
        cout << "Doctor con cédula " << cedula << " no encontrado" << endl;
        return vacio;
    } else {
        cout << "Doctor encontrado: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
    }

    return doctor;
}

int OperacionesDoctores::buscarPorEspecialidad(const char* especialidad, Doctor* resultados, int maxResultados) {
    if (!especialidad || strlen(especialidad) == 0 || !resultados || maxResultados <= 0) {
        cout << "Error: Parámetros de búsqueda inválidos" << endl;
        return 0;
    }

    // Leer todos los doctores activos
    const int MAX_DOCTORES = 100;
    Doctor todosDoctores[MAX_DOCTORES];
    int cantidadTotal = leerTodosDoctores(todosDoctores, MAX_DOCTORES, true);

    if (cantidadTotal == 0) {
        cout << "No hay doctores registrados" << endl;
        return 0;
    }

    // Realizar búsqueda
    int encontrados = 0;

    for (int i = 0; i < cantidadTotal && encontrados < maxResultados; i++) {
        Doctor* d = &todosDoctores[i];

        if (strcmp(d->getEspecialidad(), especialidad) == 0) {
            resultados[encontrados] = *d;
            encontrados++;
        }
    }

    // Mostrar resultados
    if (encontrados > 0) {
        cout << "Encontrados " << encontrados << " doctores de " << especialidad << endl;
    } else {
        cout << "No se encontraron doctores de " << especialidad << endl;
    }

    return encontrados;
}

int OperacionesDoctores::buscarPorNombre(const char* nombre, Doctor* resultados, int maxResultados) {
    if (!nombre || strlen(nombre) == 0 || !resultados || maxResultados <= 0) {
        cout << "Error: Parámetros de búsqueda inválidos" << endl;
        return 0;
    }

    // Leer todos los doctores activos
    const int MAX_DOCTORES = 100;
    Doctor todosDoctores[MAX_DOCTORES];
    int cantidadTotal = leerTodosDoctores(todosDoctores, MAX_DOCTORES, true);

    if (cantidadTotal == 0) {
        cout << "No hay doctores registrados" << endl;
        return 0;
    }

    // Convertir nombre de búsqueda a minúsculas
    char* nombreLower = new char[strlen(nombre) + 1];
    for (int i = 0; nombre[i]; i++) {
        nombreLower[i] = tolower(nombre[i]);
    }
    nombreLower[strlen(nombre)] = '\0';

    // Realizar búsqueda
    int encontrados = 0;

    for (int i = 0; i < cantidadTotal && encontrados < maxResultados; i++) {
        Doctor* d = &todosDoctores[i];

        // Convertir nombre del doctor a minúsculas
        char nombreDoctorLower[50];
        strcpy(nombreDoctorLower, d->getNombre());
        for (int j = 0; nombreDoctorLower[j]; j++) {
            nombreDoctorLower[j] = tolower(nombreDoctorLower[j]);
        }

        // Buscar coincidencia parcial
        if (strstr(nombreDoctorLower, nombreLower) != nullptr) {
            resultados[encontrados] = *d;
            encontrados++;
        }
    }

    delete[] nombreLower;

    // Mostrar resultados
    if (encontrados > 0) {
        cout << "Encontrados " << encontrados << " doctores con '" << nombre << "'" << endl;
    } else {
        cout << "No se encontraron doctores con '" << nombre << "'" << endl;
    }

    return encontrados;
}

void OperacionesDoctores::listarDoctores(bool mostrarEliminados) {
    // Leer header para saber cuántos doctores hay
    ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");

    if (strcmp(header.tipoArchivo, "INVALIDO") == 0 ||
        header.registrosActivos == 0) {
        cout << "No hay doctores registrados." << endl;
        return;
    }

    cout << "\nLISTA DE DOCTORES (" << header.registrosActivos << "):" << endl;
    cout << "+----------------------------------------------------------------+" << endl;
    cout << "|  ID  |       NOMBRE COMPLETO    |  ESPECIALIDAD  | COSTO | ESTADO |" << endl;
    cout << "+------+--------------------------+----------------+-------+--------+" << endl;

    // Abrir archivo y leer doctores
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de doctores" << endl;
        return;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Doctor d;
    int doctoresMostrados = 0;

    while (archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) &&
           doctoresMostrados < header.registrosActivos) {
        // Solo mostrar doctores no eliminados (o todos si se solicita)
        if (mostrarEliminados || !d.isEliminado()) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "Dr. %s %s", d.getNombre(), d.getApellido());

            // Truncar nombre si es muy largo
            if (strlen(nombreCompleto) > 22) {
                nombreCompleto[19] = '.';
                nombreCompleto[20] = '.';
                nombreCompleto[21] = '.';
                nombreCompleto[22] = '\0';
            }

            // Truncar especialidad si es muy larga
            char especialidadMostrar[16];
            strncpy(especialidadMostrar, d.getEspecialidad(), sizeof(especialidadMostrar) - 1);
            especialidadMostrar[sizeof(especialidadMostrar) - 1] = '\0';
            if (strlen(d.getEspecialidad()) > 14) {
                especialidadMostrar[11] = '.';
                especialidadMostrar[12] = '.';
                especialidadMostrar[13] = '.';
                especialidadMostrar[14] = '\0';
            }

            char estado[10];
            if (d.isEliminado()) {
                strcpy(estado, "ELIMINADO");
            } else if (d.isDisponible()) {
                strcpy(estado, "ACTIVO");
            } else {
                strcpy(estado, "INACTIVO");
            }

            printf("| %4d | %-24s | %-14s | %5.0f | %-6s |\n", 
                   d.getId(), nombreCompleto, especialidadMostrar, d.getCostoConsulta(), estado);

            doctoresMostrados++;
        }
    }

    archivo.close();

    cout << "+----------------------------------------------------------------+" << endl;

    // Mostrar estadísticas adicionales
    if (doctoresMostrados > 0) {
        cout << "Total mostrados: " << doctoresMostrados << " doctor(es)" << endl;
        cout << "Registros en archivo: " << header.cantidadRegistros << endl;
    }
}

void OperacionesDoctores::listarDoctoresEliminados() {
    ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");

    if (header.registrosActivos == header.cantidadRegistros) {
        cout << "No hay doctores eliminados" << endl;
        return;
    }

    cout << "DOCTORES ELIMINADOS (" << (header.cantidadRegistros - header.registrosActivos) << "):" << endl;
    cout << "=========================================" << endl;

    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) return;

    archivo.seekg(sizeof(ArchivoHeader));

    Doctor d;
    int contador = 0;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor));
        if (d.isEliminado()) {
            cout << "ID: " << d.getId() << " | Dr. " << d.getNombre() << " " << d.getApellido();

            char fechaStr[20];
            strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d", localtime(&d.getFechaModificacion()));
            cout << " | Eliminado: " << fechaStr << endl;

            contador++;
        }
    }

    archivo.close();
    cout << "=========================================" << endl;
    cout << "Total: " << contador << " doctores eliminados" << endl;
}

int OperacionesDoctores::contarDoctores() {
    ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");
    return header.registrosActivos;
}

int OperacionesDoctores::contarDoctoresEliminados() {
    ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");
    return header.cantidadRegistros - header.registrosActivos;
}

bool OperacionesDoctores::existeDoctorConCedula(const char* cedula) {
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Doctor doctor;
    while (archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor))) {
        if (!doctor.isEliminado() && strcmp(doctor.getCedula(), cedula) == 0) {
            archivo.close();
            return true;
        }
    }

    archivo.close();
    return false;
}

int OperacionesDoctores::leerTodosDoctores(Doctor* doctores, int maxDoctores, bool soloDisponibles) {
    if (!doctores || maxDoctores <= 0) return 0;

    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de doctores" << endl;
        return 0;
    }

    // Leer header
    ArchivoHeader header = GestorArchivos::leerHeader("doctores.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        archivo.close();
        return 0;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    int contador = 0;
    Doctor temp;

    while (contador < maxDoctores &&
           archivo.read(reinterpret_cast<char*>(&temp), sizeof(Doctor))) {
        // Usar campo 'disponible' para filtrar
        if (!soloDisponibles || temp.isDisponible()) {
            doctores[contador] = temp;
            contador++;
        }
    }

    archivo.close();
    return contador;
}

// Funciones privadas helper
int OperacionesDoctores::buscarIndicePorID(int id) {
    if (id <= 0) return -1;

    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) return -1;

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Búsqueda secuencial por índice
    Doctor d;
    int indice = 0;

    while (indice < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor))) {
        if (d.getId() == id) {
            archivo.close();
            return indice;
        }
        indice++;
    }

    archivo.close();
    return -1;
}

long OperacionesDoctores::calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Doctor));
}