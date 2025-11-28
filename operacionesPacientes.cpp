#include "../include/operacionesPacientes.hpp"
#include "../include/GestorArchivos.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>

using namespace std;

bool OperacionesPacientes::agregarPaciente(Paciente& paciente) {
    // Validaciones básicas
    if (strlen(paciente.getNombre()) == 0 || 
        strlen(paciente.getApellido()) == 0) {
        cout << "Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }

    if (strlen(paciente.getCedula()) == 0) {
        cout << "Error: La cédula es obligatoria" << endl;
        return false;
    }

    // Verificar cédula única
    if (existePacienteConCedula(paciente.getCedula())) {
        cout << "Error: Ya existe un paciente con la cédula " << paciente.getCedula() << endl;
        return false;
    }

    // Obtener próximo ID
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "Error: No se puede leer header de pacientes" << endl;
        return false;
    }

    paciente.setId(header.proximoID);

    // Guardar en archivo
    ofstream archivo("datos/pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de pacientes" << endl;
        return false;
    }

    // Si archivo vacío, escribir header primero
    archivo.seekp(0, ios::end);
    if (archivo.tellp() == 0) {
        ArchivoHeader nuevoHeader;
        strcpy(nuevoHeader.tipoArchivo, "PACIENTES");
        nuevoHeader.version = 1;
        nuevoHeader.cantidadRegistros = 1;
        nuevoHeader.registrosActivos = 1;
        nuevoHeader.proximoID = paciente.getId() + 1;
        nuevoHeader.fechaCreacion = time(nullptr);
        nuevoHeader.fechaUltimaModificacion = nuevoHeader.fechaCreacion;

        archivo.seekp(0);
        archivo.write(reinterpret_cast<const char*>(&nuevoHeader), sizeof(ArchivoHeader));
    }

    archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
    bool exito = archivo.good();
    archivo.close();

    if (exito) {
        // Actualizar header
        header.cantidadRegistros++;
        header.registrosActivos++;
        header.proximoID++;
        header.fechaUltimaModificacion = time(nullptr);
        GestorArchivos::actualizarHeader("datos/pacientes.bin", header);

        cout << "PACIENTE AGREGADO EXITOSAMENTE" << endl;
        cout << "ID: " << paciente.getId() << endl;
        cout << "Nombre: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
        return true;
    }

    cout << "ERROR: No se pudo agregar el paciente" << endl;
    return false;
}

bool OperacionesPacientes::actualizarPaciente(const Paciente& paciente) {
    if (paciente.getId() <= 0) {
        cout << "Error: ID de paciente inválido" << endl;
        return false;
    }

    // Buscar posición del paciente
    int indice = buscarIndicePorID(paciente.getId());
    if (indice == -1) {
        cout << "Error: No se puede encontrar paciente con ID " << paciente.getId() << endl;
        return false;
    }

    // Abrir archivo en modo lectura/escritura
    fstream archivo("datos/pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir pacientes.bin para actualización" << endl;
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
    archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (exito) {
        cout << "Paciente ID " << paciente.getId() << " actualizado exitosamente" << endl;
    } else {
        cout << "Error al actualizar paciente ID " << paciente.getId() << endl;
    }

    return exito;
}

Paciente OperacionesPacientes::buscarPorID(int id) {
    Paciente paciente;
    
    if (id <= 0) {
        cout << "Error: ID de paciente inválido" << endl;
        return paciente;
    }

    // Abrir pacientes.bin
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir pacientes.bin" << endl;
        return paciente;
    }

    // Leer header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se puede leer header de pacientes.bin" << endl;
        archivo.close();
        return paciente;
    }

    // Verificar si hay registros
    if (header.cantidadRegistros == 0) {
        cout << "No hay pacientes registrados" << endl;
        archivo.close();
        return paciente;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);

    // Búsqueda secuencial
    bool encontrado = false;
    int pacientesLeidos = 0;

    while (pacientesLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        if (paciente.getId() == id && !paciente.isEliminado()) {
            encontrado = true;
            break;
        }
        pacientesLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        Paciente vacio;
        cout << "Paciente con ID " << id << " no encontrado" << endl;
        return vacio;
    } else {
        cout << "Paciente encontrado: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    }

    return paciente;
}

