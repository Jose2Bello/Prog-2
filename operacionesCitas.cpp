#include "operacionesCitas.hpp"
#include "../persistencia/GestorArchivos.hpp"
#include "../pacientes/operacionesPacientes.hpp"
#include "../doctores/operacionesDoctores.hpp"
#include "operacionesHistorial.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>

using namespace std;

bool OperacionesCitas::agendarCita(int idPaciente, int idDoctor, const char* fecha, 
                                  const char* hora, const char* motivo) {
    // Validaciones básicas de parámetros
    if (!fecha || !hora || !motivo) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }

    // Verificar que el paciente exista
    Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
    if (paciente.getId() == -1) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        return false;
    }

    // Verificar que el doctor exista
    Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
    if (doctor.getId() == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return false;
    }

    // Verificar que el doctor esté disponible
    if (!doctor.isDisponible() || doctor.isEliminado()) {
        cout << "Error: El doctor no está disponible." << endl;
        return false;
    }

    // Validar formato de fecha
    if (!Cita::validarFecha(fecha)) {
        cout << "Error: Formato de fecha inválido. Use YYYY-MM-DD" << endl;
        return false;
    }

    // Validar formato de hora
    if (!Cita::validarHora(hora)) {
        cout << "Error: Formato de hora inválido. Use HH:MM" << endl;
        return false;
    }

    // Verificar disponibilidad del doctor
    ifstream archivoCitas("citas.bin", ios::binary);
    if (archivoCitas.is_open()) {
        ArchivoHeader header;
        archivoCitas.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

        for (int i = 0; i < header.cantidadRegistros; i++) {
            Cita citaExistente;
            if (archivoCitas.read(reinterpret_cast<char*>(&citaExistente), sizeof(Cita))) {
                if (!citaExistente.isEliminado() && 
                    citaExistente.getIdDoctor() == idDoctor &&
                    strcmp(citaExistente.getFecha(), fecha) == 0 &&
                    strcmp(citaExistente.getHora(), hora) == 0 &&
                    strcmp(citaExistente.getEstado(), "AGENDADA") == 0) {
                    cout << "Error: El doctor ya tiene una cita agendada para " << fecha
                         << " a las " << hora << endl;
                    archivoCitas.close();
                    return false;
                }
            }
        }
        archivoCitas.close();
    }

    // Verificar que el paciente no tenga otra cita a la misma hora
    ifstream archivoCitas2("citas.bin", ios::binary);
    if (archivoCitas2.is_open()) {
        ArchivoHeader header;
        archivoCitas2.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

        for (int i = 0; i < header.cantidadRegistros; i++) {
            Cita citaExistente;
            if (archivoCitas2.read(reinterpret_cast<char*>(&citaExistente), sizeof(Cita))) {
                if (!citaExistente.isEliminado() &&
                    citaExistente.getIdPaciente() == idPaciente &&
                    strcmp(citaExistente.getFecha(), fecha) == 0 &&
                    strcmp(citaExistente.getHora(), hora) == 0 &&
                    strcmp(citaExistente.getEstado(), "AGENDADA") == 0) {
                    cout << "Error: El paciente ya tiene una cita agendada para " << fecha
                         << " a las " << hora << endl;
                    archivoCitas2.close();
                    return false;
                }
            }
        }
        archivoCitas2.close();
    }

    // Crear nueva cita
    Cita nuevaCita(idPaciente, idDoctor, fecha, hora, motivo);

    // Obtener próximo ID del header
    ArchivoHeader headerCitas = GestorArchivos::leerHeader("citas.bin");
    if (strcmp(headerCitas.tipoArchivo, "INVALIDO") == 0) {
        // Archivo no existe, inicializarlo
        if (!GestorArchivos::inicializarArchivo("citas.bin", "CITAS")) {
            cout << "Error: No se puede inicializar archivo de citas" << endl;
            return false;
        }
        headerCitas = GestorArchivos::leerHeader("citas.bin");
    }

    nuevaCita.setId(headerCitas.proximoID);

    // Guardar cita en archivo
    ofstream archivo("citas.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin para guardar" << endl;
        return false;
    }

    archivo.write(reinterpret_cast<const char*>(&nuevaCita), sizeof(Cita));
    bool exito = !archivo.fail();
    archivo.close();

    if (!exito) {
        cout << "Error: No se pudo guardar la cita" << endl;
        return false;
    }

    // Actualizar header
    headerCitas.cantidadRegistros++;
    headerCitas.proximoID++;
    headerCitas.registrosActivos++;
    headerCitas.fechaUltimaModificacion = time(nullptr);
    GestorArchivos::actualizarHeader("citas.bin", headerCitas);

    cout << "CITA AGENDADA EXITOSAMENTE" << endl;
    cout << "ID Cita: " << nuevaCita.getId() << endl;
    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
    cout << "Fecha: " << fecha << " " << hora << endl;
    cout << "Motivo: " << motivo << endl;

    return true;
}

