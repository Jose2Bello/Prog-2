#include <iostream>
#include "Menu/Menu.hpp"
#include "persistencia/GestorArchivos.hpp"

using namespace std;

int main() {
    cout << "==========================================" << endl;
    cout << "    SISTEMA HOSPITALARIO - VERSION ARCHIVOS" << endl;
    cout << "==========================================" << endl;

   
    if (!GestorArchivos::inicializarArchivo("datos/pacientes.bin", "PACIENTES") ||
        !GestorArchivos::inicializarArchivo("datos/doctores.bin", "DOCTORES") ||
        !GestorArchivos::inicializarArchivo("datos/citas.bin", "CITAS") ||
        !GestorArchivos::inicializarArchivo("datos/historiales.bin", "HISTORIALES")) {
        cout << "Advertencia: Hubo problemas al inicializar los archivos." << endl;
    }

    int opcion;
    
    do {
        Menus::mostrarMenuPrincipal();
        opcion = Menus::leerOpcionMenu("Seleccione una opcion", 0, 6);

        switch (opcion) {
            case 1:
                Menus::menuGestionPacientes();
                break;
            case 2:
                Menus::menuGestionDoctores();
                break;
            case 3:
                Menus::menuGestionCitas();
                break;
            case 4:
                Menus::menuHistorialMedico();
                break;
            case 5:
                Menus::menuMantenimientoSistema();
                break;
            case 0:
                cout << "Saliendo del programa..." << endl;
                break;
        }

    } while (opcion != 0);

    cout << "Programa terminado correctamente." << endl;
    return 0;
}