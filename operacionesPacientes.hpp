#ifndef OPERACIONES_PACIENTES_HPP
#define OPERACIONES_PACIENTES_HPP

#include "Pacientes.hpp"

class OperacionesPacientes {
public:
    // Operaciones CRUD
    static bool agregarPaciente(Paciente& paciente);
    static bool actualizarPaciente(const Paciente& paciente);
    static bool eliminarPaciente(int id, bool confirmar = true);
    static bool restaurarPaciente(int id);
    
    // Búsquedas
    static Paciente buscarPorID(int id);
    static Paciente buscarPorCedula(const char* cedula);
    static int buscarPorNombre(const char* nombre, Paciente* resultados, int maxResultados);
    
    // Listados
    static void listarPacientes(bool mostrarEliminados = false);
    static void listarPacientesEliminados();
    
    // Utilidades
    static int contarPacientes();
    static int contarPacientesEliminados();
    static bool existePacienteConCedula(const char* cedula);
    static int leerTodosPacientes(Paciente* pacientes, int maxPacientes, bool soloActivos = true);
    
private:
    static int buscarIndicePorID(int id);
    static long calcularPosicion(int indice);
};

#endif