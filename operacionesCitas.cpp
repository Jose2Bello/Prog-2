#include "../Citas/operacionesCitas.hpp"
#include "../Pacientes/operacionesPacientes.hpp"
#include "../Doctores/operacionesDoctores.hpp"
#include "../Historial/HistorialMedico.hpp"
#include "../persistencia/Constantes.hpp"
#include "../Pacientes/Pacientes.hpp"
#include "../Doctores/Doctor.hpp"
#include "../Citas/Citas.hpp"
#include "../persistencia/GestorArchivos.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>

using namespace std;

bool OperacionesCitas::agendarCita(int idPaciente, int idDoctor, const char* fecha, 
                                     const char* hora, const char* motivo) {
   
    std::cout << "\n=== DEBUG AGENDAR CITA ===" << std::endl;
    std::cout << "RUTA_CITAS = " << RUTA_CITAS << std::endl;
    
    if (!fecha || !hora || !motivo) {
        std::cout << "Error: Parametros invalidos." << std::endl;
        return false;
    }
    
    Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
    if (paciente.getId() == -1) {
        std::cout << "Error: No existe paciente con ID " << idPaciente << std::endl;
        return false;
    }
    
    Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
    if (doctor.getId() == -1) {
        std::cout << "Error: No existe doctor con ID " << idDoctor << std::endl;
        return false;
    }
    
    if (!doctor.isDisponible() || doctor.isEliminado()) {
        std::cout << "Error: El doctor no esta disponible." << std::endl;
        return false;
    }
    
    if (!Cita::validarFecha(fecha)) {
        std::cout << "Error: Formato de fecha invalido. Use YYYY-MM-DD" << std::endl;
        return false;
    }
    
    if (!Cita::validarHora(hora)) {
        std::cout << "Error: Formato de hora invalido. Use HH:MM" << std::endl;
        return false;
    }

    // ========== VERIFICAR SI ARCHIVO EXISTE ==========
    std::cout << "\nVerificando archivo de citas..." << std::endl;
    ifstream test(RUTA_CITAS, ios::binary);
    if (test.is_open()) {
        test.seekg(0, ios::end);
        long tamanio = test.tellg();
        test.close();
        std::cout << "Archivo existe - Tamanio: " << tamanio << " bytes" << std::endl;
    } else {
        std::cout << "Archivo NO existe" << std::endl;
    }

    // Verificar disponibilidad
    ifstream archivoCitas(RUTA_CITAS, ios::binary);
    
    if (archivoCitas.is_open()) {
        // Leer el Header
        ArchivoHeader header;
        archivoCitas.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
        
        std::cout << "Header leido - Registros: " << header.cantidadRegistros 
                  << ", Proximo ID: " << header.proximoID << std::endl;
        
        // Verificar si se pudo leer el header
        if (archivoCitas.gcount() == sizeof(ArchivoHeader)) {
            // Verificar conflictos de horario
            for (int i = 0; i < header.cantidadRegistros; i++) {
                Cita citaExistente;
                
                archivoCitas.read(reinterpret_cast<char*>(&citaExistente), sizeof(Cita));
                
                // Si no se pudo leer el registro completo, salir
                if (archivoCitas.gcount() != sizeof(Cita)) {
                    std::cout << "Error leyendo cita " << i+1 << std::endl;
                    break;
                }
                
                // Saltar registros eliminados o no agendados
                if (citaExistente.isEliminado() || 
                    strcmp(citaExistente.getEstado(), "AGENDADA") != 0) {
                    continue;
                }

                // Verificar conflicto con doctor
                if (citaExistente.getIdDoctor() == idDoctor &&
                    strcmp(citaExistente.getFecha(), fecha) == 0 &&
                    strcmp(citaExistente.getHora(), hora) == 0) {
                    
                    std::cout << "Error: El doctor ya tiene una cita agendada para " << fecha
                              << " a las " << hora << std::endl;
                    archivoCitas.close();
                    return false;
                }

                // Verificar conflicto con paciente
                if (citaExistente.getIdPaciente() == idPaciente &&
                    strcmp(citaExistente.getFecha(), fecha) == 0 &&
                    strcmp(citaExistente.getHora(), hora) == 0) {
                    
                    std::cout << "Error: El paciente ya tiene una cita agendada para " << fecha
                              << " a las " << hora << std::endl;
                    archivoCitas.close();
                    return false;
                }
            }
        }
        
        archivoCitas.close();
    }
    
    // ========== CREAR Y GUARDAR LA CITA ==========
    std::cout << "\nCreando cita..." << std::endl;
    Cita nuevaCita(idPaciente, idDoctor, fecha, hora, motivo);

    // Obtener próximo ID del header
    ArchivoHeader headerCitas = GestorArchivos::leerHeader(RUTA_CITAS);
    std::cout << "Header leido - Tipo: " << headerCitas.tipoArchivo 
              << ", Proximo ID: " << headerCitas.proximoID << std::endl;
    
    if (strcmp(headerCitas.tipoArchivo, "INVALIDO") == 0) {
        // Archivo no existe, inicializarlo
        std::cout << "Inicializando archivo de citas..." << std::endl;
        if (!GestorArchivos::inicializarArchivo(RUTA_CITAS, "CITAS")) {
            std::cout << "Error: No se puede inicializar archivo de citas" << std::endl;
            return false;
        }
        headerCitas = GestorArchivos::leerHeader(RUTA_CITAS);
        std::cout << "Nuevo header - Proximo ID: " << headerCitas.proximoID << std::endl;
    }

    nuevaCita.setId(headerCitas.proximoID);
    std::cout << "ID asignado a cita: " << nuevaCita.getId() << std::endl;

    // Guardar cita en archivo (modo append para agregar al final)
    ofstream archivo(RUTA_CITAS, ios::binary | ios::app);
    if (!archivo.is_open()) {
        std::cout << "Error: No se puede abrir archivo de citas para guardar" << std::endl;
        return false;
    }

    std::cout << "Escribiendo cita en archivo..." << std::endl;
    archivo.write(reinterpret_cast<const char*>(&nuevaCita), sizeof(Cita));
    bool exito = !archivo.fail();
    archivo.close();

    if (!exito) {
        std::cout << "Error: No se pudo guardar la cita" << std::endl;
        return false;
    }

    // Actualizar header
    headerCitas.cantidadRegistros++;
    headerCitas.proximoID++;
    headerCitas.registrosActivos++;
    headerCitas.fechaUltimaModificacion = time(nullptr);
    
    std::cout << "Actualizando header..." << std::endl;
    std::cout << "Nuevos valores - Registros: " << headerCitas.cantidadRegistros
              << ", Proximo ID: " << headerCitas.proximoID << std::endl;
    
    if (!GestorArchivos::actualizarHeader(RUTA_CITAS, headerCitas)) {
        std::cout << "Error: No se pudo actualizar header" << std::endl;
        // Podría seguir siendo exitoso si la cita se guardó
    }

    // ========== VERIFICACIÓN FINAL ==========
    std::cout << "\n=== VERIFICACIÓN FINAL ===" << std::endl;
    
    // Verificar tamaño del archivo
    ifstream verificar(RUTA_CITAS, ios::binary | ios::ate);
    if (verificar.is_open()) {
        long tamanioFinal = verificar.tellg();
        verificar.close();
        
        long tamanioEsperado = sizeof(ArchivoHeader) + (headerCitas.cantidadRegistros * sizeof(Cita));
        std::cout << "Tamanio archivo: " << tamanioFinal << " bytes" << std::endl;
        std::cout << "Tamanio esperado: " << tamanioEsperado << " bytes" << std::endl;
        
        if (tamanioFinal != tamanioEsperado) {
            std::cout << "ADVERTENCIA: Tamaño incorrecto!" << std::endl;
        }
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "CITA AGENDADA EXITOSAMENTE" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "ID Cita: " << nuevaCita.getId() << std::endl;
    std::cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << std::endl;
    std::cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << std::endl;
    std::cout << "Fecha: " << fecha << " " << hora << std::endl;
    std::cout << "Motivo: " << motivo << std::endl;
    std::cout << "========================================" << std::endl;

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
    if (!diagnostico || strlen(diagnostico) == 0) {
        cout << "Error: El diagnóstico no puede estar vacío" << endl;
        return false;
    }

    cout << "\n=== ATENDIENDO CITA ID: " << idCita << " ===" << endl;

    // Buscar la cita por ID
    Cita cita = buscarPorID(idCita);
    if (cita.getId() == -1) {
        cout << "Error: No se encontró cita con ID " << idCita << endl;
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

    // Actualizar contador de consultas del paciente
    paciente.setCantidadConsultas(paciente.getCantidadConsultas() + 1);
    
    // Actualizar paciente en el archivo
    if (!OperacionesPacientes::actualizarPaciente(paciente)) {
        cout << "Advertencia: No se pudo actualizar contador de consultas del paciente" << endl;
    }

    // ========== MOSTRAR RESULTADO ==========
    
    cout << "\n========================================" << endl;
    cout << "CITA ATENDIDA EXITOSAMENTE" << endl;
    cout << "========================================" << endl;
    cout << "ID Cita: " << cita.getId() << endl;
    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    cout << "Cédula: " << paciente.getCedula() << endl;
    cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
    cout << "Especialidad: " << doctor.getEspecialidad() << endl;
    cout << "Fecha: " << cita.getFecha() << " " << cita.getHora() << endl;
    cout << "Costo: $" << doctor.getCostoConsulta() << endl;
    cout << "Consultas totales del paciente: " << paciente.getCantidadConsultas() << endl;
    cout << "\n--- RESUMEN DE ATENCIÓN ---" << endl;
    cout << "Diagnóstico: " << diagnostico << endl;
    if (tratamiento && strlen(tratamiento) > 0) {
        cout << "Tratamiento: " << tratamiento << endl;
    }
    if (medicamentos && strlen(medicamentos) > 0) {
        cout << "Medicamentos: " << medicamentos << endl;
    }
    cout << "========================================" << endl;

    return true;
}

Cita OperacionesCitas::buscarPorID(int id) {
    Cita cita;
    
    if (id <= 0) {
        cout << "Error: ID de cita inválido" << endl;
        return cita;
    }

    // Abrir citas.bin usando RUTA_CITAS
    ifstream archivo(RUTA_CITAS, ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de citas: " << RUTA_CITAS << endl;
        return cita;
    }

    // Leer header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    cout << "\n=== DEBUG buscarPorID ===" << endl;
    cout << "Buscando ID: " << id << endl;
    cout << "Tipo archivo: " << header.tipoArchivo << endl;
    cout << "Cantidad registros: " << header.cantidadRegistros << endl;

    if (archivo.fail() || strcmp(header.tipoArchivo, "CITAS") != 0) {
        cout << "Error: Archivo de citas corrupto o vacío" << endl;
        archivo.close();
        return cita;
    }

    // Verificar si hay registros
    if (header.cantidadRegistros == 0) {
        cout << "No hay citas registradas" << endl;
        archivo.close();
        return cita;
    }

    // Búsqueda secuencial
    bool encontrado = false;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita));
        
        if (archivo.gcount() != sizeof(Cita)) {
            cout << "Error leyendo cita " << i+1 << endl;
            break;
        }
        
        cout << "Cita #" << (i+1) 
             << " - ID: " << cita.getId() 
             << ", Estado: " << cita.getEstado()
             << ", Eliminada: " << (cita.isEliminado() ? "Sí" : "No") 
             << endl;
        
        if (cita.getId() == id && !cita.isEliminado()) {
            encontrado = true;
            cout << "¡Cita encontrada!" << endl;
            break;
        }
    }

    archivo.close();

    if (!encontrado) {
        Cita vacia;
        cout << "Cita con ID " << id << " no encontrada o eliminada" << endl;
        return vacia;
    }

    cout << "=== FIN DEBUG ===" << endl;
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
    cout << "\n=== LISTANDO CITAS ===" << endl;
    
    // Crear directorio si no existe
    system("mkdir datos 2>nul");
    
    // Leer header usando RUTA_CITAS
    ArchivoHeader header = GestorArchivos::leerHeader(RUTA_CITAS);
    
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "Archivo de citas no existe" << endl;
        return;
    }
    
    if (header.registrosActivos == 0) {
        cout << "No hay citas registradas." << endl;
        return;
    }

    cout << "\nLISTA DE CITAS (" << header.registrosActivos << " registros):" << endl;
    cout << "================================================================" << endl;

    // Abrir archivo y leer citas
    ifstream archivo(RUTA_CITAS, ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de citas" << endl;
        return;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Cita c;
    int citasMostradas = 0;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&c), sizeof(Cita));
        
        if (archivo.gcount() != sizeof(Cita)) {
            cout << "Error leyendo cita " << i+1 << endl;
            break;
        }
        
        // Solo mostrar citas no eliminadas
        if (!c.isEliminado()) {
            if (mostrarCanceladas || strcmp(c.getEstado(), "CANCELADA") != 0) {
                cout << "\n[" << ++citasMostradas << "] ID: " << c.getId() << endl;
                cout << "   Fecha: " << c.getFecha() << " " << c.getHora() << endl;
                
                // Obtener información del paciente
                Paciente paciente = OperacionesPacientes::buscarPorID(c.getIdPaciente());
                if (paciente.getId() != -1) {
                    cout << "   Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                } else {
                    cout << "   Paciente: ID " << c.getIdPaciente() << " (no encontrado)" << endl;
                }
                
                // Obtener información del doctor
                Doctor doctor = OperacionesDoctores::buscarPorID(c.getIdDoctor());
                if (doctor.getId() != -1) {
                    cout << "   Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                } else {
                    cout << "   Doctor: ID " << c.getIdDoctor() << " (no encontrado)" << endl;
                }
                
                cout << "   Estado: " << c.getEstado() << endl;
                cout << "   Motivo: " << c.getMotivo() << endl;
            }
        }
    }

    archivo.close();

    if (citasMostradas == 0) {
        cout << "No hay citas para mostrar." << endl;
    } else {
        cout << "\n================================================================" << endl;
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
int OperacionesCitas::buscarIndicePorID(int idCita) {
    if (idCita <= 0) {
        return -1;
    }

    ifstream archivo(RUTA_CITAS, ios::binary);  // Misma ruta que buscarPorID
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de citas" << endl;
        return -1;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail() || strcmp(header.tipoArchivo, "CITAS") != 0) {
        archivo.close();
        return -1;
    }

    Cita cita;
    int indice = -1;

    // Búsqueda secuencial (misma lógica que buscarPorID)
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita));
        
        if (archivo.gcount() != sizeof(Cita)) {
            break;
        }
        
        // Misma condición EXACTA que en buscarPorID
        if (cita.getId() == idCita && !cita.isEliminado()) {
            indice = i;
            break;
        }
    }

    archivo.close();
    return indice;
}

long OperacionesCitas::calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Cita));
}
