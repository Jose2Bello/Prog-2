#include "Doctor.hpp"
#include <iostream>
#include <cstring>
#include <ctime>
#include <iomanip>

using namespace std;

Doctor::Doctor() {
    inicializar();
}

Doctor::Doctor(const char* nombre, const char* apellido, const char* cedula, 
               const char* especialidad, int aniosExperiencia, float costoConsulta) {
    inicializar();
    
    setNombre(nombre);
    setApellido(apellido);
    setCedula(cedula);
    setEspecialidad(especialidad);
    setAniosExperiencia(aniosExperiencia);
    setCostoConsulta(costoConsulta);
}

void Doctor::inicializar() {
    id = -1;
    aniosExperiencia = 0;
    costoConsulta = 0.0f;
    cantidadPacientes = 0;
    cantidadCitas = 0;
    disponible = true;
    eliminado = false;

    // Inicializar strings
    memset(nombre, 0, sizeof(nombre));
    memset(apellido, 0, sizeof(apellido));
    memset(cedula, 0, sizeof(cedula));
    memset(especialidad, 0, sizeof(especialidad));
    memset(horarioAtencion, 0, sizeof(horarioAtencion));
    strcpy(horarioAtencion, "L-V 8:00-17:00"); // Valor por defecto
    memset(telefono, 0, sizeof(telefono));
    memset(email, 0, sizeof(email));

    // Inicializar arrays
    for (int i = 0; i < 50; i++) pacientesIDs[i] = -1;
    for (int i = 0; i < 30; i++) citasIDs[i] = -1;

    // Timestamps
    time_t ahora = time(nullptr);
    fechaCreacion = ahora;
    fechaModificacion = ahora;
}

// Getters
int Doctor::getId() const { return id; }
const char* Doctor::getNombre() const { return nombre; }
const char* Doctor::getApellido() const { return apellido; }
const char* Doctor::getCedula() const { return cedula; }
const char* Doctor::getEspecialidad() const { return especialidad; }
int Doctor::getAniosExperiencia() const { return aniosExperiencia; }
float Doctor::getCostoConsulta() const { return costoConsulta; }
const char* Doctor::getHorarioAtencion() const { return horarioAtencion; }
const char* Doctor::getTelefono() const { return telefono; }
const char* Doctor::getEmail() const { return email; }
bool Doctor::isDisponible() const { return disponible; }
bool Doctor::isEliminado() const { return eliminado; }
int Doctor::getCantidadPacientes() const { return cantidadPacientes; }
int Doctor::getCantidadCitas() const { return cantidadCitas; }
const int* Doctor::getPacientesIDs() const { return pacientesIDs; }
const int* Doctor::getCitasIDs() const { return citasIDs; }
time_t Doctor::getFechaCreacion() const { return fechaCreacion; }
time_t Doctor::getFechaModificacion() const { return fechaModificacion; }

// Setters
void Doctor::setId(int id) { 
    this->id = id; 
    actualizarTimestamp();
}