bool OperacionesCitas::actualizarCita(const Cita& cita) {
    if (cita.getId() <= 0) {
        cout << "Error: ID de cita inválido" << endl;
        return false;
    }

    // Buscar posición de la cita
    int indice = buscarIndicePorID(cita.getId());
    if (indice == -1) {
        cout << "Error: No se puede encontrar cita con ID " << cita.getId() << endl;
        return false;
    }

    // Abrir archivo en modo lectura/escritura
    fstream archivo("citas.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin para actualización" << endl;
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
    archivo.write(reinterpret_cast<const char*>(&cita), sizeof(Cita));
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (exito) {
        cout << "Cita ID " << cita.getId() << " actualizada exitosamente" << endl;
    } else {
        cout << "Error al actualizar cita ID " << cita.getId() << endl;
    }

    return exito;
}

bool OperacionesCitas::cancelarCita(int idCita) {
    // Validar ID
    if (idCita <= 0) {
        cout << "Error: ID de cita inválido" << endl;
        return false;
    }

    // Buscar la cita por ID
    int indice = buscarIndicePorID(idCita);
    if (indice == -1) {
        cout << "Error: No se encontró cita con ID " << idCita << endl;
        return false;
    }

    // Leer la cita completa
    Cita cita = buscarPorID(idCita);
    if (cita.getId() == -1) {
        cout << "Error: No se puede acceder a la cita ID " << idCita << endl;
        return false;
    }

    // Verificar que la cita no esté ya cancelada
    if (strcmp(cita.getEstado(), "CANCELADA") == 0) {
        cout << "La cita ya está cancelada." << endl;
        return false;
    }

    // Verificar que no esté atendida
    if (cita.isAtendida()) {
        cout << "Error: No se puede cancelar una cita ya atendida." << endl;
        return false;
    }

    // Obtener información del paciente y doctor para el mensaje
    Paciente paciente = OperacionesPacientes::buscarPorID(cita.getIdPaciente());
    Doctor doctor = OperacionesDoctores::buscarPorID(cita.getIdDoctor());

    // Actualizar estado de la cita
    cita.marcarComoCancelada("Cita cancelada por el usuario");

    // Guardar los cambios en el archivo
    bool exito = actualizarCita(cita);

    if (exito) {
        cout << "CITA CANCELADA EXITOSAMENTE" << endl;
        cout << "ID Cita: " << cita.getId() << endl;

        if (paciente.getId() != -1) {
            cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
        } else {
            cout << "Paciente: No encontrado" << endl;
        }

        if (doctor.getId() != -1) {
            cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
        } else {
            cout << "Doctor: No encontrado" << endl;
        }

        cout << "Fecha: " << cita.getFecha() << " " << cita.getHora() << endl;
        cout << "Motivo original: " << cita.getMotivo() << endl;
    } else {
        cout << "Error: No se pudo cancelar la cita" << endl;
    }

    return exito;
}

bool OperacionesCitas::atenderCita(int idCita, const char* diagnostico, const char* tratamiento, 
                                  const char* medicamentos) {
    // Validar parámetros
    if (!diagnostico || !tratamiento || !medicamentos) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }

    if (strlen(diagnostico) == 0) {
        cout << "Error: El diagnóstico no puede estar vacío" << endl;
        return false;
    }

    // Buscar la cita por ID
    int indiceCita = buscarIndicePorID(idCita);
    if (indiceCita == -1) {
        cout << "Error: No se encontró cita con ID " << idCita << endl;
        return false;
    }

    // Leer la cita completa
    Cita cita = buscarPorID(idCita);
    if (cita.getId() == -1) {
        cout << "Error: No se puede acceder a la cita ID " << idCita << endl;
        return false;
    }

    // Verificar que esté en estado "Agendada"
    if (strcmp(cita.getEstado(), "AGENDADA") != 0) {
        cout << "Error: La cita no está en estado 'Agendada'. Estado actual: "
             << cita.getEstado() << endl;
        return false;
    }

    if (cita.isAtendida()) {
        cout << "Error: La cita ya fue atendida." << endl;
        return false;
    }

    // Obtener información del paciente y doctor
    Paciente paciente = OperacionesPacientes::buscarPorID(cita.getIdPaciente());
    Doctor doctor = OperacionesDoctores::buscarPorID(cita.getIdDoctor());

    if (paciente.getId() == -1) {
        cout << "Error: No se encontró el paciente asociado a la cita." << endl;
        return false;
    }

    if (doctor.getId() == -1) {
        cout << "Error: No se encontró el doctor asociado a la cita." << endl;
        return false;
    }

    // Actualizar estado de la cita
    char observaciones[200];
    snprintf(observaciones, sizeof(observaciones), "Atendida - Diagnóstico: %s", diagnostico);
    
    cita.marcarComoAtendida(observaciones);

    // Guardar cambios en la cita
    if (!actualizarCita(cita)) {
        cout << "Error: No se pudo actualizar la cita" << endl;
        return false;
    }

    // Crear entrada en el historial médico (si existe el módulo)
    // Nota: Aquí asumimos que existe OperacionesHistorial
    HistorialMedico nuevaConsulta;
    // ... código para crear la consulta en historial ...

    cout << "CITA ATENDIDA EXITOSAMENTE" << endl;
    cout << "ID Cita: " << cita.getId() << endl;
    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
    cout << "Fecha: " << cita.getFecha() << " " << cita.getHora() << endl;
    cout << "Costo: $" << doctor.getCostoConsulta() << endl;

    return true;
}

