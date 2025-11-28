#include "../include/Menu.hpp"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "../include/Utilidades.hpp"
#include "../include/GestorArchivos.hpp"
#include "../include/Constantes.hpp"
using namespace std;

// ==================== MENUS PUBLICOS ====================

void Menus::mostrarMenuPrincipal() {
    Formatos::limpiarPantalla();
    
    cout << "\n\n\n";
    cout << ("===============================================");
    cout << ("       HOSPITAL EL CALLAO V3");
    cout << ("===============================================");
    cout << ("1.  Gestion de Pacientes");
    cout << ("2.  Gestion de Doctores");
    cout << ("3.  Gestion de Citas");
    cout << ("4.  Historial Medico");
    cout << ("5.  Mantenimiento del Sistema");
    cout << ("0.  Salir");
    cout << ("===============================================");
    cout << "\n\n";
}

void Menus::menuGestionPacientes() {
    int opcion;
    
    do {
        Formatos::limpiarPantalla();
        mostrarEncabezado("GESTION DE PACIENTES");
        
        cout << "1.  Registrar nuevo paciente" << endl;
        cout << "2.  Buscar paciente por ID" << endl;
        cout << "3.  Buscar paciente por cedula" << endl;
        cout << "4.  Buscar pacientes por nombre" << endl;
        cout << "5.  Actualizar datos de paciente" << endl;
        cout << "6.  Listar todos los pacientes" << endl;
        cout << "7.  Eliminar paciente (logico)" << endl;
        cout << "8.  Restaurar paciente eliminado" << endl;
        cout << "9.  Listar pacientes eliminados" << endl;
        cout << "10. Ver informacion completa de paciente" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 10);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- REGISTRAR NUEVO PACIENTE ---" << endl;
                
                string nombre = EntradaUsuario::leerTexto("Nombre", 49);
                string apellido = EntradaUsuario::leerTexto("Apellido", 49);
                string cedula = EntradaUsuario::leerTexto("Cedula", 19);
                int edad = EntradaUsuario::leerEntero("Edad", 0, 150);
                char sexo;
                
                do {
                    string sexoStr = EntradaUsuario::leerTexto("Sexo (M/F)", 1);
                    sexo = toupper(sexoStr[0]);
                    if (!Validaciones::validarSexo(sexo)) {
                        cout << "Error: Sexo debe ser M o F" << endl;
                    }
                } while (!Validaciones::validarSexo(sexo));
                
                Paciente nuevoPaciente(nombre.c_str(), apellido.c_str(), 
                                     cedula.c_str(), edad, sexo);
                
                if (OperacionesPacientes::agregarPaciente(nuevoPaciente)) {
                    cout << "Paciente registrado exitosamente." << endl;
                } else {
                    cout << "Error al registrar paciente." << endl;
                }
                break;
            }
            
            case 2: {
                cout << "\n--- BUSCAR PACIENTE POR ID ---" << endl;
                int id = EntradaUsuario::leerEntero("Ingrese ID del paciente", 1, 10000);
                
                Paciente paciente = OperacionesPacientes::buscarPorID(id);
                if (paciente.getId() != -1) {
                    paciente.mostrarInfo();
                }
                break;
            }
            
            case 3: {
                cout << "\n--- BUSCAR PACIENTE POR CEDULA ---" << endl;
                string cedula = EntradaUsuario::leerTexto("Ingrese cedula", 19);
                
                Paciente paciente = OperacionesPacientes::buscarPorCedula(cedula.c_str());
                if (paciente.getId() != -1) {
                    paciente.mostrarInfo();
                }
                break;
            }
            
            case 6: {
                cout << "\n--- LISTA DE PACIENTES ---" << endl;
                OperacionesPacientes::listarPacientes();
                break;
            }
            
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
                
            default:
                cout << "Opcion en desarrollo..." << endl;
                break;
        }
        
        if (opcion != 0) {
            Formatos::pausarPantalla();
        }
        
    } while (opcion != 0);
}

