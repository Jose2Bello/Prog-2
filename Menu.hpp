#ifndef MENUS_HPP
#define MENUS_HPP

#include "../pacientes/operacionesPacientes.hpp"
#include "../doctores/operacionesDoctores.hpp"
#include "../citas/operacionesCitas.hpp"
#include "../historial/operacionesHistorial.hpp"
#include "../utilidades/Utilidades.hpp"

class Menus {
public:
    // Menú Principal
    static void mostrarMenuPrincipal();
    
    // Menús Específicos
    static void menuGestionPacientes();
    static void menuGestionDoctores();
    static void menuGestionCitas();
    static void menuHistorialMedico();
    static void menuReportesEstadisticas();
    static void menuMantenimientoSistema();
    
private:
    // Funciones helper para los menús
    static void mostrarEncabezado(const std::string& titulo);
    static int leerOpcionMenu(const std::string& mensaje, int min, int max);
};

#endif