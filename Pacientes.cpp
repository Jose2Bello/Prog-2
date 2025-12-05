#include "Pacientes.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

// Constructor por defecto
Paciente::Paciente() {
    inicializar();
}

// Constructor con parámetros básicos
Paciente::Paciente(const char* nombre, const char* apellido, 
                   const char* cedula, int edad, char sexo) {
    inicializar();
    
    setNombre(nombre);
    setApellido(apellido);
    setCedula(cedula);
    setEdad(edad);
    setSexo(sexo);
}

void Paciente::inicializar() {
    id = -1;
    edad = 0;
    sexo = 'M';
    activo = true;
    eliminado = false;
    cantidadConsultas = 0;
    primerConsultaID = -1;
    cantidadCitas = 0;
    
    
    strcpy(nombre, "");
    strcpy(apellido, "");
    strcpy(cedula, "");
    strcpy(tiposangre, "");
    strcpy(telefono, "");
    strcpy(direccion, "");
    strcpy(email, "");
    strcpy(alergias, "");
    strcpy(observaciones, "");
    
  
    for (int i = 0; i < 20; i++) {
        citasIDs[i] = -1;
    }
    
    // Timestamps
    time_t ahora = time(nullptr);
    fechaCreacion = ahora;
    fechaModificacion = ahora;
}

// Getters
int Paciente::getId() const { return id; }
const char* Paciente::getNombre() const { return nombre; }
const char* Paciente::getApellido() const { return apellido; }
const char* Paciente::getCedula() const { return cedula; }
int Paciente::getEdad() const { return edad; }
char Paciente::getSexo() const { return sexo; }
const char* Paciente::getTipoSangre() const { return tiposangre; }
const char* Paciente::getTelefono() const { return telefono; }
const char* Paciente::getDireccion() const { return direccion; }
const char* Paciente::getEmail() const { return email; }
const char* Paciente::getAlergias() const { return alergias; }
const char* Paciente::getObservaciones() const { return observaciones; }
bool Paciente::isActivo() const { return activo; }
bool Paciente::isEliminado() const { return eliminado; }
int Paciente::getCantidadConsultas() const { return cantidadConsultas; }
int Paciente::getPrimerConsultaID() const { return primerConsultaID; }
int Paciente::getCantidadCitas() const { return cantidadCitas; }
const int* Paciente::getCitasIDs() const { return citasIDs; }
time_t Paciente::getFechaCreacion() const { return fechaCreacion; }
time_t Paciente::getFechaModificacion() const { return fechaModificacion; }

// Setters
void Paciente::setId(int id) { 
    this->id = id; 
    actualizarTimestamp();
}

