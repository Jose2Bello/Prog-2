#ifndef OPERACIONES_DOCTORES_HPP
#define OPERACIONES_DOCTORES_HPP

#include "Doctor.hpp"

class OperacionesDoctores {
public:
    // Operaciones CRUD
    static bool agregarDoctor(Doctor& doctor);
    static bool actualizarDoctor(const Doctor& doctor);
    static bool eliminarDoctor(int id, bool confirmar = true);
    static bool restaurarDoctor(int id);
    
    // Búsquedas
    static Doctor buscarPorID(int id);
    static Doctor buscarPorCedula(const char* cedula);
    static int buscarPorEspecialidad(const char* especialidad, Doctor* resultados, int maxResultados);
    static int buscarPorNombre(const char* nombre, Doctor* resultados, int maxResultados);
    
    // Listados
    static void listarDoctores(bool mostrarEliminados = false);
    static void listarDoctoresEliminados();
    
    // Utilidades
    static int contarDoctores();
    static int contarDoctoresEliminados();
    static bool existeDoctorConCedula(const char* cedula);
    static int leerTodosDoctores(Doctor* doctores, int maxDoctores, bool soloDisponibles = true);
    
private:
    static int buscarIndicePorID(int id);
    static long calcularPosicion(int indice);
};

#endif
