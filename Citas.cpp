#include "../include/Citas.hpp"
#include <iostream>
#include <cstring>
#include <iomanip>

using namespace std;

Cita::Cita() {
    inicializar();
}

Cita::Cita(int idPaciente, int idDoctor, const char* fecha, const char* hora, const char* motivo) {
    inicializar();
    
    setIdPaciente(idPaciente);
    setIdDoctor(idDoctor);
    setFecha(fecha);
    setHora(hora);
    setMotivo(motivo);
    setEstado("AGENDADA");
}

void Cita::inicializar() {
    id = -1;
    idPaciente = -1;
    idDoctor = -1;
    atendida = false;
    eliminado = false;
    
    strcpy(fecha, "");
    strcpy(hora, "");
    strcpy(motivo, "");
    strcpy(estado, "AGENDADA");
    strcpy(observaciones, "");
}

// Getters
int Cita::getId() const { return id; }
int Cita::getIdPaciente() const { return idPaciente; }
int Cita::getIdDoctor() const { return idDoctor; }
const char* Cita::getFecha() const { return fecha; }
const char* Cita::getHora() const { return hora; }
const char* Cita::getMotivo() const { return motivo; }
const char* Cita::getEstado() const { return estado; }
const char* Cita::getObservaciones() const { return observaciones; }
bool Cita::isAtendida() const { return atendida; }
bool Cita::isEliminado() const { return eliminado; }

// Setters
void Cita::setId(int id) { this->id = id; }
void Cita::setIdPaciente(int idPaciente) { this->idPaciente = idPaciente; }
void Cita::setIdDoctor(int idDoctor) { this->idDoctor = idDoctor; }

void Cita::setFecha(const char* fecha) {
    if (fecha && validarFecha(fecha)) {
        strncpy(this->fecha, fecha, sizeof(this->fecha) - 1);
        this->fecha[sizeof(this->fecha) - 1] = '\0';
    }
}

void Cita::setHora(const char* hora) {
    if (hora && validarHora(hora)) {
        strncpy(this->hora, hora, sizeof(this->hora) - 1);
        this->hora[sizeof(this->hora) - 1] = '\0';
    }
}

void Cita::setMotivo(const char* motivo) {
    if (motivo) {
        strncpy(this->motivo, motivo, sizeof(this->motivo) - 1);
        this->motivo[sizeof(this->motivo) - 1] = '\0';
    }
}

void Cita::setEstado(const char* estado) {
    if (estado) {
        strncpy(this->estado, estado, sizeof(this->estado) - 1);
        this->estado[sizeof(this->estado) - 1] = '\0';
    }
}

void Cita::setObservaciones(const char* observaciones) {
    if (observaciones) {
        strncpy(this->observaciones, observaciones, sizeof(this->observaciones) - 1);
        this->observaciones[sizeof(this->observaciones) - 1] = '\0';
    }
}

void Cita::setAtendida(bool atendida) { 
    this->atendida = atendida; 
    if (atendida) {
        setEstado("ATENDIDA");
    }
}

void Cita::setEliminado(bool eliminado) { this->eliminado = eliminado; }

// Métodos de utilidad
void Cita::mostrarInfo() const {
    cout << "\n=== INFORMACION DE CITA ===" << endl;
    cout << "ID: " << id << endl;
    cout << "Paciente ID: " << idPaciente << endl;
    cout << "Doctor ID: " << idDoctor << endl;
    cout << "Fecha: " << fecha << " " << hora << endl;
    cout << "Motivo: " << motivo << endl;
    cout << "Estado: " << estado << endl;
    cout << "Observaciones: " << observaciones << endl;
    cout << "Atendida: " << (atendida ? "SI" : "NO") << endl;
}

void Cita::marcarComoAtendida(const char* observaciones) {
    setAtendida(true);
    setEstado("ATENDIDA");
    if (observaciones) {
        setObservaciones(observaciones);
    }
}

void Cita::marcarComoCancelada(const char* observaciones) {
    setAtendida(false);
    setEstado("CANCELADA");
    if (observaciones) {
        setObservaciones(observaciones);
    }
}

// Validaciones
bool Cita::esValida() const {
    return (id > 0 && 
            idPaciente > 0 && 
            idDoctor > 0 && 
            validarFecha(fecha) && 
            validarHora(hora) && 
            strlen(motivo) > 0);
}

bool Cita::validarFecha(const char* fecha) {
    if (!fecha || strlen(fecha) != 10) return false;
    if (fecha[4] != '-' || fecha[7] != '-') return false;
    
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(fecha[i])) return false;
    }
    
    return true;
}

bool Cita::validarHora(const char* hora) {
    if (!hora || strlen(hora) != 5) return false;
    if (hora[2] != ':') return false;
    
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) return false;
    }
    
    int horas = atoi(hora);
    int minutos = atoi(hora + 3);
    return (horas >= 0 && horas <= 23 && minutos >= 0 && minutos <= 59);
}