void Paciente::setNombre(const char* nombre) {
    if (nombre) {
        strncpy(this->nombre, nombre, sizeof(this->nombre) - 1);
        this->nombre[sizeof(this->nombre) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setApellido(const char* apellido) {
    if (apellido) {
        strncpy(this->apellido, apellido, sizeof(this->apellido) - 1);
        this->apellido[sizeof(this->apellido) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setCedula(const char* cedula) {
    if (cedula) {
        strncpy(this->cedula, cedula, sizeof(this->cedula) - 1);
        this->cedula[sizeof(this->cedula) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setEdad(int edad) {
    if (validarEdad(edad)) {
        this->edad = edad;
        actualizarTimestamp();
    }
}

void Paciente::setSexo(char sexo) {
    if (validarSexo(sexo)) {
        this->sexo = sexo;
        actualizarTimestamp();
    }
}

void Paciente::setTipoSangre(const char* tipoSangre) {
    if (tipoSangre) {
        strncpy(this->tiposangre, tiposangre, sizeof(this->tiposangre) - 1);
        this->tiposangre[sizeof(this->tiposangre) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setTelefono(const char* telefono) {
    if (telefono) {
        strncpy(this->telefono, telefono, sizeof(this->telefono) - 1);
        this->telefono[sizeof(this->telefono) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setDireccion(const char* direccion) {
    if (direccion) {
        strncpy(this->direccion, direccion, sizeof(this->direccion) - 1);
        this->direccion[sizeof(this->direccion) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setEmail(const char* email) {
    if (email) {
        strncpy(this->email, email, sizeof(this->email) - 1);
        this->email[sizeof(this->email) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setAlergias(const char* alergias) {
    if (alergias) {
        strncpy(this->alergias, alergias, sizeof(this->alergias) - 1);
        this->alergias[sizeof(this->alergias) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setObservaciones(const char* observaciones) {
    if (observaciones) {
        strncpy(this->observaciones, observaciones, sizeof(this->observaciones) - 1);
        this->observaciones[sizeof(this->observaciones) - 1] = '\0';
        actualizarTimestamp();
    }
}

void Paciente::setActivo(bool activo) {
    this->activo = activo;
    actualizarTimestamp();
}

void Paciente::setEliminado(bool eliminado) {
    this->eliminado = eliminado;
    actualizarTimestamp();
}

void Paciente::setCantidadConsultas(int cantidad) {
    if (cantidad >= 0) {
        this->cantidadConsultas = cantidad;
        actualizarTimestamp();
    }
}

void Paciente::setPrimerConsultaID(int id) {
    this->primerConsultaID = id;
    actualizarTimestamp();
}

void Paciente::setFechaModificacion(time_t fecha) {
    this->fechaModificacion = fecha;
}

bool Paciente::eliminarCita(int idCita) {
    for (int i = 0; i < cantidadCitas; i++) {
        if (citasIDs[i] == idCita) {
            // Mover las citas restantes
            for (int j = i; j < cantidadCitas - 1; j++) {
                citasIDs[j] = citasIDs[j + 1];
            }
            citasIDs[cantidadCitas - 1] = -1;
            cantidadCitas--;
            actualizarTimestamp();
            return true;
        }
    }
    return false;
}

bool Paciente::tieneCita(int idCita) const {
    for (int i = 0; i < cantidadCitas; i++) {
        if (citasIDs[i] == idCita) {
            return true;
        }
    }
    return false;
}

void Paciente::limpiarCitas() {
    for (int i = 0; i < 20; i++) {
        citasIDs[i] = -1;
    }
    cantidadCitas = 0;
    actualizarTimestamp();
}

// Métodos de utilidad
void Paciente::actualizarTimestamp() {
    fechaModificacion = time(nullptr);
}

void Paciente::mostrarInfo() const {
    cout << "\n=== INFORMACION COMPLETA DEL PACIENTE ===" << endl;
    cout << "ID: " << id << endl;
    cout << "Nombre: " << nombre << " " << apellido << endl;
    cout << "Cedula: " << cedula << endl;
    cout << "Edad: " << edad << " anos" << endl;
    cout << "Sexo: " << (sexo == 'M' ? "Masculino" : "Femenino") << endl;
    cout << "Tipo de sangre: " << tiposangre << endl;
    cout << "Telefono: " << telefono << endl;
    cout << "Email: " << email << endl;
    cout << "Direccion: " << direccion << endl;
    cout << "Observaciones: " << observaciones << endl;
    cout << "Consultas realizadas: " << cantidadConsultas << endl;
    cout << "Citas agendadas: " << cantidadCitas << endl;
    
    char fechaCreacionStr[20], fechaModificacionStr[20];
    strftime(fechaCreacionStr, sizeof(fechaCreacionStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaCreacion));
    strftime(fechaModificacionStr, sizeof(fechaModificacionStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaModificacion));
    
    cout << "Fecha de registro: " << fechaCreacionStr << endl;
    cout << "Ultima modificacion: " << fechaModificacionStr << endl;
    cout << "Estado: " << (eliminado ? "ELIMINADO" : "ACTIVO") << endl;
}

void Paciente::mostrarResumen() const {
    cout << "    RESUMEN DEL PACIENTE:" << endl;
    cout << "   +-- ID: " << id << endl;
    cout << "   +-- Nombre: " << nombre << " " << apellido << endl;
    cout << "   +-- Cedula: " << cedula << endl;
    cout << "   +-- Edad: " << edad << " anos" << endl;
    cout << "   +-- Sexo: " << (sexo == 'M' ? "Masculino" : "Femenino") << endl;
    cout << "   +-- Telefono: " << telefono << endl;
    
    char fechaStr[20];
    strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d %H:%M", 
             localtime(&fechaCreacion));
    cout << "    Registrado: " << fechaStr << endl;
}

bool Paciente::esValido() const {
    return (id > 0 && 
            strlen(nombre) > 0 && 
            strlen(apellido) > 0 && 
            strlen(cedula) > 0 && 
            validarEdad(edad) && 
            validarSexo(sexo));
}

bool Paciente::validarSexo(char sexo) {
    return (sexo == 'M' || sexo == 'F');
}

bool Paciente::validarEdad(int edad) {
    return (edad >= 0 && edad <= 150);
}