void Doctor::setNombre(const char* nombre) {
    if (nombre) {
        strncpy(this->nombre, nombre, sizeof(this->nombre) - 1);
        this->nombre[sizeof(this->nombre) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setApellido(const char* apellido) {
    if (apellido) {
        strncpy(this->apellido, apellido, sizeof(this->apellido) - 1);
        this->apellido[sizeof(this->apellido) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setCedula(const char* cedula) {
    if (cedula) {
        strncpy(this->cedula, cedula, sizeof(this->cedula) - 1);
        this->cedula[sizeof(this->cedula) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setEspecialidad(const char* especialidad) {
    if (especialidad) {
        strncpy(this->especialidad, especialidad, sizeof(this->especialidad) - 1);
        this->especialidad[sizeof(this->especialidad) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setAniosExperiencia(int anios) {
    if (validarAniosExperiencia(anios)) {
        this->aniosExperiencia = anios;
        actualizarTimestamp();
    }
}

void Doctor::setCostoConsulta(float costo) {
    if (validarCostoConsulta(costo)) {
        this->costoConsulta = costo;
        actualizarTimestamp();
    }
}

void Doctor::setHorarioAtencion(const char* horario) {
    if (horario) {
        strncpy(this->horarioAtencion, horario, sizeof(this->horarioAtencion) - 1);
        this->horarioAtencion[sizeof(this->horarioAtencion) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setTelefono(const char* telefono) {
    if (telefono) {
        strncpy(this->telefono, telefono, sizeof(this->telefono) - 1);
        this->telefono[sizeof(this->telefono) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setEmail(const char* email) {
    if (email) {
        strncpy(this->email, email, sizeof(this->email) - 1);
        this->email[sizeof(this->email) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Doctor::setDisponible(bool disponible) {
    this->disponible = disponible;
    actualizarTimestamp();
}

void Doctor::setEliminado(bool eliminado) {
    this->eliminado = eliminado;
    actualizarTimestamp();
}

void Doctor::setFechaModificacion(time_t fecha) {
    this->fechaModificacion = fecha;
}

// Métodos de gestión de pacientes
bool Doctor::agregarPacienteID(int idPaciente) {
    if (cantidadPacientes >= 50 || idPaciente <= 0) return false;
    
    // Verificar si ya existe
    if (tienePaciente(idPaciente)) return false;
    
    pacientesIDs[cantidadPacientes++] = idPaciente;
    actualizarTimestamp();
    return true;
}

bool Doctor::eliminarPacienteID(int idPaciente) {
    for (int i = 0; i < cantidadPacientes; i++) {
        if (pacientesIDs[i] == idPaciente) {
            // Mover los elementos restantes
            for (int j = i; j < cantidadPacientes - 1; j++) {
                pacientesIDs[j] = pacientesIDs[j + 1];
            }
            pacientesIDs[--cantidadPacientes] = -1;
            actualizarTimestamp();
            return true;
        }
    }
    return false;
}

bool Doctor::tienePaciente(int idPaciente) const {
    for (int i = 0; i < cantidadPacientes; i++) {
        if (pacientesIDs[i] == idPaciente) return true;
    }
    return false;
}

void Doctor::limpiarPacientes() {
    for (int i = 0; i < 50; i++) {
        pacientesIDs[i] = -1;
    }
    cantidadPacientes = 0;
    actualizarTimestamp();
}

// Métodos de gestión de citas
bool Doctor::agregarCitaID(int idCita) {
    if (cantidadCitas >= 30 || idCita <= 0) return false;
    
    // Verificar si ya existe
    if (tieneCita(idCita)) return false;
    
    citasIDs[cantidadCitas++] = idCita;
    actualizarTimestamp();
    return true;
}

bool Doctor::eliminarCitaID(int idCita) {
    for (int i = 0; i < cantidadCitas; i++) {
        if (citasIDs[i] == idCita) {
            // Mover los elementos restantes
            for (int j = i; j < cantidadCitas - 1; j++) {
                citasIDs[j] = citasIDs[j + 1];
            }
            citasIDs[--cantidadCitas] = -1;
            actualizarTimestamp();
            return true;
        }
    }
    return false;
}

bool Doctor::tieneCita(int idCita) const {
    for (int i = 0; i < cantidadCitas; i++) {
        if (citasIDs[i] == idCita) return true;
    }
    return false;
}

void Doctor::limpiarCitas() {
    for (int i = 0; i < 30; i++) {
        citasIDs[i] = -1;
    }
    cantidadCitas = 0;
    actualizarTimestamp();
}

// Métodos de utilidad
void Doctor::actualizarTimestamp() {
    fechaModificacion = time(nullptr);
}

void Doctor::mostrarInfo() const {
    cout << "\n=== INFORMACION COMPLETA DEL DOCTOR ===" << endl;
    cout << "ID: " << id << endl;
    cout << "Nombre: Dr. " << nombre << " " << apellido << endl;
    cout << "Cédula: " << cedula << endl;
    cout << "Especialidad: " << especialidad << endl;
    cout << "Años experiencia: " << aniosExperiencia << endl;
    cout << "Costo consulta: $" << fixed << setprecision(2) << costoConsulta << endl;
    cout << "Horario: " << horarioAtencion << endl;
    cout << "Teléfono: " << telefono << endl;
    cout << "Email: " << email << endl;
    cout << "Pacientes asignados: " << cantidadPacientes << endl;
    cout << "Citas pendientes: " << cantidadCitas << endl;
    
    char fechaCreacionStr[20], fechaModificacionStr[20];
    strftime(fechaCreacionStr, sizeof(fechaCreacionStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaCreacion));
    strftime(fechaModificacionStr, sizeof(fechaModificacionStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaModificacion));
    
    cout << "Fecha de registro: " << fechaCreacionStr << endl;
    cout << "Última modificación: " << fechaModificacionStr << endl;
    cout << "Estado: " << (eliminado ? "ELIMINADO" : (disponible ? "DISPONIBLE" : "NO DISPONIBLE")) << endl;
}

void Doctor::mostrarResumen() const {
    cout << "    RESUMEN DEL DOCTOR:" << endl;
    cout << "   +-- ID: " << id << endl;
    cout << "   +-- Nombre: Dr. " << nombre << " " << apellido << endl;
    cout << "   +-- Cédula: " << cedula << endl;
    cout << "   +-- Especialidad: " << especialidad << endl;
    cout << "   +-- Experiencia: " << aniosExperiencia << " años" << endl;
    cout << "   +-- Costo consulta: $" << fixed << setprecision(2) << costoConsulta << endl;
    cout << "   +-- Teléfono: " << telefono << endl;
    
    char fechaStr[20];
    strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaCreacion));
    cout << "    Registrado: " << fechaStr << endl;
}

// Validaciones
bool Doctor::esValido() const {
    return (id > 0 && 
            strlen(nombre) > 0 && 
            strlen(apellido) > 0 && 
            strlen(cedula) > 0 && 
            strlen(especialidad) > 0 &&
            validarCostoConsulta(costoConsulta) &&
            validarAniosExperiencia(aniosExperiencia));
}

bool Doctor::validarCostoConsulta(float costo) {
    return costo >= 0;
}

bool Doctor::validarAniosExperiencia(int anios) {
    return anios >= 0;
}