Cita OperacionesCitas::buscarPorID(int id) {
    Cita cita;
    
    if (id <= 0) {
        cout << "Error: ID de cita inválido" << endl;
        return cita;
    }

    // Abrir citas.bin
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin" << endl;
        return cita;
    }

    // Leer header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se puede leer header de citas.bin" << endl;
        archivo.close();
        return cita;
    }

    // Verificar si hay registros
    if (header.cantidadRegistros == 0) {
        cout << "No hay citas registradas" << endl;
        archivo.close();
        return cita;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);

    // Búsqueda secuencial
    bool encontrado = false;
    int citasLeidas = 0;

    while (citasLeidas < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
        if (cita.getId() == id && !cita.isEliminado()) {
            encontrado = true;
            break;
        }
        citasLeidas++;
    }

    archivo.close();

    if (!encontrado) {
        Cita vacia;
        cout << "Cita con ID " << id << " no encontrada" << endl;
        return vacia;
    }

    return cita;
}

int OperacionesCitas::buscarCitasDePaciente(int idPaciente, Cita* resultados, int maxResultados) {
    if (idPaciente <= 0 || !resultados || maxResultados <= 0) {
        cout << "Error: Parámetros de búsqueda inválidos" << endl;
        return 0;
    }

    // Verificar que el paciente exista
    Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
    if (paciente.getId() == -1) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        return 0;
    }

    // Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "El paciente no tiene citas registradas." << endl;
        return 0;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        archivo.close();
        return 0;
    }

    // Buscar citas del paciente
    int encontradas = 0;
    Cita cita;

    while (encontradas < maxResultados &&
           archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
        if (!cita.isEliminado() && cita.getIdPaciente() == idPaciente) {
            resultados[encontradas] = cita;
            encontradas++;
        }
    }

    archivo.close();

    if (encontradas > 0) {
        cout << "Encontradas " << encontradas << " citas para el paciente ID " << idPaciente << endl;
    } else {
        cout << "No se encontraron citas para este paciente." << endl;
    }

    return encontradas;
}

int OperacionesCitas::buscarCitasDeDoctor(int idDoctor, Cita* resultados, int maxResultados) {
    if (idDoctor <= 0 || !resultados || maxResultados <= 0) {
        cout << "Error: Parámetros de búsqueda inválidos" << endl;
        return 0;
    }

    // Verificar que el doctor exista
    Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
    if (doctor.getId() == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return 0;
    }

    // Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "El doctor no tiene citas registradas." << endl;
        return 0;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        archivo.close();
        return 0;
    }

    // Buscar citas del doctor
    int encontradas = 0;
    Cita cita;

    while (encontradas < maxResultados &&
           archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
        if (!cita.isEliminado() && cita.getIdDoctor() == idDoctor) {
            resultados[encontradas] = cita;
            encontradas++;
        }
    }

    archivo.close();

    if (encontradas > 0) {
        cout << "Encontradas " << encontradas << " citas para el doctor ID " << idDoctor << endl;
    } else {
        cout << "No se encontraron citas para este doctor." << endl;
    }

    return encontradas;
}

