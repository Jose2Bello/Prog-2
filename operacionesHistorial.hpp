#ifndef OPERACIONES_HISTORIAL_HPP
#define OPERACIONES_HISTORIAL_HPP

#include "HistorialMedico.hpp"

class OperacionesHistorial {
public:
    // Operaciones CRUD
    
    static bool agregarConsultaAlHistorial(int pacienteID, HistorialMedico nuevaConsulta);
    static bool agregarConsulta(HistorialMedico& consulta);
    static bool actualizarConsulta(const HistorialMedico& consulta);
    static bool eliminarConsulta(int id);
    
    // Búsquedas
    static HistorialMedico buscarPorID(int id);
    static HistorialMedico buscarUltimaConsulta(int pacienteID);
    static int obtenerHistorialCompleto(int pacienteID, HistorialMedico* resultados, int maxResultados);
    
    // Listados
    static void mostrarHistorialMedico(int pacienteID);
    static void listarConsultasPorDoctor(int idDoctor);
    static void listarConsultasPorFecha(const char* fecha);
    
    // Utilidades
    static int contarConsultas();
    static int contarConsultasDePaciente(int pacienteID);
    static float calcularTotalIngresos();

private:
    static int buscarIndicePorID(int id);
    static long calcularPosicion(int indice);
    static HistorialMedico crearConsultaVacia();
};

#endif
