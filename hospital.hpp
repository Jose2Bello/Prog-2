#ifndef HOSPITAL_HPP
#define HOSPITAL_HPP

#include <cstring>

class Hospital {

private:
    char nombre[100];
    char direccion[150];
    char telefono[15];

    int siguienteIDPaciente;
    int siguienteIDDoctor;
    int siguienteIDCita;
    int siguienteIDConsulta;

    int totalPacientesRegistrados;
    int totalDoctoresRegistrados;
    int totalCitasAgendadas;
    int totalConsultasRealizadas;

public:
    
    Hospital();

    
    const char* getNombre() const;
    void setNombre(const char* n);

    const char* getDireccion() const;
    void setDireccion(const char* d);

    const char* getTelefono() const;
    void setTelefono(const char* t);

    // Getters y Setters para contadores
    int getSiguienteIDPaciente() const;
    void incrementarIDPaciente();

    int getSiguienteIDDoctor() const;
    void incrementarIDDoctor();

    int getSiguienteIDCita() const;
    void incrementarIDCita();

    int getSiguienteIDConsulta() const;
    void incrementarIDConsulta();
};

#endif