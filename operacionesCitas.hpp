#ifndef OPERACIONES_CITAS_HPP
#define OPERACIONES_CITAS_HPP

#include "Citas.hpp"

class OperacionesCitas {
public:
    // Operaciones CRUD
    static bool agendarCita(int idPaciente, int idDoctor, const char* fecha, 
                           const char* hora, const char* motivo);
    static bool actualizarCita(const Cita& cita);
    static bool cancelarCita(int idCita);
    static bool atenderCita(int idCita, const char* diagnostico, const char* tratamiento, 
                           const char* medicamentos);
    
    // Búsquedas
    static Cita buscarPorID(int id);
    static int buscarCitasDePaciente(int idPaciente, Cita* resultados, int maxResultados);
    static int buscarCitasDeDoctor(int idDoctor, Cita* resultados, int maxResultados);
    static int buscarCitasPorFecha(const char* fecha, Cita* resultados, int maxResultados);
    
    // Listados
    static void listarCitasPendientes();
    static void listarTodasCitas(bool mostrarCanceladas = false);
    
    // Utilidades
    static int contarCitas();
    static int contarCitasPendientes();
    static bool verificarDisponibilidad(int idDoctor, const char* fecha, const char* hora);
    
private:
    static int buscarIndicePorID(int id);
    static long calcularPosicion(int indice);
};

#endif