void Menus::menuGestionDoctores() {
    int opcion;
    
    do {
        Formatos::limpiarPantalla();
        mostrarEncabezado("GESTION DE DOCTORES");
        
        cout << "1.  Registrar nuevo doctor" << endl;
        cout << "2.  Buscar doctor por ID" << endl;
        cout << "3.  Buscar doctor por cedula" << endl;
        cout << "4.  Buscar doctores por especialidad" << endl;
        cout << "5.  Actualizar datos de doctor" << endl;
        cout << "6.  Listar todos los doctores" << endl;
        cout << "7.  Eliminar doctor (logico)" << endl;
        cout << "8.  Restaurar doctor eliminado" << endl;
        cout << "9.  Listar doctores eliminados" << endl;
        cout << "10. Asignar paciente a doctor" << endl;
        cout << "11. Ver pacientes de un doctor" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 11);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- REGISTRAR NUEVO DOCTOR ---" << endl;
                
                string nombre = EntradaUsuario::leerTexto("Nombre", 49);
                string apellido = EntradaUsuario::leerTexto("Apellido", 49);
                string cedula = EntradaUsuario::leerTexto("Cedula", 19);
                string especialidad = EntradaUsuario::leerTexto("Especialidad", 49);
                int experiencia = EntradaUsuario::leerEntero("Anios de experiencia", 0, 60);
                float costo = EntradaUsuario::leerEntero("Costo de consulta", 0, 10000);   
                // Crear el objeto Doctor con los datos ingresados y pasarlo a la operación
                Doctor nuevoDoctor(nombre.c_str(), apellido.c_str(), cedula.c_str(), 
                                   especialidad.c_str(), experiencia, costo);
                
                if (OperacionesDoctores::agregarDoctor(nuevoDoctor)) {
                    cout << "Doctor registrado exitosamente." << endl;
                } else {
                    cout << "Error al registrar doctor." << endl;
                }
                break;
            }
            
            case 2: {
                cout << "\n--- BUSCAR DOCTOR POR ID ---" << endl;
                int id = EntradaUsuario::leerEntero("Ingrese ID del doctor", 1, 10000);
                
                Doctor doctor = OperacionesDoctores::buscarPorID(id);
                if (doctor.getId() != -1) {
                    doctor.mostrarInfo();
                }
                break;
            }
            
            case 6: {
                cout << "\n--- LISTA DE DOCTORES ---" << endl;
                OperacionesDoctores::listarDoctores();
                break;
            }
            
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
                
            default:
                cout << "Opcion en desarrollo..." << endl;
                break;
        }
        
        if (opcion != 0) {
            Formatos::pausarPantalla();
        }
        
    } while (opcion != 0);
}

void Menus::menuGestionCitas() {
    int opcion;
    
    do {
        Formatos::limpiarPantalla();
        mostrarEncabezado("GESTION DE CITAS");
        
        cout << "1.  Agendar nueva cita" << endl;
        cout << "2.  Cancelar cita" << endl;
        cout << "3.  Atender cita" << endl;
        cout << "4.  Ver citas de un paciente" << endl;
        cout << "5.  Ver citas de un doctor" << endl;
        cout << "6.  Ver citas por fecha" << endl;
        cout << "7.  Ver citas pendientes" << endl;
        cout << "8.  Verificar disponibilidad de doctor" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 8);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- AGENDAR NUEVA CITA ---" << endl;
                
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                string fecha = EntradaUsuario::leerTexto("Fecha (YYYY-MM-DD)", 10);
                string hora = EntradaUsuario::leerTexto("Hora (HH:MM)", 5);
                string motivo = EntradaUsuario::leerTexto("Motivo", 149);
                
                if (OperacionesCitas::agendarCita(idPaciente, idDoctor, 
                                                fecha.c_str(), hora.c_str(), 
                                                motivo.c_str())) {
                    cout << "Cita agendada exitosamente." << endl;
                } else {
                    cout << "Error al agendar cita." << endl;
                }
                break;
            }
            
            case 7: {
                cout << "\n--- CITAS PENDIENTES ---" << endl;
                OperacionesCitas::listarCitasPendientes();
                break;
            }
            
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
                
            default:
                cout << "Opcion en desarrollo..." << endl;
                break;
        }
        
        if (opcion != 0) {
            Formatos::pausarPantalla();
        }
        
    } while (opcion != 0);
}

void Menus::menuHistorialMedico() {
    int opcion;
    
    do {
        Formatos::limpiarPantalla();
        mostrarEncabezado("HISTORIAL MEDICO");
        
        cout << "1.  Ver historial completo de paciente" << endl;
        cout << "2.  Agregar consulta al historial" << endl;
        cout << "3.  Buscar consulta por ID" << endl;
        cout << "4.  Ver ultima consulta de paciente" << endl;
        cout << "5.  Generar reporte de historial" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 5);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- HISTORIAL COMPLETO ---" << endl;
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                OperacionesHistorial::mostrarHistorialMedico(idPaciente);
                break;
            }
            
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
                
            default:
                cout << "Opcion en desarrollo..." << endl;
                break;
        }
        
        if (opcion != 0) {
            Formatos::pausarPantalla();
        }
        
    } while (opcion != 0);
}


