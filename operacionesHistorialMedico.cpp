#include "operacionesHistorial.hpp"
#include "../persistencia/GestorArchivos.hpp"
#include "../pacientes/operacionesPacientes.hpp"
#include "../doctores/operacionesDoctores.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>

using namespace std;

// 🔥 NUEVO: Función helper para formatear fecha - similar a la que tenías en el código original
const char* formatearFecha(time_t tiempo) {
    static char buffer[20];  // static para que persista después de retornar
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", localtime(&tiempo));
    return buffer;
}

bool OperacionesHistorial::agregarConsulta(HistorialMedico& consulta) {
    // Validaciones básicas
    if (consulta.getPacienteID() <= 0) {
        cout << "Error: ID de paciente inválido" << endl;
        return false;
    }

    if (consulta.getIdDoctor() <= 0) {
        cout << "Error: ID de doctor inválido" << endl;
        return false;
    }

    if (strlen(consulta.getDiagnostico()) == 0) {
        cout << "Error: El diagnóstico es obligatorio" << endl;
        return false;
    }

    // Verificar que el paciente exista
    Paciente paciente = OperacionesPacientes::buscarPorID(consulta.getPacienteID());
    if (paciente.getId() == -1) {
        cout << "Error: No existe paciente con ID " << consulta.getPacienteID() << endl;
        return false;
    }

    if (paciente.isEliminado()) {
        cout << "Error: No se puede agregar consulta a paciente eliminado" << endl;
        return false;
    }

    // Obtener próximo ID
    ArchivoHeader header = GestorArchivos::leerHeader("historiales.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        // Archivo no existe, inicializarlo
        if (!GestorArchivos::inicializarArchivo("historiales.bin", "HISTORIALES")) {
            cout << "Error: No se puede inicializar archivo de historiales" << endl;
            return false;
        }
        header = GestorArchivos::leerHeader("historiales.bin");
    }

    consulta.setId(header.proximoID);

    // Guardar en archivo
    ofstream archivo("historiales.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir historiales.bin para guardar" << endl;
        return false;
    }

    archivo.write(reinterpret_cast<const char*>(&consulta), sizeof(HistorialMedico));
    bool exito = archivo.good();
    archivo.close();

    if (!exito) {
        cout << "Error: No se pudo guardar la consulta" << endl;
        return false;
    }

    // Actualizar header
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    header.fechaUltimaModificacion = time(nullptr);
    GestorArchivos::actualizarHeader("historiales.bin", header);

    // Actualizar contador de consultas del paciente
    paciente.setCantidadConsultas(paciente.getCantidadConsultas() + 1);
    OperacionesPacientes::actualizarPaciente(paciente);

    cout << "CONSULTA AGREGADA EXITOSAMENTE" << endl;
    cout << "ID Consulta: " << consulta.getId() << endl;
    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    cout << "Diagnóstico: " << consulta.getDiagnostico() << endl;
    cout << "Costo: $" << fixed << setprecision(2) << consulta.getCosto() << endl;

    return true;
}

bool OperacionesHistorial::agregarConsultaAlHistorial(int pacienteID, HistorialMedico nuevaConsulta) {
    // Esta función maneja la lista enlazada de consultas del paciente
    Paciente paciente = OperacionesPacientes::buscarPorID(pacienteID);
    if (paciente.getId() == -1) {
        cout << "Error: Paciente ID " << pacienteID << " no encontrado" << endl;
        return false;
    }

    if (paciente.isEliminado()) {
        cout << "Error: No se puede agregar consulta a paciente eliminado" << endl;
        return false;
    }

    // Asignar ID a la nueva consulta
    ArchivoHeader headerHistorial = GestorArchivos::leerHeader("historiales.bin");
    nuevaConsulta.setId(headerHistorial.proximoID);
    nuevaConsulta.setPacienteID(pacienteID);
    nuevaConsulta.setEliminado(false);
    nuevaConsulta.setFechaRegistro(time(nullptr));

    // Si es su primera consulta
    if (paciente.getPrimerConsultaID() == -1) {
        nuevaConsulta.setSiguienteConsultaID(-1);  // Será la única consulta
        paciente.setPrimerConsultaID(nuevaConsulta.getId());
        cout << "Primera consulta para el paciente" << endl;
    } else {
        // Buscar última consulta en la lista enlazada
        HistorialMedico ultimaConsulta = buscarUltimaConsulta(pacienteID);
        if (ultimaConsulta.getId() == -1) {
            cout << "Error: No se puede encontrar la última consulta" << endl;
            return false;
        }

        // Actualizar la última consulta para que apunte a la nueva
        ultimaConsulta.setSiguienteConsultaID(nuevaConsulta.getId());
        if (!actualizarConsulta(ultimaConsulta)) {
            cout << "Error: No se puede actualizar última consulta" << endl;
            return false;
        }

        // La nueva consulta será la última
        nuevaConsulta.setSiguienteConsultaID(-1);
        cout << "Consulta #" << (paciente.getCantidadConsultas() + 1) 
             << " para el paciente" << endl;
    }

    // Agregar nueva consulta a historiales.bin
    if (!agregarConsulta(nuevaConsulta)) {
        cout << "Error: No se puede guardar la nueva consulta" << endl;
        return false;
    }

    // Actualizar contador del paciente
    paciente.setCantidadConsultas(paciente.getCantidadConsultas() + 1);
    if (!OperacionesPacientes::actualizarPaciente(paciente)) {
        cout << "Error: No se puede actualizar datos del paciente" << endl;
        return false;
    }

    cout << "CONSULTA AGREGADA AL HISTORIAL EXITOSAMENTE" << endl;
    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    cout << "Consulta ID: " << nuevaConsulta.getId() << endl;

    return true;
}

