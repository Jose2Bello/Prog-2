#ifndef MENUS_HPP
#define MENUS_HPP

#include "../Pacientes/operacionesPacientes.hpp"
#include "../Doctores/operacionesDoctores.hpp"
#include "../Citas/operacionesCitas.hpp"
#include "../Historial/operacionesHistorial.hpp"
#include "../Utilidades/Utilidades.hpp"

class Menus {
public:
    // Menú Principal
    static void mostrarMenuPrincipal();
    
    // Menús Específicos
    static void menuGestionPacientes();
    static void menuGestionDoctores();
    static void menuGestionCitas();
    static void menuHistorialMedico();
    static void menuMantenimientoSistema();
    
private:
    // Funciones helper para los menús
    static void mostrarEncabezado(const std::string& titulo);
    static int leerOpcionMenu(const std::string& mensaje, int min, int max);
};

#endif
