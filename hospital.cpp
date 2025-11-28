#include "../include/hospital.hpp"
#include <cstring>

Hospital::Hospital() {
    // Inicializar cadenas vacías
    memset(nombre, 0, sizeof(nombre));
    memset(direccion, 0, sizeof(direccion));
    memset(telefono, 0, sizeof(telefono));

    // Valores por defecto
    siguienteIDPaciente = 1;
    siguienteIDDoctor = 1;
    siguienteIDCita = 1;
    siguienteIDConsulta = 1;

    totalPacientesRegistrados = 0;
    totalDoctoresRegistrados = 0;
    totalCitasAgendadas = 0;
    totalConsultasRealizadas = 0;
}

const char* Hospital::getNombre() const { return nombre; }
void Hospital::setNombre(const char* n) { 
    strncpy(nombre, n, sizeof(nombre) - 1); 
    nombre[sizeof(nombre) - 1] = '\0';
}

const char* Hospital::getDireccion() const { return direccion; }
void Hospital::setDireccion(const char* d) { 
    strncpy(direccion, d, sizeof(direccion) - 1);
    direccion[sizeof(direccion) - 1] = '\0';
}

const char* Hospital::getTelefono() const { return telefono; }
void Hospital::setTelefono(const char* t) { 
    strncpy(telefono, t, sizeof(telefono) - 1);
    telefono[sizeof(telefono) - 1] = '\0';
}

int Hospital::getSiguienteIDPaciente() const { return siguienteIDPaciente; }
void Hospital::incrementarIDPaciente() { siguienteIDPaciente++; }

int Hospital::getSiguienteIDDoctor() const { return siguienteIDDoctor; }
void Hospital::incrementarIDDoctor() { siguienteIDDoctor++; }

int Hospital::getSiguienteIDCita() const { return siguienteIDCita; }
void Hospital::incrementarIDCita() { siguienteIDCita++; }

int Hospital::getSiguienteIDConsulta() const { return siguienteIDConsulta; }
void Hospital::incrementarIDConsulta() { siguienteIDConsulta++; }