bool OperacionesHistorial::actualizarConsulta(const HistorialMedico& consulta) {
    if (consulta.getId() <= 0) {
        cout << "Error: ID de consulta inválido" << endl;
        return false;
    }

    // Buscar posición de la consulta
    int indice = buscarIndicePorID(consulta.getId());
    if (indice == -1) {
        cout << "Error: No se puede encontrar consulta con ID " << consulta.getId() << endl;
        return false;
    }

    // Abrir archivo en modo lectura/escritura
    fstream archivo("historiales.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir historiales.bin para actualización" << endl;
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
    archivo.write(reinterpret_cast<const char*>(&consulta), sizeof(HistorialMedico));
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (exito) {
        cout << "Consulta ID " << consulta.getId() << " actualizada exitosamente" << endl;
    } else {
        cout << "Error al actualizar consulta ID " << consulta.getId() << endl;
    }

    return exito;
}

bool OperacionesHistorial::eliminarConsulta(int id) {
    // Validar ID
    if (id <= 0) {
        cout << "Error: ID de consulta inválido" << endl;
        return false;
    }

    // Buscar la consulta
    HistorialMedico consulta = buscarPorID(id);
    if (consulta.getId() == -1) {
        cout << "Error: Consulta con ID " << id << " no encontrada" << endl;
        return false;
    }

    if (consulta.isEliminado()) {
        cout << "La consulta ya está eliminada" << endl;
        return false;
    }

    // Marcar como eliminada (eliminación lógica)
    consulta.setEliminado(true);

    // Guardar cambios
    bool exito = actualizarConsulta(consulta);

    if (exito) {
        // Actualizar header
        ArchivoHeader header = GestorArchivos::leerHeader("historiales.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.registrosActivos = max(0, header.registrosActivos - 1);
            header.fechaUltimaModificacion = time(nullptr);
            GestorArchivos::actualizarHeader("historiales.bin", header);
        }

        cout << "CONSULTA ELIMINADA EXITOSAMENTE" << endl;
        cout << "ID Consulta: " << consulta.getId() << endl;
    } else {
        cout << "Error: No se pudo eliminar la consulta" << endl;
    }

    return exito;
}

HistorialMedico OperacionesHistorial::buscarPorID(int id) {
    if (id <= 0) {
        return crearConsultaVacia();
    }

    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) {
        return crearConsultaVacia();
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Buscar consulta por ID
    archivo.seekg(sizeof(ArchivoHeader));
    HistorialMedico consulta;

    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (consulta.getId() == id && !consulta.isEliminado()) {
            archivo.close();
            return consulta;
        }
    }

    archivo.close();
    return crearConsultaVacia();
}

HistorialMedico OperacionesHistorial::buscarUltimaConsulta(int pacienteID) {
    Paciente paciente = OperacionesPacientes::buscarPorID(pacienteID);
    if (paciente.getId() == -1 || paciente.getPrimerConsultaID() == -1) {
        return crearConsultaVacia();
    }

    HistorialMedico actual = buscarPorID(paciente.getPrimerConsultaID());
    HistorialMedico ultima = actual;

    // 🔥 NUEVO CONCEPTO: Recorrer lista enlazada
    // Esto es como seguir una cadena de consultas hasta llegar a la última
    while (actual.getSiguienteConsultaID() != -1) {
        actual = buscarPorID(actual.getSiguienteConsultaID());
        if (actual.getId() == -1) break;  // Error en la lista
        ultima = actual;
    }

    return ultima;
}