void Menus::menuMantenimientoSistema() {
    int opcion;
    
    do {
        Formatos::limpiarPantalla();
        mostrarEncabezado("MANTENIMIENTO DEL SISTEMA");
        
        cout << "1.  Verificar integridad de archivos" << endl;
        cout << "2.  Reconstruir archivos danados" << endl;
        cout << "3.  Respaldar datos" << endl;
        cout << "4.  Restaurar respaldo" << endl;
        cout << "5.  Limpiar registros eliminados" << endl;
        cout << "6.  Mostrar informacion de archivos" << endl;
        cout << "7.  Reindexar archivos" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 7);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- VERIFICAR INTEGRIDAD DE ARCHIVOS ---" << endl;
                cout << "Verificando todos los archivos del sistema..." << endl;
                
                GestorArchivos::verificarIntegridad(RUTA_PACIENTES);
                GestorArchivos::verificarIntegridad(RUTA_DOCTORES);
                GestorArchivos::verificarIntegridad(RUTA_CITAS);
                GestorArchivos::verificarIntegridad(RUTA_HOSPITAL);
                
                cout << "\nVerificacion completada." << endl;
                break;
            }
            
            case 2: {
                cout << "\n--- RECONSTRUIR ARCHIVOS DANADOS ---" << endl;
                cout << "Reconstruyendo todos los archivos del sistema..." << endl;
                
                GestorArchivos::reconstruirArchivo(RUTA_PACIENTES);
                GestorArchivos::reconstruirArchivo(RUTA_DOCTORES);
                GestorArchivos::reconstruirArchivo(RUTA_CITAS);
                GestorArchivos::reconstruirArchivo(RUTA_HOSPITAL);
                
                cout << "Reconstruccion completada." << endl;
                break;
            }
            
            case 3: {
                cout << "\n--- RESPALDAR DATOS ---" << endl;
                cout << "Creando respaldo completo del sistema..." << endl;
                
                if (GestorArchivos::realizarRespaldoCompleto()) {
                    cout << "Respaldo completado exitosamente." << endl;
                } else {
                    cout << "Error en el proceso de respaldo." << endl;
                }
                break;
            }
            
            case 4: {
                cout << "\n--- RESTAURAR RESPALDO ---" << endl;
                cout << "Funcionalidad no disponible en este menu." << endl;
                cout << "Contacte al administrador del sistema." << endl;
                break;
            }
            
            case 5: {
                cout << "\n--- LIMPIAR REGISTROS ELIMINADOS ---" << endl;
                cout << "Limpiando registros eliminados de todos los archivos..." << endl;
                
                GestorArchivos::limpiarRegistrosEliminados(RUTA_PACIENTES);
                GestorArchivos::limpiarRegistrosEliminados(RUTA_DOCTORES);
                GestorArchivos::limpiarRegistrosEliminados(RUTA_CITAS);
                
                cout << "Limpieza completada." << endl;
                break;
            }
            
            case 6: {
                cout << "\n--- INFORMACION DE ARCHIVOS ---" << endl;
                GestorArchivos::mostrarInformacionArchivos();
                break;
            }
            
            case 7: {
                cout << "\n--- REINDEXAR ARCHIVOS ---" << endl;
                cout << "Reindexando todos los archivos del sistema..." << endl;
                
                GestorArchivos::reindexarArchivo(RUTA_PACIENTES);
                GestorArchivos::reindexarArchivo(RUTA_DOCTORES);
                GestorArchivos::reindexarArchivo(RUTA_CITAS);
                
                cout << "Reindexacion completada." << endl;
                break;
            }
            
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }
        
        if (opcion != 0) {
            Formatos::pausarPantalla();
        }
        
    } while (opcion != 0);
}
// ==================== FUNCIONES PRIVADAS ====================

void Menus::mostrarEncabezado(const string& titulo) {
    cout << "\n===============================================" << endl;
    cout << "           " << titulo << endl;
    cout << "===============================================" << endl;
}

int Menus::leerOpcionMenu(const string& mensaje, int min, int max) {
    return EntradaUsuario::leerEntero(mensaje, min, max);
}
