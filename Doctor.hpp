#ifndef DOCTOR_HPP
#define DOCTOR_HPP

#include <cstring>
#include <ctime>

class Doctor {
private:
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];
    char especialidad[50];
    int aniosExperiencia;
    float costoConsulta;
    char horarioAtencion[50];
    char telefono[15];
    char email[50];

    int pacientesIDs[50];
    int cantidadPacientes;
    
    int citasIDs[30];
    int cantidadCitas;

    bool disponible;
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;

public:
    // Constructores
    Doctor();
    Doctor(const char* nombre, const char* apellido, const char* cedula, 
           const char* especialidad, int aniosExperiencia, float costoConsulta);
    
    // Getters
    int getId() const;
    const char* getNombre() const;
    const char* getApellido() const;
    const char* getCedula() const;
    const char* getEspecialidad() const;
    int getAniosExperiencia() const;
    float getCostoConsulta() const;
    const char* getHorarioAtencion() const;
    const char* getTelefono() const;
    const char* getEmail() const;
    bool isDisponible() const;
    bool isEliminado() const;
    int getCantidadPacientes() const;
    int getCantidadCitas() const;
    const int* getPacientesIDs() const;
    const int* getCitasIDs() const;
    time_t getFechaCreacion() const;
    time_t getFechaModificacion() const;

    // Setters
    void setId(int id);
    void setNombre(const char* nombre);
    void setApellido(const char* apellido);
    void setCedula(const char* cedula);
    void setEspecialidad(const char* especialidad);
    void setAniosExperiencia(int anios);
    void setCostoConsulta(float costo);
    void setHorarioAtencion(const char* horario);
    void setTelefono(const char* telefono);
    void setEmail(const char* email);
    void setDisponible(bool disponible);
    void setEliminado(bool eliminado);
    void setFechaModificacion(time_t fecha);

    // Métodos de gestión
    bool agregarPacienteID(int idPaciente);
    bool eliminarPacienteID(int idPaciente);
    bool tienePaciente(int idPaciente) const;
    bool agregarCitaID(int idCita);
    bool eliminarCitaID(int idCita);
    bool tieneCita(int idCita) const;
    void limpiarPacientes();
    void limpiarCitas();

    // Métodos de utilidad
    void inicializar();
    void actualizarTimestamp();
    void mostrarInfo() const;
    void mostrarResumen() const;

    // Validaciones
    bool esValido() const;
    static bool validarCostoConsulta(float costo);
    static bool validarAniosExperiencia(int anios);
};

#endif