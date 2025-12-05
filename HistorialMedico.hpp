#ifndef HISTORIALMEDICO_HPP
#define HISTORIALMEDICO_HPP

#include <ctime>
#include <cstring>

class HistorialMedico {
private:
    int id;                    // ID único de la consulta
    int pacienteID;            // ID del paciente
    int idDoctor;              // ID del doctor que atendió
    bool eliminado;
    time_t fechaRegistro;      // Timestamp del registro en sistema
    int siguienteConsultaID;   // Para lista enlazada (-1 si es el último)
    
   
    char fecha[11];           // Fecha de la consulta (YYYY-MM-DD)
    char hora[6];             // Hora de la consulta (HH:MM)
    char diagnostico[200];
    char tratamiento[200];
    char medicamentos[150];
    float costo;

public:
    // Constructores
    HistorialMedico();
    HistorialMedico(int pacienteID, int idDoctor, const char* fecha, const char* hora);
    
    // Getters
    int getId() const;
    int getPacienteID() const;
    int getIdDoctor() const;
    bool isEliminado() const;
    time_t getFechaRegistro() const;
    int getSiguienteConsultaID() const;
    const char* getFecha() const;
    const char* getHora() const;
    const char* getDiagnostico() const;
    const char* getTratamiento() const;
    const char* getMedicamentos() const;
    float getCosto() const;

    // Setters
    void setId(int id);
    void setPacienteID(int pacienteID);
    void setIdDoctor(int idDoctor);
    void setEliminado(bool eliminado);
    void setFechaRegistro(time_t fechaRegistro);
    void setSiguienteConsultaID(int siguienteConsultaID);
    void setFecha(const char* fecha);
    void setHora(const char* hora);
    void setDiagnostico(const char* diagnostico);
    void setTratamiento(const char* tratamiento);
    void setMedicamentos(const char* medicamentos);
    void setCosto(float costo);

    // Métodos de utilidad
    void inicializar();
    void actualizarTimestamp();
    void mostrarInfo() const;
    void mostrarResumen() const;

    // Validaciones
    bool esValido() const;
    static bool validarFecha(const char* fecha);
    static bool validarHora(const char* hora);
    static bool validarCosto(float costo);
};

#endif