int OperacionesHistorial::obtenerHistorialCompleto(int pacienteID, HistorialMedico* resultados, int maxResultados) {
    if (pacienteID <= 0 || !resultados || maxResultados <= 0) {
        return 0;
    }

    // Primero verificar si el paciente existe y tiene consultas
    Paciente paciente = OperacionesPacientes::buscarPorID(pacienteID);
    if (paciente.getId() == -1 || paciente.getCantidadConsultas() == 0) {
        return 0;
    }

    // Abrir archivo de historial médico
    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de historial médico" << endl;
        return 0;
    }

    // Contar cuántas consultas tiene este paciente
    HistorialMedico consulta;
    int totalConsultas = 0;

    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (consulta.getPacienteID() == pacienteID && !consulta.isEliminado()) {
            totalConsultas++;
        }
    }

    if (totalConsultas == 0) {
        archivo.close();
        return 0;
    }

    // Volver a leer y guardar las consultas
    archivo.clear();
    archivo.seekg(0, ios::beg);

    int index = 0;
    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico)) &&
           index < maxResultados && index < totalConsultas) {
        if (consulta.getPacienteID() == pacienteID && !consulta.isEliminado()) {
            resultados[index] = consulta;
            index++;
        }
    }

    archivo.close();
    return totalConsultas;
}

void OperacionesHistorial::mostrarHistorialMedico(int pacienteID) {
    if (pacienteID <= 0) {
        cout << "Error: ID de paciente no válido." << endl;
        return;
    }

    // Buscar paciente
    Paciente paciente = OperacionesPacientes::buscarPorID(pacienteID);
    if (paciente.getId() == -1) {
        cout << "Error: Paciente no encontrado." << endl;
        return;
    }

    if (paciente.isEliminado()) {
        cout << "Error: No se puede mostrar historial de paciente eliminado." << endl;
        return;
    }

    // Obtener historial completo
    const int MAX_CONSULTAS = 100;
    HistorialMedico historial[MAX_CONSULTAS];
    int cantidad = obtenerHistorialCompleto(pacienteID, historial, MAX_CONSULTAS);

    if (cantidad == 0) {
        cout << "El paciente no tiene consultas en su historial médico." << endl;
        return;
    }

    cout << "\n=== HISTORIAL MÉDICO ===" << endl;
    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    cout << "Cédula: " << paciente.getCedula() << " | Edad: " << paciente.getEdad() << endl;
    cout << "Total de consultas: " << cantidad << endl;
    cout << endl;

    cout << left << setw(12) << "FECHA" << setw(8) << "HORA" << setw(25)
         << "DIAGNÓSTICO" << setw(8) << "DOCTOR" << setw(10) << "COSTO" << endl;

    cout << string(65, '-') << endl;

    float costoTotal = 0;
    for (int i = 0; i < cantidad; i++) {
        char diagnosticoMostrar[26];
        strcpy(diagnosticoMostrar, historial[i].getDiagnostico());
        if (strlen(diagnosticoMostrar) > 23) {
            diagnosticoMostrar[20] = '.';
            diagnosticoMostrar[21] = '.';
            diagnosticoMostrar[22] = '.';
            diagnosticoMostrar[23] = '\0';
        }

        cout << left << setw(12) << historial[i].getFecha() << setw(8)
             << historial[i].getHora() << setw(25) << diagnosticoMostrar << setw(8)
             << historial[i].getIdDoctor() << "$" << setw(9) << fixed << setprecision(2)
             << historial[i].getCosto() << endl;

        costoTotal += historial[i].getCosto();
    }

    cout << string(65, '-') << endl;
    cout << "Costo total del historial: $" << fixed << setprecision(2) << costoTotal << endl;
}