int OperacionesCitas::buscarCitasPorFecha(const char* fecha, Cita* resultados, int maxResultados) {
    if (!fecha || !resultados || maxResultados <= 0) {
        cout << "Error: Parámetros de búsqueda inválidos" << endl;
        return 0;
    }

    // Validar formato de fecha
    if (!Cita::validarFecha(fecha)) {
        cout << "Error: Formato de fecha inválido. Use YYYY-MM-DD" << endl;
        return 0;
    }

    // Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin" << endl;
        return 0;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        cout << "Error: No se puede leer header de citas.bin" << endl;
        archivo.close();
        return 0;
    }

    // Buscar citas por fecha
    int encontradas = 0;
    Cita cita;

    while (encontradas < maxResultados &&
           archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
        if (!cita.isEliminado() && strcmp(cita.getFecha(), fecha) == 0) {
            resultados[encontradas] = cita;
            encontradas++;
        }
    }

    archivo.close();

    if (encontradas > 0) {
        cout << "Encontradas " << encontradas << " citas para la fecha " << fecha << endl;
    } else {
        cout << "No hay citas registradas para la fecha " << fecha << endl;
    }

    return encontradas;
}

void OperacionesCitas::listarCitasPendientes() {
    // Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "No hay citas en el sistema." << endl;
        return;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail() || header.cantidadRegistros == 0) {
        cout << "No hay citas en el sistema." << endl;
        archivo.close();
        return;
    }

    // Contar citas pendientes
    int cantidadPendientes = 0;
    Cita* citasPendientes = new Cita[header.cantidadRegistros];

    // Leer todas las citas y filtrar pendientes
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.isEliminado() && strcmp(cita.getEstado(), "AGENDADA") == 0) {
                citasPendientes[cantidadPendientes] = cita;
                cantidadPendientes++;
            }
        }
    }
    archivo.close();

    // Mostrar resultados
    if (cantidadPendientes == 0) {
        cout << "No hay citas pendientes." << endl;
        delete[] citasPendientes;
        return;
    }

    cout << "\n================================================================" << endl;
    cout << "               CITAS PENDIENTES - TOTAL: " << cantidadPendientes << endl;
    cout << "================================================================" << endl;

    for (int i = 0; i < cantidadPendientes; i++) {
        Cita c = citasPendientes[i];
        Paciente paciente = OperacionesPacientes::buscarPorID(c.getIdPaciente());
        Doctor doctor = OperacionesDoctores::buscarPorID(c.getIdDoctor());

        cout << "CITA #" << c.getId() << " -----------------------------------------" << endl;
        cout << "  Fecha: " << c.getFecha() << " " << c.getHora() << endl;
        cout << "  Paciente: "
             << (paciente.getId() != -1
                     ? string(paciente.getNombre()) + " " + paciente.getApellido()
                     : "No encontrado")
             << endl;
        cout << "  Doctor: "
             << (doctor.getId() != -1
                     ? "Dr. " + string(doctor.getNombre()) + " " + doctor.getApellido()
                     : "No encontrado")
             << endl;
        cout << "  Especialidad: "
             << (doctor.getId() != -1 ? doctor.getEspecialidad() : "N/A") << endl;
        cout << "  Motivo: " << c.getMotivo() << endl;
        cout << "  Costo: $" << (doctor.getId() != -1 ? doctor.getCostoConsulta() : 0)
             << endl;
        cout << endl;
    }

    cout << "================================================================" << endl;
    delete[] citasPendientes;
}

