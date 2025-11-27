#ifndef CITA_HPP
#define CITA_HPP

#include <cstring>
#include <ctime>

class Cita {
private:
    int id;
    int idPaciente;
    int idDoctor;
    char fecha[11];     // YYYY-MM-DD
    char hora[6];       // HH:MM
    char motivo[150];
    char estado[20];    // "AGENDADA", "CANCELADA", "ATENDIDA"
    char observaciones[200];
    bool atendida;
    bool eliminado;

public:
    // Constructores
    Cita();
    Cita(int idPaciente, int idDoctor, const char* fecha, const char* hora, const char* motivo);
    
    // Getters
    int getId() const;
    int getIdPaciente() const;
    int getIdDoctor() const;
    const char* getFecha() const;
    const char* getHora() const;
    const char* getMotivo() const;
    const char* getEstado() const;
    const char* getObservaciones() const;
    bool isAtendida() const;
    bool isEliminado() const;

    // Setters
    void setId(int id);
    void setIdPaciente(int idPaciente);
    void setIdDoctor(int idDoctor);
    void setFecha(const char* fecha);
    void setHora(const char* hora);
    void setMotivo(const char* motivo);
    void setEstado(const char* estado);
    void setObservaciones(const char* observaciones);
    void setAtendida(bool atendida);
    void setEliminado(bool eliminado);

    // Métodos de utilidad
    void inicializar();
    void mostrarInfo() const;
    void marcarComoAtendida(const char* observaciones = "");
    void marcarComoCancelada(const char* observaciones = "");

    // Validaciones
    bool esValida() const;
    static bool validarFecha(const char* fecha);
    static bool validarHora(const char* hora);
};

#endif