Paciente OperacionesPacientes::buscarPorCedula(const char* cedula) {
    Paciente paciente;
    
    if (!cedula || strlen(cedula) == 0) {
        cout << "Error: Cédula inválida" << endl;
        return paciente;
    }

    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de pacientes" << endl;
        return paciente;
    }

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se puede leer header de pacientes" << endl;
        archivo.close();
        return paciente;
    }

    // Buscar paciente por cédula
    bool encontrado = false;
    int pacientesLeidos = 0;

    while (pacientesLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        if (strcmp(paciente.getCedula(), cedula) == 0 && !paciente.isEliminado()) {
            encontrado = true;
            break;
        }
        pacientesLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        Paciente vacio;
        cout << "Paciente con cédula " << cedula << " no encontrado" << endl;
        return vacio;
    } else {
        cout << "Paciente encontrado: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    }

    return paciente;
}

void OperacionesPacientes::listarPacientes(bool mostrarEliminados) {
    // Leer header para saber cuántos pacientes hay
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");

    if (strcmp(header.tipoArchivo, "INVALIDO") == 0 ||
        header.registrosActivos == 0) {
        cout << "No hay pacientes registrados." << endl;
        return;
    }

    cout << "\nLISTA DE PACIENTES (" << header.registrosActivos << "):" << endl;
    cout << "+-------------------------------------------------------------------+" << endl;
    cout << "|  ID  |       NOMBRE COMPLETO    |    CÉDULA    | EDAD | CONSULTAS |" << endl;
    cout << "+------+--------------------------+--------------+------+-----------+" << endl;

    // Abrir archivo y leer pacientes
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de pacientes" << endl;
        return;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Paciente p;
    int pacientesMostrados = 0;

    while (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) &&
           pacientesMostrados < header.registrosActivos) {
        // Solo mostrar pacientes no eliminados (o todos si se solicita)
        if (mostrarEliminados || !p.isEliminado()) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", p.getNombre(), p.getApellido());

            // Truncar nombre si es muy largo
            if (strlen(nombreCompleto) > 22) {
                nombreCompleto[19] = '.';
                nombreCompleto[20] = '.';
                nombreCompleto[21] = '.';
                nombreCompleto[22] = '\0';
            }

            printf("| %4d | %-24s | %-12s | %4d | %10d |\n", 
                   p.getId(), nombreCompleto, p.getCedula(), p.getEdad(), p.getCantidadConsultas());

            pacientesMostrados++;
        }
    }

    archivo.close();

    cout << "+-------------------------------------------------------------------+" << endl;

    // Mostrar estadísticas adicionales
    if (pacientesMostrados > 0) {
        cout << "Total mostrados: " << pacientesMostrados << " paciente(s)" << endl;
        cout << "Registros en archivo: " << header.cantidadRegistros << endl;
    }
}

bool OperacionesPacientes::existePacienteConCedula(const char* cedula) {
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Paciente paciente;
    while (archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        if (!paciente.isEliminado() && strcmp(paciente.getCedula(), cedula) == 0) {
            archivo.close();
            return true;
        }
    }

    archivo.close();
    return false;
}

// Funciones privadas helper
int OperacionesPacientes::buscarIndicePorID(int id) {
    if (id <= 0) return -1;

    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) return -1;

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Búsqueda secuencial por índice
    Paciente p;
    int indice = 0;

    while (indice < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente))) {
        if (p.getId() == id) {
            archivo.close();
            return indice;
        }
        indice++;
    }

    archivo.close();
    return -1;
}

long OperacionesPacientes::calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
}

// Implementaciones simples de funciones no críticas
bool OperacionesPacientes::eliminarPaciente(int id, bool confirmar) {
    cout << "Eliminar paciente - Funcionalidad en desarrollo" << endl;
    return false;
}

bool OperacionesPacientes::restaurarPaciente(int id) {
    cout << "Restaurar paciente - Funcionalidad en desarrollo" << endl;
    return false;
}

int OperacionesPacientes::buscarPorNombre(const char* nombre, Paciente* resultados, int maxResultados) {
    cout << "Buscar por nombre - Funcionalidad en desarrollo" << endl;
    return 0;
}

void OperacionesPacientes::listarPacientesEliminados() {
    cout << "Listar eliminados - Funcionalidad en desarrollo" << endl;
}

int OperacionesPacientes::contarPacientes() {
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
    return header.registrosActivos;
}

int OperacionesPacientes::contarPacientesEliminados() {
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
    return header.cantidadRegistros - header.registrosActivos;
}

int OperacionesPacientes::leerTodosPacientes(Paciente* pacientes, int maxPacientes, bool soloActivos) {
    // Implementación básica
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int contador = 0;
    Paciente temp;

    while (contador < maxPacientes && contador < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente))) {
        if (!soloActivos || !temp.isEliminado()) {
            pacientes[contador] = temp;
            contador++;
        }
    }

    archivo.close();
    return contador;
}