void OperacionesCitas::listarTodasCitas(bool mostrarCanceladas) {
    // Leer header para saber cuántas citas hay
    ArchivoHeader header = GestorArchivos::leerHeader("citas.bin");

    if (strcmp(header.tipoArchivo, "INVALIDO") == 0 ||
        header.registrosActivos == 0) {
        cout << "No hay citas registradas." << endl;
        return;
    }

    cout << "\nLISTA DE TODAS LAS CITAS (" << header.registrosActivos << "):" << endl;
    cout << "+--------------------------------------------------------------------------------+" << endl;
    cout << "|  ID  |   FECHA    | HORA  | PACIENTE | DOCTOR |    ESTADO    |     MOTIVO      |" << endl;
    cout << "+------+------------+-------+----------+--------+--------------+-----------------+" << endl;

    // Abrir archivo y leer citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de citas" << endl;
        return;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Cita c;
    int citasMostradas = 0;

    while (archivo.read(reinterpret_cast<char*>(&c), sizeof(Cita)) &&
           citasMostradas < header.registrosActivos) {
        // Solo mostrar citas no eliminadas (o todas si se solicita)
        if (mostrarCanceladas || strcmp(c.getEstado(), "CANCELADA") != 0) {
            char motivoMostrar[18];
            strncpy(motivoMostrar, c.getMotivo(), sizeof(motivoMostrar) - 1);
            motivoMostrar[sizeof(motivoMostrar) - 1] = '\0';
            if (strlen(c.getMotivo()) > 16) {
                motivoMostrar[13] = '.';
                motivoMostrar[14] = '.';
                motivoMostrar[15] = '.';
                motivoMostrar[16] = '\0';
            }

            printf("| %4d | %10s | %5s | %8d | %6d | %-12s | %-15s |\n", 
                   c.getId(), c.getFecha(), c.getHora(), c.getIdPaciente(), 
                   c.getIdDoctor(), c.getEstado(), motivoMostrar);

            citasMostradas++;
        }
    }

    archivo.close();

    cout << "+--------------------------------------------------------------------------------+" << endl;

    // Mostrar estadísticas adicionales
    if (citasMostradas > 0) {
        cout << "Total mostradas: " << citasMostradas << " cita(s)" << endl;
    }
}

int OperacionesCitas::contarCitas() {
    ArchivoHeader header = GestorArchivos::leerHeader("citas.bin");
    return header.registrosActivos;
}

int OperacionesCitas::contarCitasPendientes() {
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int pendientes = 0;
    Cita cita;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.isEliminado() && strcmp(cita.getEstado(), "AGENDADA") == 0) {
                pendientes++;
            }
        }
    }

    archivo.close();
    return pendientes;
}

bool OperacionesCitas::verificarDisponibilidad(int idDoctor, const char* fecha, const char* hora) {
    // Validar parámetros
    if (!fecha || !hora) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }

    // Verificar que el doctor exista
    Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
    if (doctor.getId() == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return false;
    }

    // Verificar que el doctor esté disponible
    if (!doctor.isDisponible() || doctor.isEliminado()) {
        cout << "El doctor no está disponible para consultas." << endl;
        return false;
    }

    // Validar formato de fecha
    if (!Cita::validarFecha(fecha)) {
        cout << "Error: Formato de fecha inválido." << endl;
        return false;
    }

    // Validar formato de hora
    if (!Cita::validarHora(hora)) {
        cout << "Error: Formato de hora inválido." << endl;
        return false;
    }

    // Verificar si el doctor ya tiene una cita a esa fecha/hora
    ifstream archivo("citas.bin", ios::binary);
    if (archivo.is_open()) {
        ArchivoHeader header;
        archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

        for (int i = 0; i < header.cantidadRegistros; i++) {
            Cita cita;
            if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
                if (!cita.isEliminado() && cita.getIdDoctor() == idDoctor &&
                    strcmp(cita.getFecha(), fecha) == 0 && strcmp(cita.getHora(), hora) == 0 &&
                    strcmp(cita.getEstado(), "AGENDADA") == 0) {
                    // Encontrar información del paciente para el mensaje
                    Paciente paciente = OperacionesPacientes::buscarPorID(cita.getIdPaciente());
                    cout << "El doctor ya tiene una cita agendada para " << fecha
                         << " a las " << hora << endl;
                    if (paciente.getId() != -1) {
                        cout << "Cita con: " << paciente.getNombre() << " " << paciente.getApellido()
                             << endl;
                    }
                    archivo.close();
                    return false;
                }
            }
        }
        archivo.close();
    }

    // Si llegamos aquí, el doctor está disponible
    cout << "   DOCTOR DISPONIBLE" << endl;
    cout << "   Fecha: " << fecha << " a las " << hora << endl;
    cout << "   Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
    cout << "   Especialidad: " << doctor.getEspecialidad() << endl;
    cout << "   Costo consulta: $" << doctor.getCostoConsulta() << endl;

    return true;
}

// Funciones privadas helper
int OperacionesCitas::buscarIndicePorID(int id) {
    if (id <= 0) return -1;

    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) return -1;

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Búsqueda secuencial por índice
    Cita c;
    int indice = 0;

    while (indice < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&c), sizeof(Cita))) {
        if (c.getId() == id) {
            archivo.close();
            return indice;
        }
        indice++;
    }

    archivo.close();
    return -1;
}

long OperacionesCitas::calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Cita));
}