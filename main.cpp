#include <iostream>
#include <fstream>
#include "../Menus/Menu.hpp"
#include "../persistencia/GestorArchivos.hpp"

using namespace std;

int main() {
    cout << "=== SISTEMA HOSPITALARIO ===" << endl;
    
    // 1. Crear directorio datos si no existe (PERO NO los archivos)
    GestorArchivos::crearDirectorioSiNoExiste("datos");
    
    // 2. Verificar archivos existentes (SOLO INFORMATIVO)
    cout << "\n=== VERIFICANDO ARCHIVOS EXISTENTES ===" << endl;
    
    const char* archivos[] = {
        "datos\\pacientes.bin",
        "datos\\doctores.bin", 
        "datos\\citas.bin",
        "datos\\historiales.bin",
        "datos\\hospital.bin"
    };
    
    for (int i = 0; i < 5; i++) {
        ifstream test(archivos[i], ios::binary);
        if (test.is_open()) {
            test.seekg(0, ios::end);
            long tamanio = test.tellg();
            test.close();
            cout << "✓ " << archivos[i] << " existe (" << tamanio << " bytes)" << endl;
        } else {
            cout << "✗ " << archivos[i] << " no existe (se creará cuando sea necesario)" << endl;
        }
    }
    
    // 3. MENSAJE IMPORTANTE
    cout << "\nNOTA: Los archivos se crearán automáticamente al agregar datos." << endl;
    cout << "No se borrarán datos existentes.\n" << endl;
    
    int opcion;
    
    do {
        Menus::mostrarMenuPrincipal();
        cin >> opcion;
        
        if (opcion < 0 || opcion > 5) {
            cout << "Error: Opcion invalida. Intente de nuevo." << endl;
            opcion = -1;
        }
        
        cin.ignore();
        
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