#include "../Historial/HistorialMedico.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

HistorialMedico::HistorialMedico() {
    inicializar();
}

HistorialMedico::HistorialMedico(int pacienteID, int idDoctor, const char* fecha, const char* hora) {
    inicializar();
    
    setPacienteID(pacienteID);
    setIdDoctor(idDoctor);
    setFecha(fecha);
    setHora(hora);
}

void HistorialMedico::inicializar() {
    id = -1;
    pacienteID = -1;
    idDoctor = -1;
    eliminado = false;
    fechaRegistro = time(nullptr);
    siguienteConsultaID = -1;  // Por defecto, no tiene siguiente
    costo = 0.0f;

    // Inicializar strings
    memset(fecha, 0, sizeof(fecha));
    memset(hora, 0, sizeof(hora));
    memset(diagnostico, 0, sizeof(diagnostico));
    memset(tratamiento, 0, sizeof(tratamiento));
    memset(medicamentos, 0, sizeof(medicamentos));
}

// Getters
int HistorialMedico::getId() const { return id; }
int HistorialMedico::getPacienteID() const { return pacienteID; }
int HistorialMedico::getIdDoctor() const { return idDoctor; }
bool HistorialMedico::isEliminado() const { return eliminado; }
time_t HistorialMedico::getFechaRegistro() const { return fechaRegistro; }
int HistorialMedico::getSiguienteConsultaID() const { return siguienteConsultaID; }
const char* HistorialMedico::getFecha() const { return fecha; }
const char* HistorialMedico::getHora() const { return hora; }
const char* HistorialMedico::getDiagnostico() const { return diagnostico; }
const char* HistorialMedico::getTratamiento() const { return tratamiento; }
const char* HistorialMedico::getMedicamentos() const { return medicamentos; }
float HistorialMedico::getCosto() const { return costo; }

// Setters
void HistorialMedico::setId(int id) { 
    this->id = id; 
    actualizarTimestamp();
}

void HistorialMedico::setPacienteID(int pacienteID) { 
    this->pacienteID = pacienteID; 
    actualizarTimestamp();
}

void HistorialMedico::setIdDoctor(int idDoctor) { 
    this->idDoctor = idDoctor; 
    actualizarTimestamp();
}

void HistorialMedico::setEliminado(bool eliminado) { 
    this->eliminado = eliminado; 
    actualizarTimestamp();
}

void HistorialMedico::setFechaRegistro(time_t fechaRegistro) { 
    this->fechaRegistro = fechaRegistro; 
}

void HistorialMedico::setSiguienteConsultaID(int siguienteConsultaID) { 
    this->siguienteConsultaID = siguienteConsultaID; 
    actualizarTimestamp();
}

void HistorialMedico::setFecha(const char* fecha) {
    if (fecha && validarFecha(fecha)) {
        strncpy(this->fecha, fecha, sizeof(this->fecha) - 1);
        this->fecha[sizeof(this->fecha) - 1] = '\0';
        actualizarTimestamp();
    }
}

void HistorialMedico::setHora(const char* hora) {
    if (hora && validarHora(hora)) {
        strncpy(this->hora, hora, sizeof(this->hora) - 1);
        this->hora[sizeof(this->hora) - 1] = '\0';
        actualizarTimestamp();
    }
}

void HistorialMedico::setDiagnostico(const char* diagnostico) {
    if (diagnostico) {
        strncpy(this->diagnostico, diagnostico, sizeof(this->diagnostico) - 1);
        this->diagnostico[sizeof(this->diagnostico) - 1] = '\0';
        actualizarTimestamp();
    }
}

void HistorialMedico::setTratamiento(const char* tratamiento) {
    if (tratamiento) {
        strncpy(this->tratamiento, tratamiento, sizeof(this->tratamiento) - 1);
        this->tratamiento[sizeof(this->tratamiento) - 1] = '\0';
        actualizarTimestamp();
    }
}

void HistorialMedico::setMedicamentos(const char* medicamentos) {
    if (medicamentos) {
        strncpy(this->medicamentos, medicamentos, sizeof(this->medicamentos) - 1);
        this->medicamentos[sizeof(this->medicamentos) - 1] = '\0';
        actualizarTimestamp();
    }
}

void HistorialMedico::setCosto(float costo) {
    if (validarCosto(costo)) {
        this->costo = costo;
        actualizarTimestamp();
    }
}

// Métodos de utilidad
void HistorialMedico::actualizarTimestamp() {
    fechaRegistro = time(nullptr);
}

void HistorialMedico::mostrarInfo() const {
    cout << "\n=== INFORMACION DE CONSULTA MEDICA ===" << endl;
    cout << "ID Consulta: " << id << endl;
    cout << "Paciente ID: " << pacienteID << endl;
    cout << "Doctor ID: " << idDoctor << endl;
    cout << "Fecha consulta: " << fecha << " " << hora << endl;
    cout << "Diagnóstico: " << diagnostico << endl;
    cout << "Tratamiento: " << tratamiento << endl;
    cout << "Medicamentos: " << medicamentos << endl;
    cout << "Costo: $" << fixed << setprecision(2) << costo << endl;
    
    char fechaRegistroStr[20];
    strftime(fechaRegistroStr, sizeof(fechaRegistroStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaRegistro));
    cout << "Registrado en sistema: " << fechaRegistroStr << endl;
    
    // CORRECCIÓN: Sin to_string()
    cout << "Siguiente consulta ID: ";
    if (siguienteConsultaID == -1) {
        cout << "NINGUNA";
    } else {
        cout << siguienteConsultaID;  // Directamente el número
    }
    cout << endl;
    
    cout << "Estado: " << (eliminado ? "ELIMINADO" : "ACTIVO") << endl;
}

void HistorialMedico::mostrarResumen() const {
    cout << "    RESUMEN DE CONSULTA:" << endl;
    cout << "   +-- ID: " << id << endl;
    cout << "   +-- Fecha: " << fecha << " " << hora << endl;
    
    // Mostrar diagnóstico truncado si es muy largo
    char diagMostrar[50];
    strncpy(diagMostrar, diagnostico, sizeof(diagMostrar) - 1);
    diagMostrar[sizeof(diagMostrar) - 1] = '\0';
    if (strlen(diagnostico) > 47) {
        diagMostrar[44] = '.';
        diagMostrar[45] = '.';
        diagMostrar[46] = '.';
        diagMostrar[47] = '\0';
    }
    cout << "   +-- Diagnóstico: " << diagMostrar << endl;
    cout << "   +-- Costo: $" << fixed << setprecision(2) << costo << endl;
}

// Validaciones
bool HistorialMedico::esValido() const {
    return (id > 0 && 
            pacienteID > 0 && 
            idDoctor > 0 && 
            validarFecha(fecha) && 
            validarHora(hora) && 
            strlen(diagnostico) > 0 &&
            validarCosto(costo));
}

bool HistorialMedico::validarFecha(const char* fecha) {
    if (!fecha || strlen(fecha) != 10) return false;
    if (fecha[4] != '-' || fecha[7] != '-') return false;
    
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(fecha[i])) return false;
    }
    
    return true;
}

bool HistorialMedico::validarHora(const char* hora) {
    if (!hora || strlen(hora) != 5) return false;
    if (hora[2] != ':') return false;
    
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) return false;
    }
    
    int horas = atoi(hora);
    int minutos = atoi(hora + 3);
    return (horas >= 0 && horas <= 23 && minutos >= 0 && minutos <= 59);
}

bool HistorialMedico::validarCosto(float costo) {
    return costo >= 0;
}