void OperacionesHistorial::listarConsultasPorDoctor(int idDoctor) {
    if (idDoctor <= 0) {
        cout << "Error: ID de doctor inválido" << endl;
        return;
    }

    // Verificar que el doctor exista
    Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
    if (doctor.getId() == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return;
    }

    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "No hay consultas registradas." << endl;
        return;
    }

    cout << "\n=== CONSULTAS DEL DOCTOR " << doctor.getNombre() << " " << doctor.getApellido() << " ===" << endl;
    cout << left << setw(8) << "ID" << setw(12) << "FECHA" << setw(8) << "HORA" 
         << setw(8) << "PACIENTE" << setw(20) << "DIAGNÓSTICO" << setw(10) << "COSTO" << endl;
    cout << string(70, '-') << endl;

    HistorialMedico consulta;
    int contador = 0;

    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (!consulta.isEliminado() && consulta.getIdDoctor() == idDoctor) {
            // Obtener nombre del paciente
            Paciente paciente = OperacionesPacientes::buscarPorID(consulta.getPacienteID());
            char nombrePaciente[30];
            if (paciente.getId() != -1) {
                snprintf(nombrePaciente, sizeof(nombrePaciente), "%s %s", 
                         paciente.getNombre(), paciente.getApellido());
                // Truncar si es muy largo
                if (strlen(nombrePaciente) > 28) {
                    nombrePaciente[25] = '.';
                    nombrePaciente[26] = '.';
                    nombrePaciente[27] = '.';
                    nombrePaciente[28] = '\0';
                }
            } else {
                strcpy(nombrePaciente, "No encontrado");
            }

            // Truncar diagnóstico
            char diagMostrar[22];
            strncpy(diagMostrar, consulta.getDiagnostico(), sizeof(diagMostrar) - 1);
            diagMostrar[sizeof(diagMostrar) - 1] = '\0';
            if (strlen(consulta.getDiagnostico()) > 19) {
                diagMostrar[16] = '.';
                diagMostrar[17] = '.';
                diagMostrar[18] = '.';
                diagMostrar[19] = '\0';
            }

            cout << left << setw(8) << consulta.getId() << setw(12) << consulta.getFecha() 
                 << setw(8) << consulta.getHora() << setw(8) << consulta.getPacienteID()
                 << setw(20) << diagMostrar << "$" << setw(9) << fixed << setprecision(2)
                 << consulta.getCosto() << endl;

            contador++;
        }
    }

    archivo.close();
    cout << string(70, '-') << endl;
    cout << "Total de consultas: " << contador << endl;
}

void OperacionesHistorial::listarConsultasPorFecha(const char* fecha) {
    if (!fecha || !HistorialMedico::validarFecha(fecha)) {
        cout << "Error: Fecha inválida" << endl;
        return;
    }

    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "No hay consultas registradas." << endl;
        return;
    }

    cout << "\n=== CONSULTAS DEL " << fecha << " ===" << endl;
    cout << left << setw(8) << "ID" << setw(8) << "HORA" << setw(8) << "PACIENTE" 
         << setw(8) << "DOCTOR" << setw(25) << "DIAGNÓSTICO" << setw(10) << "COSTO" << endl;
    cout << string(70, '-') << endl;

    HistorialMedico consulta;
    int contador = 0;

    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (!consulta.isEliminado() && strcmp(consulta.getFecha(), fecha) == 0) {
            // Truncar diagnóstico
            char diagMostrar[28];
            strncpy(diagMostrar, consulta.getDiagnostico(), sizeof(diagMostrar) - 1);
            diagMostrar[sizeof(diagMostrar) - 1] = '\0';
            if (strlen(consulta.getDiagnostico()) > 25) {
                diagMostrar[22] = '.';
                diagMostrar[23] = '.';
                diagMostrar[24] = '.';
                diagMostrar[25] = '\0';
            }

            cout << left << setw(8) << consulta.getId() << setw(8) << consulta.getHora()
                 << setw(8) << consulta.getPacienteID() << setw(8) << consulta.getIdDoctor()
                 << setw(25) << diagMostrar << "$" << setw(9) << fixed << setprecision(2)
                 << consulta.getCosto() << endl;

            contador++;
        }
    }

    archivo.close();
    cout << string(70, '-') << endl;
    cout << "Total de consultas: " << contador << endl;
}

int OperacionesHistorial::contarConsultas() {
    ArchivoHeader header = GestorArchivos::leerHeader("historiales.bin");
    return header.registrosActivos;
}

int OperacionesHistorial::contarConsultasDePaciente(int pacienteID) {
    if (pacienteID <= 0) return 0;

    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) return 0;

    HistorialMedico consulta;
    int contador = 0;

    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (!consulta.isEliminado() && consulta.getPacienteID() == pacienteID) {
            contador++;
        }
    }

    archivo.close();
    return contador;
}

float OperacionesHistorial::calcularTotalIngresos() {
    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) return 0.0f;

    HistorialMedico consulta;
    float total = 0.0f;

    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (!consulta.isEliminado()) {
            total += consulta.getCosto();
        }
    }

    archivo.close();
    return total;
}

// Funciones privadas helper
int OperacionesHistorial::buscarIndicePorID(int id) {
    if (id <= 0) return -1;

    ifstream archivo("historiales.bin", ios::binary);
    if (!archivo.is_open()) return -1;

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Búsqueda secuencial por índice
    HistorialMedico h;
    int indice = 0;

    while (indice < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&h), sizeof(HistorialMedico))) {
        if (h.getId() == id) {
            archivo.close();
            return indice;
        }
        indice++;
    }

    archivo.close();
    return -1;
}

long OperacionesHistorial::calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(HistorialMedico));
}

HistorialMedico OperacionesHistorial::crearConsultaVacia() {
    HistorialMedico vacia;
    // Inicializar como no encontrada
    vacia.setId(-1);
    vacia.setSiguienteConsultaID(-1);
    return vacia;
}