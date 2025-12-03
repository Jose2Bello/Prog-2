#include "../Menus/Menu.hpp"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include "../Utilidades/Utilidades.hpp"
#include "../persistencia/GestorArchivos.hpp"
#include "../persistencia/Constantes.hpp"
#include "../Historial/operacionesHistorial.hpp"
#include "../Historial/HistorialMedico.hpp"

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
        
        cout << "1. Registrar nuevo paciente" << endl;
        cout << "2. Buscar paciente por ID" << endl;
        cout << "3. Buscar paciente por cedula" << endl;
        cout << "4. Buscar pacientes por nombre" << endl;
        cout << "5. Listar todos los pacientes" << endl;
        cout << "6. Eliminar paciente" << endl;
        cout << "7. Restaurar paciente eliminado" << endl;
        cout << "8. Listar pacientes eliminados" << endl;
        cout << "0. Volver al menu principal" << endl;
        cout << "=================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 8);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- REGISTRAR NUEVO PACIENTE ---" << endl;
                
                // Datos obligatorios
                string nombre = EntradaUsuario::leerTexto("Nombre", 49);
                string apellido = EntradaUsuario::leerTexto("Apellido", 49);
                string cedula = EntradaUsuario::leerTexto("Cedula", 19);
                int edad = EntradaUsuario::leerEntero("Edad", 0, 150);
                
                // Validar sexo
                char sexo;
                do {
                    cout << "Sexo (M/F): ";
                    cin >> sexo;
                    cin.ignore();
                    sexo = toupper(sexo);
                    if (sexo != 'M' && sexo != 'F') {
                        cout << "Error: Debe ser M o F" << endl;
                    }
                } while (sexo != 'M' && sexo != 'F');
                
                // Crear y agregar paciente
                Paciente nuevoPaciente(nombre.c_str(), apellido.c_str(), 
                                     cedula.c_str(), edad, sexo);
                
                if (OperacionesPacientes::agregarPaciente(nuevoPaciente)) {
                    cout << "\nPaciente registrado exitosamente!" << endl;
                    cout << "ID asignado: " << nuevoPaciente.getId() << endl;
                } else {
                    cout << "\nError al registrar paciente." << endl;
                }
                break;
            }
            
            case 2: {
                cout << "\n--- BUSCAR PACIENTE POR ID ---" << endl;
                int id = EntradaUsuario::leerEntero("ID del paciente", 1, 9999);
                
                Paciente paciente = OperacionesPacientes::buscarPorID(id);
                if (paciente.getId() != -1) {
                    cout << "\n=== PACIENTE ENCONTRADO ===" << endl;
                    cout << "ID: " << paciente.getId() << endl;
                    cout << "Nombre: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                    cout << "Cedula: " << paciente.getCedula() << endl;
                    cout << "Edad: " << paciente.getEdad() << endl;
                    cout << "Sexo: " << paciente.getSexo() << endl;
                    cout << "Estado: " << (paciente.isEliminado() ? "ELIMINADO" : "ACTIVO") << endl;
                } else {
                    cout << "Paciente no encontrado." << endl;
                }
                break;
            }
            
            case 3: {
                cout << "\n--- BUSCAR PACIENTE POR CEDULA ---" << endl;
                string cedula = EntradaUsuario::leerTexto("Cedula", 19);
                
                Paciente paciente = OperacionesPacientes::buscarPorCedula(cedula.c_str());
                if (paciente.getId() != -1) {
                    cout << "\n=== PACIENTE ENCONTRADO ===" << endl;
                    cout << "ID: " << paciente.getId() << endl;
                    cout << "Nombre: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                    cout << "Cedula: " << paciente.getCedula() << endl;
                    cout << "Edad: " << paciente.getEdad() << endl;
                } else {
                    cout << "Paciente no encontrado." << endl;
                }
                break;
            }
            
            case 4: {
                cout << "\n--- BUSCAR PACIENTES POR NOMBRE ---" << endl;
                string nombreBusqueda = EntradaUsuario::leerTexto("Ingrese nombre o parte del nombre", 49);
                
                const int MAX_RESULTADOS = 50;
                Paciente resultados[MAX_RESULTADOS];
                
                int cantidad = OperacionesPacientes::buscarPorNombre(nombreBusqueda.c_str(), resultados, MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    cout << "\n=== RESULTADOS DE BUSQUEDA (" << cantidad << " encontrados) ===" << endl;
                    cout << "==================================================" << endl;
                    
                    for (int i = 0; i < cantidad; i++) {
                        Paciente& p = resultados[i];
                        
                        cout << "\n--- Paciente " << (i + 1) << " ---" << endl;
                        cout << "ID: " << p.getId() << endl;
                        cout << "Nombre: " << p.getNombre() << " " << p.getApellido() << endl;
                        cout << "Cédula: " << p.getCedula() << endl;
                        cout << "Edad: " << p.getEdad() << " años" << endl;
                        cout << "Sexo: " << p.getSexo() << endl;
                        
                        // Mostrar datos adicionales si existen
                        if (strlen(p.getTelefono()) > 0) {
                            cout << "Teléfono: " << p.getTelefono() << endl;
                        }
                    }
                    
                    cout << "\n==================================================" << endl;
                    
                    // Opción para ver detalles específicos
                    if (cantidad == 1) {
                        cout << "\n¿Ver información completa? (s/n): ";
                        char respuesta;
                        cin >> respuesta;
                        cin.ignore();
                        
                        if (tolower(respuesta) == 's') {
                            resultados[0].mostrarInfo(); // Asumiendo que existe este método
                        }
                    }
                } else {
                    cout << "No se encontraron pacientes con ese nombre." << endl;
                }
                break;
            }
            
            case 5: {
                cout << "\n--- LISTA DE PACIENTES ---" << endl;
                OperacionesPacientes::listarPacientes();
                break;
            }
            
            case 6: {
                cout << "\n--- ELIMINAR PACIENTE ---" << endl;
                int id = EntradaUsuario::leerEntero("ID del paciente a eliminar", 1, 9999);
                
                // Primero mostrar info
                Paciente paciente = OperacionesPacientes::buscarPorID(id);
                if (paciente.getId() == -1) {
                    cout << "Paciente no encontrado." << endl;
                    break;
                }
                
                cout << "\nDatos del paciente:" << endl;
                cout << "ID: " << paciente.getId() << endl;
                cout << "Nombre: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                cout << "Cedula: " << paciente.getCedula() << endl;
                
                // Confirmar eliminación
                cout << "\n¿Está seguro de eliminar este paciente? (s/n): ";
                char respuesta;
                cin >> respuesta;
                cin.ignore();
                
                if (tolower(respuesta) == 's') {
                    if (OperacionesPacientes::eliminarPaciente(id, false)) {
                        cout << "Paciente eliminado exitosamente." << endl;
                    } else {
                        cout << "Error al eliminar paciente." << endl;
                    }
                } else {
                    cout << "Eliminación cancelada." << endl;
                }
                break;
            }
            
            case 7: {
                cout << "\n--- RESTAURAR PACIENTE ELIMINADO ---" << endl;
                
                // Mostrar pacientes eliminados primero
                cout << "Pacientes eliminados:" << endl;
                OperacionesPacientes::listarPacientesEliminados();
                
                if (OperacionesPacientes::contarPacientesEliminados() > 0) {
                    int id = EntradaUsuario::leerEntero("ID del paciente a restaurar (0 para cancelar)", 0, 9999);
                    
                    if (id > 0) {
                        if (OperacionesPacientes::restaurarPaciente(id)) {
                            cout << "Paciente restaurado exitosamente." << endl;
                        } else {
                            cout << "Error al restaurar paciente." << endl;
                        }
                    } else {
                        cout << "Operación cancelada." << endl;
                    }
                }
                break;
            }
            
            case 8: {
                cout << "\n--- PACIENTES ELIMINADOS ---" << endl;
                OperacionesPacientes::listarPacientesEliminados();
                break;
            }
            
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }
        
        if (opcion != 0) {
            cout << "\nPresione Enter para continuar...";
            cin.ignore();
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
                
                // Datos obligatorios
                string nombre = EntradaUsuario::leerTexto("Nombre", 49);
                string apellido = EntradaUsuario::leerTexto("Apellido", 49);
                string cedula = EntradaUsuario::leerTexto("Cedula", 19);
                string especialidad = EntradaUsuario::leerTexto("Especialidad", 49);
                int experiencia = EntradaUsuario::leerEntero("Anios de experiencia", 0, 60);
                float costo = EntradaUsuario::leerFloat("Costo de consulta", 0, 10000);
                
                // Datos adicionales
                cout << "\n--- DATOS ADICIONALES ---" << endl;
                string telefono = EntradaUsuario::leerTexto("Telefono", 14);
                string email = EntradaUsuario::leerTexto("Email", 49);
                string horario = EntradaUsuario::leerTexto("Horario de atencion", 49);
                
                // Crear doctor
                Doctor nuevoDoctor(nombre.c_str(), apellido.c_str(), cedula.c_str(), 
                                   especialidad.c_str(), experiencia, costo);
                
                // Asignar datos adicionales
                if (!telefono.empty()) nuevoDoctor.setTelefono(telefono.c_str());
                if (!email.empty()) nuevoDoctor.setEmail(email.c_str());
                if (!horario.empty()) nuevoDoctor.setHorarioAtencion(horario.c_str());
                
                // Marcar como disponible por defecto
                nuevoDoctor.setDisponible(true);
                
                if (OperacionesDoctores::agregarDoctor(nuevoDoctor)) {
                    cout << "\n✅ DOCTOR REGISTRADO EXITOSAMENTE" << endl;
                    cout << "ID asignado: " << nuevoDoctor.getId() << endl;
                    cout << "Nombre: Dr. " << nuevoDoctor.getNombre() << " " << nuevoDoctor.getApellido() << endl;
                    cout << "Especialidad: " << nuevoDoctor.getEspecialidad() << endl;
                    cout << "Años experiencia: " << nuevoDoctor.getAniosExperiencia() << endl;
                    cout << "Costo consulta: $" << fixed << setprecision(2) << nuevoDoctor.getCostoConsulta() << endl;
                    
                    if (strlen(nuevoDoctor.getTelefono()) > 0)
                        cout << "Teléfono: " << nuevoDoctor.getTelefono() << endl;
                    if (strlen(nuevoDoctor.getEmail()) > 0)
                        cout << "Email: " << nuevoDoctor.getEmail() << endl;
                    if (strlen(nuevoDoctor.getHorarioAtencion()) > 0)
                        cout << "Horario: " << nuevoDoctor.getHorarioAtencion() << endl;
                } else {
                    cout << "\n Error al registrar doctor." << endl;
                }
                break;
            }
            
            case 2: {
                cout << "\n--- BUSCAR DOCTOR POR ID ---" << endl;
                int id = EntradaUsuario::leerEntero("Ingrese ID del doctor", 1, 10000);
                
                Doctor doctor = OperacionesDoctores::buscarPorID(id);
                if (doctor.getId() != -1) {
                    cout << "\n=== INFORMACION DEL DOCTOR ===" << endl;
                    doctor.mostrarInfo();
                } else {
                    cout << "Doctor no encontrado." << endl;
                }
                break;
            }
            
            case 3: {
                cout << "\n--- BUSCAR DOCTOR POR CEDULA ---" << endl;
                string cedula = EntradaUsuario::leerTexto("Ingrese cedula", 19);
                
                Doctor doctor = OperacionesDoctores::buscarPorCedula(cedula.c_str());
                if (doctor.getId() != -1) {
                    cout << "\n=== INFORMACION DEL DOCTOR ===" << endl;
                    doctor.mostrarInfo();
                } else {
                    cout << "Doctor no encontrado." << endl;
                }
                break;
            }
            
            case 4: {
                cout << "\n--- BUSCAR DOCTORES POR ESPECIALIDAD ---" << endl;
                string especialidad = EntradaUsuario::leerTexto("Ingrese especialidad", 49);
                
                const int MAX_RESULTADOS = 50;
                Doctor resultados[MAX_RESULTADOS];
                
                int cantidad = OperacionesDoctores::buscarPorEspecialidad(especialidad.c_str(), resultados, MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    cout << "\n=== RESULTADOS DE BUSQUEDA (" << cantidad << " encontrados) ===" << endl;
                    cout << "=====================================================" << endl;
                    
                    for (int i = 0; i < cantidad; i++) {
                        Doctor& d = resultados[i];
                        
                        cout << "\n--- Doctor " << (i + 1) << " ---" << endl;
                        cout << "ID: " << d.getId() << endl;
                        cout << "Nombre: Dr. " << d.getNombre() << " " << d.getApellido() << endl;
                        cout << "Especialidad: " << d.getEspecialidad() << endl;
                        cout << "Años experiencia: " << d.getAniosExperiencia() << endl;
                        cout << "Costo consulta: $" << fixed << setprecision(2) << d.getCostoConsulta() << endl;
                        cout << "Pacientes asignados: " << d.getCantidadPacientes() << endl;
                        cout << "Estado: " << (d.isDisponible() ? "DISPONIBLE" : "NO DISPONIBLE") << endl;
                    }
                    
                    cout << "\n=====================================================" << endl;
                } else {
                    cout << "No se encontraron doctores de esa especialidad." << endl;
                }
                break;
            }
            
            case 5: {
                cout << "\n--- ACTUALIZAR DATOS DE DOCTOR ---" << endl;
                int id = EntradaUsuario::leerEntero("ID del doctor a actualizar", 1, 10000);
                
                // Buscar el doctor
                Doctor doctor = OperacionesDoctores::buscarPorID(id);
                if (doctor.getId() == -1) {
                    cout << "Doctor no encontrado." << endl;
                    break;
                }
                
                // Mostrar datos actuales
                cout << "\n=== DATOS ACTUALES ===" << endl;
                cout << "ID: " << doctor.getId() << endl;
                cout << "Nombre: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                cout << "Cédula: " << doctor.getCedula() << endl;
                cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                cout << "Años experiencia: " << doctor.getAniosExperiencia() << endl;
                cout << "Costo consulta: $" << fixed << setprecision(2) << doctor.getCostoConsulta() << endl;
                
                if (strlen(doctor.getTelefono()) > 0)
                    cout << "Teléfono: " << doctor.getTelefono() << endl;
                if (strlen(doctor.getEmail()) > 0)
                    cout << "Email: " << doctor.getEmail() << endl;
                if (strlen(doctor.getHorarioAtencion()) > 0)
                    cout << "Horario: " << doctor.getHorarioAtencion() << endl;
                
                cout << "\n=== EDITAR CAMPOS ===" << endl;
                cout << "Deje vacío para mantener el valor actual\n" << endl;
                
                // Campos básicos
                string nuevoValor;
                
                nuevoValor = EntradaUsuario::leerTexto("Nuevo nombre", 49);
                if (!nuevoValor.empty()) doctor.setNombre(nuevoValor.c_str());
                
                nuevoValor = EntradaUsuario::leerTexto("Nuevo apellido", 49);
                if (!nuevoValor.empty()) doctor.setApellido(nuevoValor.c_str());
                
                // Cédula (con confirmación)
                cout << "\n ¿Cambiar cédula? (s/n): ";
                char cambiarCedula;
                cin >> cambiarCedula;
                cin.ignore();
                
                if (tolower(cambiarCedula) == 's') {
                    nuevoValor = EntradaUsuario::leerTexto("Nueva cédula", 19);
                    if (!nuevoValor.empty()) doctor.setCedula(nuevoValor.c_str());
                }
                
                // Especialidad
                nuevoValor = EntradaUsuario::leerTexto("Nueva especialidad", 49);
                if (!nuevoValor.empty()) doctor.setEspecialidad(nuevoValor.c_str());
                
                // Años experiencia
                string expStr = EntradaUsuario::leerTexto("Nuevos años de experiencia", 2);
                if (!expStr.empty()) {
                    try {
                        int nuevaExp = stoi(expStr);
                        if (nuevaExp >= 0 && nuevaExp <= 60) {
                            doctor.setAniosExperiencia(nuevaExp);
                        } else {
                            cout << "Años inválidos. Manteniendo valor actual." << endl;
                        }
                    } catch (...) {
                        cout << "Valor inválido. Manteniendo valor actual." << endl;
                    }
                }
                
                // Costo consulta
                string costoStr = EntradaUsuario::leerTexto("Nuevo costo de consulta", 10);
                if (!costoStr.empty()) {
                    try {
                        float nuevoCosto = stof(costoStr);
                        if (nuevoCosto >= 0 && nuevoCosto <= 10000) {
                            doctor.setCostoConsulta(nuevoCosto);
                        } else {
                            cout << " Costo inválido. Manteniendo valor actual." << endl;
                        }
                    } catch (...) {
                        cout << " Valor inválido. Manteniendo valor actual." << endl;
                    }
                }
                
                // Datos adicionales
                cout << "\n--- DATOS ADICIONALES ---" << endl;
                
                nuevoValor = EntradaUsuario::leerTexto("Nuevo teléfono", 14);
                if (!nuevoValor.empty()) doctor.setTelefono(nuevoValor.c_str());
                
                nuevoValor = EntradaUsuario::leerTexto("Nuevo email", 49);
                if (!nuevoValor.empty()) doctor.setEmail(nuevoValor.c_str());
                
                nuevoValor = EntradaUsuario::leerTexto("Nuevo horario", 49);
                if (!nuevoValor.empty()) doctor.setHorarioAtencion(nuevoValor.c_str());
                
                // Disponibilidad
                cout << "¿Cambiar disponibilidad? (s/n): ";
                char cambiarDisp;
                cin >> cambiarDisp;
                cin.ignore();
                
                if (tolower(cambiarDisp) == 's') {
                    cout << "Disponibilidad (1=Si, 0=No): ";
                    int disp;
                    cin >> disp;
                    cin.ignore();
                    doctor.setDisponible(disp == 1);
                }
                
                // Confirmar actualización
                cout << "\n=== CONFIRMAR ACTUALIZACIÓN ===" << endl;
                cout << "¿Desea guardar los cambios? (s/n): ";
                char confirmar;
                cin >> confirmar;
                cin.ignore();
                
                if (tolower(confirmar) == 's') {
                    if (OperacionesDoctores::actualizarDoctor(doctor)) {
                        cout << " Doctor actualizado exitosamente." << endl;
                    } else {
                        cout << " Error al actualizar doctor." << endl;
                    }
                } else {
                    cout << " Actualización cancelada por el usuario." << endl;
                }
                break;
            }
            
            case 6: {
                cout << "\n--- LISTA DE DOCTORES ---" << endl;
                OperacionesDoctores::listarDoctores();
                break;
            }
            
            case 7: {
                cout << "\n--- ELIMINAR DOCTOR ---" << endl;
                int id = EntradaUsuario::leerEntero("ID del doctor a eliminar", 1, 10000);
                
                // Buscar doctor para mostrar info
                Doctor doctor = OperacionesDoctores::buscarPorID(id);
                if (doctor.getId() == -1) {
                    cout << "Doctor no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== DATOS DEL DOCTOR ===" << endl;
                cout << "ID: " << doctor.getId() << endl;
                cout << "Nombre: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                cout << "Pacientes asignados: " << doctor.getCantidadPacientes() << endl;
                cout << "Citas programadas: " << doctor.getCantidadCitas() << endl;
                
                // Confirmación
                cout << "\n ¿Está seguro de eliminar este doctor? (s/n): ";
                char respuesta;
                cin >> respuesta;
                cin.ignore();
                
                if (tolower(respuesta) == 's') {
                    // Doble confirmación si tiene pacientes o citas
                    if (doctor.getCantidadPacientes() > 0 || doctor.getCantidadCitas() > 0) {
                        cout << "\n ADVERTENCIA: Este doctor tiene pacientes y/o citas asignadas." << endl;
                        cout << "¿Continuar con la eliminación? (s/n): ";
                        cin >> respuesta;
                        cin.ignore();
                        
                        if (tolower(respuesta) != 's') {
                            cout << "Eliminación cancelada." << endl;
                            break;
                        }
                    }
                    
                    if (OperacionesDoctores::eliminarDoctor(id, false)) {
                        cout << " Doctor eliminado exitosamente." << endl;
                    } else {
                        cout << " Error al eliminar doctor." << endl;
                    }
                } else {
                    cout << "Eliminación cancelada." << endl;
                }
                break;
            }
            
            case 8: {
                cout << "\n--- RESTAURAR DOCTOR ELIMINADO ---" << endl;
                
                // Mostrar doctores eliminados
                cout << "Doctores eliminados:" << endl;
                OperacionesDoctores::listarDoctoresEliminados();
                
                int id = EntradaUsuario::leerEntero("ID del doctor a restaurar (0 para cancelar)", 0, 10000);
                
                if (id > 0) {
                    if (OperacionesDoctores::restaurarDoctor(id)) {
                        cout << " Doctor restaurado exitosamente." << endl;
                    } else {
                        cout << " Error al restaurar doctor." << endl;
                    }
                } else {
                    cout << "Operación cancelada." << endl;
                }
                break;
            }
            
            case 9: {
                cout << "\n--- DOCTORES ELIMINADOS ---" << endl;
                OperacionesDoctores::listarDoctoresEliminados();
                break;
            }
            
            case 10: {
                cout << "\n--- ASIGNAR PACIENTE A DOCTOR ---" << endl;
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                
                // Verificar que el doctor existe
                Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
                if (doctor.getId() == -1) {
                    cout << " Doctor no encontrado." << endl;
                    break;
                }
                
                // Verificar que el paciente existe
                Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
                if (paciente.getId() == -1) {
                    cout << "Paciente no encontrado." << endl;
                    break;
                }
                
                // Verificar si ya está asignado
                if (doctor.tienePaciente(idPaciente)) {
                    cout << " Este paciente ya está asignado a este doctor." << endl;
                    break;
                }
                
                // Asignar
                if (doctor.agregarPacienteID(idPaciente)) {
                    // Actualizar el doctor en el archivo
                    if (OperacionesDoctores::actualizarDoctor(doctor)) {
                        cout << "Paciente asignado exitosamente." << endl;
                        cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                        cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                    } else {
                        cout << " Error al actualizar datos del doctor." << endl;
                    }
                } else {
                    cout << " Error: No se pudo asignar el paciente (límite alcanzado)." << endl;
                }
                break;
            }
            
            case 11: {
                cout << "\n--- VER PACIENTES DE UN DOCTOR ---" << endl;
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                
                // Buscar doctor
                Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
                if (doctor.getId() == -1) {
                    cout << "Doctor no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== DOCTOR ===" << endl;
                cout << "ID: " << doctor.getId() << endl;
                cout << "Nombre: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                cout << "Pacientes asignados: " << doctor.getCantidadPacientes() << endl;
                
                if (doctor.getCantidadPacientes() > 0) {
                    cout << "\n=== LISTA DE PACIENTES ===" << endl;
                    cout << "=================================" << endl;
                    
                    const int* pacientesIDs = doctor.getPacientesIDs();
                    for (int i = 0; i < doctor.getCantidadPacientes(); i++) {
                        Paciente paciente = OperacionesPacientes::buscarPorID(pacientesIDs[i]);
                        if (paciente.getId() != -1) {
                            cout << "\n--- Paciente " << (i + 1) << " ---" << endl;
                            cout << "ID: " << paciente.getId() << endl;
                            cout << "Nombre: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                            cout << "Cédula: " << paciente.getCedula() << endl;
                            cout << "Edad: " << paciente.getEdad() << endl;
                        }
                    }
                    
                    cout << "\n=================================" << endl;
                    cout << "Total: " << doctor.getCantidadPacientes() << " paciente(s)" << endl;
                } else {
                    cout << "\nEste doctor no tiene pacientes asignados." << endl;
                }
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
        cout << "9.  Listar todas las citas" << endl; 
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 9);  
        
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
                    cout << "\n CITA AGENDADA EXITOSAMENTE" << endl;
                } else {
                    cout << "\n Error al agendar cita." << endl;
                }
                break;
            }
            
            case 2: {
                cout << "\n--- CANCELAR CITA ---" << endl;
                int idCita = EntradaUsuario::leerEntero("ID de la cita a cancelar", 1, 10000);
                
                // Mostrar información de la cita primero
                Cita cita = OperacionesCitas::buscarPorID(idCita);
                if (cita.getId() == -1) {
                    cout << " Cita no encontrada." << endl;
                    break;
                }
                
                // Mostrar detalles de la cita
                cout << "\n=== DETALLES DE LA CITA ===" << endl;
                cita.mostrarInfo();
                
            
                cout << "\n⚠️  ¿Está seguro de cancelar esta cita? (s/n): ";
                char confirmar;
                cin >> confirmar;
                cin.ignore();
                
                if (tolower(confirmar) == 's') {
                    if (OperacionesCitas::cancelarCita(idCita)) {
                        cout << " CITA CANCELADA EXITOSAMENTE" << endl;
                    } else {
                        cout << " Error al cancelar cita." << endl;
                    }
                } else {
                    cout << "Cancelación abortada por el usuario." << endl;
                }
                break;
            }
            
            case 3: {
                cout << "\n--- ATENDER CITA ---" << endl;
                int idCita = EntradaUsuario::leerEntero("ID de la cita a atender", 1, 10000);
                
                // Mostrar información de la cita primero
                Cita cita = OperacionesCitas::buscarPorID(idCita);
                if (cita.getId() == -1) {
                    cout << " Cita no encontrada." << endl;
                    break;
                }
                
                // Verificar que la cita está agendada
                if (strcmp(cita.getEstado(), "AGENDADA") != 0) {
                    cout << " La cita no está en estado 'AGENDADA'. Estado actual: " 
                         << cita.getEstado() << endl;
                    break;
                }
                
                cout << "\n=== DETALLES DE LA CITA A ATENDER ===" << endl;
                cita.mostrarInfo();
                
                // Pedir detalles de la atención
                cout << "\n--- INGRESE DETALLES DE LA ATENCIÓN ---" << endl;
                string diagnostico = EntradaUsuario::leerTexto("Diagnóstico", 299);
                string tratamiento = EntradaUsuario::leerTexto("Tratamiento", 299);
                string medicamentos = EntradaUsuario::leerTexto("Medicamentos recetados", 199);
                
                if (OperacionesCitas::atenderCita(idCita, diagnostico.c_str(), 
                                                tratamiento.c_str(), medicamentos.c_str())) {
                    cout << "\n CITA ATENDIDA EXITOSAMENTE" << endl;
                    cout << "La información se ha registrado." << endl;
                } else {
                    cout << "\n❌ Error al atender cita." << endl;
                }
                break;
            }
            
            case 4: {
                cout << "\n--- CITAS DE UN PACIENTE ---" << endl;
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                
                // Verificar que el paciente existe
                Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
                if (paciente.getId() == -1) {
                    cout << " Paciente no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== CITAS DEL PACIENTE ===" << endl;
                cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                cout << "Cédula: " << paciente.getCedula() << endl;
                cout << "==============================================" << endl;
                
                const int MAX_RESULTADOS = 50;
                Cita resultados[MAX_RESULTADOS];
                
                int cantidad = OperacionesCitas::buscarCitasDePaciente(idPaciente, 
                                                                     resultados, 
                                                                     MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    for (int i = 0; i < cantidad; i++) {
                        cout << "\n--- Cita " << (i + 1) << " ---" << endl;
                        resultados[i].mostrarInfo();
                        
                        // Mostrar información del doctor
                        Doctor doctor = OperacionesDoctores::buscarPorID(resultados[i].getIdDoctor());
                        if (doctor.getId() != -1) {
                            cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                            cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                        }
                    }
                    cout << "\nTotal: " << cantidad << " cita(s)" << endl;
                }
                break;
            }
            
            case 5: {
                cout << "\n--- CITAS DE UN DOCTOR ---" << endl;
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                
                // Verificar que el doctor existe
                Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
                if (doctor.getId() == -1) {
                    cout << "Doctor no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== CITAS DEL DOCTOR ===" << endl;
                cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                cout << "==============================================" << endl;
                
                const int MAX_RESULTADOS = 50;
                Cita resultados[MAX_RESULTADOS];
                
                int cantidad = OperacionesCitas::buscarCitasDeDoctor(idDoctor, 
                                                                   resultados, 
                                                                   MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    for (int i = 0; i < cantidad; i++) {
                        cout << "\n--- Cita " << (i + 1) << " ---" << endl;
                        resultados[i].mostrarInfo();
                        
                        // Mostrar información del paciente
                        Paciente paciente = OperacionesPacientes::buscarPorID(resultados[i].getIdPaciente());
                        if (paciente.getId() != -1) {
                            cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                            cout << "Cédula: " << paciente.getCedula() << endl;
                        }
                    }
                    cout << "\nTotal: " << cantidad << " cita(s)" << endl;
                }
                break;
            }
            
            case 6: {
                cout << "\n--- CITAS POR FECHA ---" << endl;
                string fecha = EntradaUsuario::leerTexto("Fecha (YYYY-MM-DD)", 10);
                
                // Validar formato de fecha
                if (!Cita::validarFecha(fecha.c_str())) {
                    cout << "Formato de fecha inválido. Use YYYY-MM-DD" << endl;
                    break;
                }
                
                cout << "\n=== CITAS PARA LA FECHA " << fecha << " ===" << endl;
                cout << "==============================================" << endl;
                
                const int MAX_RESULTADOS = 50;
                Cita resultados[MAX_RESULTADOS];
                
                int cantidad = OperacionesCitas::buscarCitasPorFecha(fecha.c_str(), 
                                                                   resultados, 
                                                                   MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    for (int i = 0; i < cantidad; i++) {
                        cout << "\n--- Cita " << (i + 1) << " ---" << endl;
                        resultados[i].mostrarInfo();
                        
                        // Mostrar información del paciente
                        Paciente paciente = OperacionesPacientes::buscarPorID(resultados[i].getIdPaciente());
                        Doctor doctor = OperacionesDoctores::buscarPorID(resultados[i].getIdDoctor());
                        
                        if (paciente.getId() != -1) {
                            cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                        }
                        if (doctor.getId() != -1) {
                            cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                        }
                    }
                    cout << "\nTotal: " << cantidad << " cita(s)" << endl;
                }
                break;
            }
            
            case 7: {
                cout << "\n--- CITAS PENDIENTES ---" << endl;
                OperacionesCitas::listarCitasPendientes();
                break;
            }
            
            case 8: {
                cout << "\n--- VERIFICAR DISPONIBILIDAD DE DOCTOR ---" << endl;
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                string fecha = EntradaUsuario::leerTexto("Fecha (YYYY-MM-DD)", 10);
                string hora = EntradaUsuario::leerTexto("Hora (HH:MM)", 5);
                
                // Validar formatos
                if (!Cita::validarFecha(fecha.c_str())) {
                    cout << " Formato de fecha inválido. Use YYYY-MM-DD" << endl;
                    break;
                }
                if (!Cita::validarHora(hora.c_str())) {
                    cout << " Formato de hora inválido. Use HH:MM" << endl;
                    break;
                }
                
                if (OperacionesCitas::verificarDisponibilidad(idDoctor, fecha.c_str(), hora.c_str())) {
                    cout << "\n EL DOCTOR ESTÁ DISPONIBLE" << endl;
                    cout << "Puede proceder a agendar la cita." << endl;
                } else {
                    cout << "\n EL DOCTOR NO ESTÁ DISPONIBLE" << endl;
                break;
            }
        }
            
            case 9: {
                cout << "\n--- TODAS LAS CITAS ---" << endl;
                
                cout << "¿Mostrar citas canceladas también? (s/n): ";
                char mostrarCanceladas;
                cin >> mostrarCanceladas;
                cin.ignore();
                
                bool mostrar = (tolower(mostrarCanceladas) == 's');
                OperacionesCitas::listarTodasCitas(mostrar);
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

string obtenerFechaActual() {
    time_t ahora = time(nullptr);
    struct tm* tiempo = localtime(&ahora);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", tiempo);
    return string(buffer);
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
        cout << "6.  Ver consultas por doctor" << endl;
        cout << "7.  Ver consultas por fecha" << endl;
        cout << "8.  Ver estadisticas generales" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;
        
        opcion = leerOpcionMenu("Seleccione una opcion", 0, 8);
        
        switch (opcion) {
            case 1: {
                cout << "\n--- HISTORIAL COMPLETO DE PACIENTE ---" << endl;
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                
                // Verificar que el paciente existe
                Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
                if (paciente.getId() == -1) {
                    cout << "Error: Paciente no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== HISTORIAL MEDICO ===" << endl;
                cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                cout << "Cedula: " << paciente.getCedula() << endl;
                cout << "Edad: " << paciente.getEdad() << " anos" << endl;
                
                // Mostrar datos medicos importantes
                if (strlen(paciente.getTipoSangre()) > 0) {
                    cout << "Tipo de sangre: " << paciente.getTipoSangre() << endl;
                }
                if (strlen(paciente.getAlergias()) > 0) {
                    cout << "Alergias: " << paciente.getAlergias() << endl;
                }
                cout << "=================================================" << endl;
                
                OperacionesHistorial::mostrarHistorialMedico(idPaciente);
                break;
            }
            
            case 2: {
                cout << "\n--- AGREGAR CONSULTA AL HISTORIAL ---" << endl;
                
                // Datos basicos
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                string fecha = EntradaUsuario::leerTexto("Fecha de la consulta (YYYY-MM-DD)", 10);
                string hora = EntradaUsuario::leerTexto("Hora de la consulta (HH:MM)", 5);
                
                // Verificar que el paciente existe
                Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
                if (paciente.getId() == -1) {
                    cout << "Error: Paciente no encontrado." << endl;
                    break;
                }
                
                // Verificar que el doctor existe
                Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
                if (doctor.getId() == -1) {
                    cout << "Error: Doctor no encontrado." << endl;
                    break;
                }
                
                // Validar fecha y hora
                if (!HistorialMedico::validarFecha(fecha.c_str())) {
                    cout << "Error: Formato de fecha invalido. Use YYYY-MM-DD" << endl;
                    break;
                }
                if (!HistorialMedico::validarHora(hora.c_str())) {
                    cout << "Error: Formato de hora invalido. Use HH:MM" << endl;
                    break;
                }
                
                // Datos medicos
                cout << "\n--- DATOS DE LA CONSULTA ---" << endl;
                string diagnostico = EntradaUsuario::leerTexto("Diagnostico", 199);
                string tratamiento = EntradaUsuario::leerTexto("Tratamiento prescrito", 199);
                string medicamentos = EntradaUsuario::leerTexto("Medicamentos recetados", 149);
                float costo = EntradaUsuario::leerFloat("Costo de la consulta", 0, 10000);
                
                // Crear la consulta
                HistorialMedico nuevaConsulta(idPaciente, idDoctor, fecha.c_str(), hora.c_str());
                nuevaConsulta.setDiagnostico(diagnostico.c_str());
                nuevaConsulta.setTratamiento(tratamiento.c_str());
                nuevaConsulta.setMedicamentos(medicamentos.c_str());
                nuevaConsulta.setCosto(costo);
                
              
            
               if (OperacionesHistorial::agregarConsultaAlHistorial(idPaciente, nuevaConsulta))  {
                    cout << "\nCONSULTA AGREGADA AL HISTORIAL EXITOSAMENTE" << endl;
                    cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                    cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                    cout << "Fecha: " << fecha << " a las " << hora << endl;
                    cout << "Costo: $" << fixed << setprecision(2) << costo << endl;
                } else {
                    cout << "\nError al agregar consulta al historial." << endl;
                }
                break;
            }
            
            case 3: {
                cout << "\n--- BUSCAR CONSULTA POR ID ---" << endl;
                int idConsulta = EntradaUsuario::leerEntero("ID de la consulta", 1, 10000);
                
                HistorialMedico consulta = OperacionesHistorial::buscarPorID(idConsulta);
                if (consulta.getId() != -1) {
                    cout << "\n=== INFORMACION DE LA CONSULTA ===" << endl;
                    
                    // Mostrar informacion basica
                    cout << "ID Consulta: " << consulta.getId() << endl;
                    cout << "Fecha: " << consulta.getFecha() << " " << consulta.getHora() << endl;
                    
                    // Mostrar informacion del paciente
                    Paciente paciente = OperacionesPacientes::buscarPorID(consulta.getPacienteID());
                    if (paciente.getId() != -1) {
                        cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                        cout << "Cedula: " << paciente.getCedula() << endl;
                    }
                    
                    // Mostrar informacion del doctor
                    Doctor doctor = OperacionesDoctores::buscarPorID(consulta.getIdDoctor());
                    if (doctor.getId() != -1) {
                        cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                        cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                    }
                    
                    cout << "\n--- DATOS MEDICOS ---" << endl;
                    cout << "Diagnostico: " << consulta.getDiagnostico() << endl;
                    
                    if (strlen(consulta.getTratamiento()) > 0) {
                        cout << "Tratamiento: " << consulta.getTratamiento() << endl;
                    }
                    
                    if (strlen(consulta.getMedicamentos()) > 0) {
                        cout << "Medicamentos: " << consulta.getMedicamentos() << endl;
                    }
                    
                    cout << "Costo: $" << fixed << setprecision(2) << consulta.getCosto() << endl;
                } else {
                    cout << "Consulta no encontrada." << endl;
                }
                break;
            }
            
            case 4: {
                cout << "\n--- ULTIMA CONSULTA DE PACIENTE ---" << endl;
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                
                // Verificar que el paciente existe
                Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
                if (paciente.getId() == -1) {
                    cout << "Error: Paciente no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== ULTIMA CONSULTA MEDICA ===" << endl;
                cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                cout << "=================================================" << endl;
                
                HistorialMedico ultimaConsulta = OperacionesHistorial::buscarUltimaConsulta(idPaciente);
                if (ultimaConsulta.getId() != -1) {
                    // Mostrar informacion del doctor
                    Doctor doctor = OperacionesDoctores::buscarPorID(ultimaConsulta.getIdDoctor());
                    if (doctor.getId() != -1) {
                        cout << "Atendido por: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                        cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                    }
                    
                    cout << "\nFecha de consulta: " << ultimaConsulta.getFecha() << " a las " << ultimaConsulta.getHora() << endl;
                    cout << "Diagnostico: " << ultimaConsulta.getDiagnostico() << endl;
                    
                    if (strlen(ultimaConsulta.getTratamiento()) > 0) {
                        cout << "Tratamiento: " << ultimaConsulta.getTratamiento() << endl;
                    }
                    
                    if (strlen(ultimaConsulta.getMedicamentos()) > 0) {
                        cout << "Medicamentos: " << ultimaConsulta.getMedicamentos() << endl;
                    }
                    
                    cout << "Costo: $" << fixed << setprecision(2) << ultimaConsulta.getCosto() << endl;
                    
                    // Mostrar tiempo transcurrido desde la ultima consulta
                    time_t ahora = time(nullptr);
                    double dias = difftime(ahora, ultimaConsulta.getFechaRegistro()) / (60 * 60 * 24);
                    cout << "\nTiempo desde la ultima consulta: " << (int)dias << " dias" << endl;
                } else {
                    cout << "El paciente no tiene consultas registradas en el historial." << endl;
                }
                break;
            }
            
            case 5: {
                cout << "\n--- GENERAR REPORTE DE HISTORIAL ---" << endl;
                int idPaciente = EntradaUsuario::leerEntero("ID del paciente", 1, 10000);
                
                // Verificar que el paciente existe
                Paciente paciente = OperacionesPacientes::buscarPorID(idPaciente);
                if (paciente.getId() == -1) {
                    cout << "Error: Paciente no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== REPORTE DE HISTORIAL MEDICO ===" << endl;
                cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
                cout << "Cedula: " << paciente.getCedula() << endl;
                cout << "Fecha de generacion: " << obtenerFechaActual() << endl;
                cout << "=================================================" << endl;
                
                // Generar estadisticas
                int totalConsultas = OperacionesHistorial::contarConsultasDePaciente(idPaciente);
                float totalGastado = 0.0f;
                
                // Calcular total gastado
                const int MAX_CONSULTAS = 100;
                HistorialMedico historial[MAX_CONSULTAS];
                int cantidad = OperacionesHistorial::obtenerHistorialCompleto(idPaciente, historial, MAX_CONSULTAS);
                
                for (int i = 0; i < cantidad; i++) {
                    totalGastado += historial[i].getCosto();
                }
                
                cout << "\n=== ESTADISTICAS ===" << endl;
                cout << "Total de consultas: " << totalConsultas << endl;
                cout << "Total gastado en consultas: $" << fixed << setprecision(2) << totalGastado << endl;
                
                if (totalConsultas > 0) {
                    cout << "Promedio por consulta: $" << fixed << setprecision(2) << (totalGastado / totalConsultas) << endl;
                    
                    // Mostrar historial detallado
                    cout << "\n=== HISTORIAL DETALLADO ===" << endl;
                    OperacionesHistorial::mostrarHistorialMedico(idPaciente);
                    
                    // Opcion para exportar a archivo
                    cout << "\nDesea exportar este reporte a un archivo? (s/n): ";
                    char exportar;
                    cin >> exportar;
                    cin.ignore();
                    
                    if (tolower(exportar) == 's') {
                        string nombreArchivo = "reporte_" + string(paciente.getCedula()) + "_" + obtenerFechaActual() + ".txt";
                        cout << "Funcionalidad de exportacion en desarrollo." << endl;
                        cout << "Archivo sugerido: " << nombreArchivo << endl;
                    }
                }
                break;
            }
            
            case 6: {
                cout << "\n--- CONSULTAS POR DOCTOR ---" << endl;
                int idDoctor = EntradaUsuario::leerEntero("ID del doctor", 1, 10000);
                
                // Verificar que el doctor existe
                Doctor doctor = OperacionesDoctores::buscarPorID(idDoctor);
                if (doctor.getId() == -1) {
                    cout << "Error: Doctor no encontrado." << endl;
                    break;
                }
                
                cout << "\n=== CONSULTAS ATENDIDAS POR DOCTOR ===" << endl;
                cout << "Doctor: Dr. " << doctor.getNombre() << " " << doctor.getApellido() << endl;
                cout << "Especialidad: " << doctor.getEspecialidad() << endl;
                cout << "=================================================" << endl;
                
                OperacionesHistorial::listarConsultasPorDoctor(idDoctor);
                break;
            }
            
            case 7: {
                cout << "\n--- CONSULTAS POR FECHA ---" << endl;
                string fecha = EntradaUsuario::leerTexto("Fecha (YYYY-MM-DD) o 'hoy' para hoy", 10);
                
                if (fecha == "hoy") {
                    // Obtener fecha actual
                    time_t ahora = time(nullptr);
                    struct tm* tiempo = localtime(&ahora);
                    char fechaActual[11];
                    strftime(fechaActual, sizeof(fechaActual), "%Y-%m-%d", tiempo);
                    fecha = fechaActual;
                }
                
                // Validar formato de fecha
                if (!HistorialMedico::validarFecha(fecha.c_str())) {
                    cout << "Error: Formato de fecha invalido. Use YYYY-MM-DD" << endl;
                    break;
                }
                
                cout << "\n=== CONSULTAS DEL DIA " << fecha << " ===" << endl;
                OperacionesHistorial::listarConsultasPorFecha(fecha.c_str());
                break;
            }
            
            case 8: {
                cout << "\n--- ESTADISTICAS GENERALES ---" << endl;
                
                int totalConsultas = OperacionesHistorial::contarConsultas();
                float totalIngresos = OperacionesHistorial::calcularTotalIngresos();
                
                cout << "\n=== ESTADISTICAS DEL SISTEMA ===" << endl;
                cout << "Total de consultas registradas: " << totalConsultas << endl;
                cout << "Total de ingresos por consultas: $" << fixed << setprecision(2) << totalIngresos << endl;
                
                if (totalConsultas > 0) {
                    cout << "Promedio por consulta: $" << fixed << setprecision(2) << (totalIngresos / totalConsultas) << endl;
                }
                
                // Estadisticas por doctor (simplificado)
                cout << "\n--- CONSULTAS POR DOCTOR (Top 5) ---" << endl;
                // Esta funcion podria implementarse mas adelante
                cout << "Funcionalidad en desarrollo." << endl;
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
