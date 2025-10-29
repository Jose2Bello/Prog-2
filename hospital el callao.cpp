#include <iostream>
#include <locale>
#include <iomanip>
#include <cstring>
#include <ctime>

using namespace std;

struct Hospital {
    char nombre[100];
    char direccion[150];
    char telefono[15];
    
    Paciente* pacientes;
    int cantidadPacientes;
    int capacidadPacientes;
    
    Doctor* doctores;
    int cantidadDoctores;
    int capacidadDoctores;
    
    Cita* citas;
    int cantidadCitas;
    int capacidadCitas;
    
    int siguienteIdPaciente;
    int siguienteIdDoctor;
    int siguienteIdCita;
    int siguienteIdConsulta;
};

struct Paciente {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];
    int edad;
    char sexo;
    char tipoSangre[5];
    char telefono[15];
    char direccion[100];
    char email[50];
    
    HistorialMedico* historial;
    int cantidadConsultas;
    int capacidadHistorial;
    
    int* citasAgendadas;
    int cantidadCitas;
    int capacidadCitas;
    
    char alergias[500];
    char observaciones[500];
    
    bool activo;
};

struct HistorialMedico {
    int idConsulta;
    char fecha[11];
    char hora[6];
    char diagnostico[200];
    char tratamiento[200];
    char medicamentos[150];
    int idDoctor;
    float costo;
};

struct Doctor {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];
    char especialidad[50];
    int aniosExperiencia;
    float costoConsulta;
    char horarioAtencion[50];
    char telefono[15];
    char email[50];
    
    int* pacientesAsignados;
    int cantidadPacientes;
    int capacidadPacientes;
    
    int* citasAgendadas;
    int cantidadCitas;
    int capacidadCitas;
    
    bool disponible;
};

struct Cita {
    int id;
    int idPaciente;
    int idDoctor;
    char fecha[11];
    char hora[6];
    char motivo[150];
    char estado[20];
    char observaciones[200];
    bool atendida;
};
void liberarHospital(Hospital* hospital) {
    // Liberar memoria de cada paciente
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        delete[] hospital->pacientes[i].historial;
        delete[] hospital->pacientes[i].citasAgendadas;
    }
    
    // Liberar memoria de cada doctor
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        delete[] hospital->doctores[i].pacientesAsignados;
        delete[] hospital->doctores[i].citasAgendadas;
    }
    
    // Liberar arrays principales
    delete[] hospital->pacientes;
    delete[] hospital->doctores;
    delete[] hospital->citas;
    
    // Liberar el hospital
    delete hospital;
    
    cout << "Memoria del hospital liberada correctamente" << endl;
}

Paciente* buscarPacientePorCedula(Hospital* hospital, const char* cedula) {
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        const char* cedulaPaciente = hospital->pacientes[i].cedula;
    
        if (strcmp(cedulaPaciente, cedula) == 0) {
            return &hospital->pacientes[i];
        }
    }
    
    return nullptr;  
}

Paciente* buscarPacientePorId(Hospital* hospital, int id) {
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        if (hospital->pacientes[i].id == id) {
            return &hospital->pacientes[i];
        }
    }
    
    return nullptr; 
}
Paciente** buscarPacientesPorNombre(Hospital* hospital,  const char* nombre, int* cantidad) {
    Paciente** resultados = new Paciente*[hospital->cantidadPacientes];
    *cantidad = 0;
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        if (strstr(hospital->pacientes[i].nombre, nombre) != nullptr) {
            resultados[*cantidad] = &hospital->pacientes[i];
            (*cantidad)++;
        }
    }
    
    return resultados; 
}

Paciente** buscarPacientesPorNombre(Hospital* hospital, const char* nombre, int* cantidad) {
    if (!hospital || !nombre || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
    
    Paciente** resultados = new Paciente*[hospital->cantidadPacientes];
    *cantidad = 0;
    
   
    char* nombreLower = new char[strlen(nombre) + 1];
    for (int i = 0; nombre[i]; i++) {
        nombreLower[i] = tolower(nombre[i]);
    }
    nombreLower[strlen(nombre)] = '\0';
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        char* nombrePacienteLower = new char[strlen(hospital->pacientes[i].nombre) + 1];
        for (int j = 0; hospital->pacientes[i].nombre[j]; j++) {
            nombrePacienteLower[j] = tolower(hospital->pacientes[i].nombre[j]);
        }
        nombrePacienteLower[strlen(hospital->pacientes[i].nombre)] = '\0';
        if (strstr(nombrePacienteLower, nombreLower) != nullptr) {
            resultados[*cantidad] = &hospital->pacientes[i];
            (*cantidad)++;
        }
        
        delete[] nombrePacienteLower;
    }
    
    delete[] nombreLower;
    
    if (*cantidad == 0) {
        delete[] resultados;
        return nullptr;
    }
    
    return resultados;
}

void liberarResultadosBusqueda(Paciente** resultados) {
    if (resultados) {
        delete[] resultados;
    }
}
bool actualizarPaciente(Hospital* hospital, int id) {
    // Buscar el paciente por ID
    Paciente* paciente = buscarPacientePorId(hospital, id);
    
    if (!paciente) {
        cout << "Error: No se encontró paciente con ID " << id << endl;
        return false;
    }
    
    cout << "\n=== ACTUALIZAR DATOS DEL PACIENTE ===" << endl;
    cout << "Paciente: " << paciente->nombre << " " << paciente->apellido << endl;
    cout << "ID: " << paciente->id << endl;
    cout << "Deje en blanco para mantener el valor actual\n" << endl;
    
    char buffer[200];
    
    
    cout << "Nombre actual: " << paciente->nombre << endl;
    cout << "Nuevo nombre: ";
    cin.ignore();
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        // Validar que el nombre no esté vacío después de trim
        bool soloEspacios = true;
        for (int i = 0; buffer[i]; i++) {
            if (!isspace(buffer[i])) {
                soloEspacios = false;
                break;
            }
        }
        if (!soloEspacios) {
            strcpy(paciente->nombre, buffer);
        }
    }
    
    
    cout << "Apellido actual: " << paciente->apellido << endl;
    cout << "Nuevo apellido: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        bool soloEspacios = true;
        for (int i = 0; buffer[i]; i++) {
            if (!isspace(buffer[i])) {
                soloEspacios = false;
                break;
            }
        }
        if (!soloEspacios) {
            strcpy(paciente->apellido, buffer);
        }
    }
    

    cout << "Edad actual: " << paciente->edad << endl;
    cout << "Nueva edad: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        int nuevaEdad = atoi(buffer);
        if (nuevaEdad > 0 && nuevaEdad <= 150) {
            paciente->edad = nuevaEdad;
        } else {
            cout << "Edad no válida. Manteniendo valor actual." << endl;
        }
    }
    

    cout << "Sexo actual: " << paciente->sexo << endl;
    cout << "Nuevo sexo (M/F): ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        char nuevoSexo = toupper(buffer[0]);
        if (nuevoSexo == 'M' || nuevoSexo == 'F') {
            paciente->sexo = nuevoSexo;
        } else {
            cout << "Sexo no válido. Use M o F. Manteniendo valor actual." << endl;
        }
    }
    
    
    cout << "Teléfono actual: " << paciente->telefono << endl;
    cout << "Nuevo teléfono: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente->telefono, buffer);
    }
 
    cout << "Dirección actual: " << paciente->direccion << endl;
    cout << "Nueva dirección: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente->direccion, buffer);
    }
    
  
    cout << "Email actual: " << paciente->email << endl;
    cout << "Nuevo email: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente->email, buffer);
    }
    
  
    cout << "Tipo de sangre actual: " << paciente->tipoSangre << endl;
    cout << "Nuevo tipo de sangre: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente->tipoSangre, buffer);
    }
    

    cout << "Alergias actuales: " << paciente->alergias << endl;
    cout << "Nuevas alergias: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente->alergias, buffer);
    }
    
 
    cout << "Observaciones actuales: " << paciente->observaciones << endl;
    cout << "Nuevas observaciones: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente->observaciones, buffer);
    }
    
    cout << "\nDatos del paciente actualizados correctamente." << endl;
    return true;
}

bool eliminarPaciente(Hospital* hospital, int id) {

    int indice = -1;
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        if (hospital->pacientes[i].id == id) {
            indice = i;
            break;
        }
    }
    
    if (indice == -1) {
        cout << "Error: No se encontró paciente con ID " << id << endl;
        return false;
    }
    
    Paciente* paciente = &hospital->pacientes[indice];
    
    // Liberar memoria de los arregloss dinámicos del paciente
    delete[] paciente->historial;
    delete[] paciente->citasAgendadas;
    
    // Eliminar o cancelar todas las citas asociadas al paciente
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idPaciente == id) {
            strcpy(hospital->citas[i].estado, "CANCELADA");
            hospital->citas[i].atendida = false;
            
            // También remover esta cita de los doctores
            for (int j = 0; j < hospital->cantidadDoctores; j++) {
                if (hospital->doctores[j].id == hospital->citas[i].idDoctor) {
                    // Buscar y eliminar la cita del array del doctor
                    for (int k = 0; k < hospital->doctores[j].cantidadCitas; k++) {
                        if (hospital->doctores[j].citasAgendadas[k] == hospital->citas[i].id) {
                            // Mover citas restantes hacia adelante
                            for (int l = k; l < hospital->doctores[j].cantidadCitas - 1; l++) {
                                hospital->doctores[j].citasAgendadas[l] = hospital->doctores[j].citasAgendadas[l + 1];
                            }
                            hospital->doctores[j].cantidadCitas--;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    
   
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        for (int j = 0; j < hospital->doctores[i].cantidadPacientes; j++) {
            if (hospital->doctores[i].pacientesAsignados[j] == id) {
              
                for (int k = j; k < hospital->doctores[i].cantidadPacientes - 1; k++) {
                    hospital->doctores[i].pacientesAsignados[k] = hospital->doctores[i].pacientesAsignados[k + 1];
                }
                hospital->doctores[i].cantidadPacientes--;
                break;
            }
        }
    }
    
    
    for (int i = indice; i < hospital->cantidadPacientes - 1; i++) {
        hospital->pacientes[i] = hospital->pacientes[i + 1];
    }
    hospital->cantidadPacientes--;
    
    cout << "Paciente con ID " << id << " eliminado correctamente." << endl;
    cout << "Citas asociadas canceladas y referencias eliminadas." << endl;
    
    return true;
}

void listarPacientes(Hospital* hospital) {
    if (hospital->cantidadPacientes == 0) {
        cout << "No hay pacientes registrados." << endl;
        return;
    }
    
    cout << "\nLISTA DE PACIENTES (" << hospital->cantidadPacientes << "):" << endl;
    cout << "┌──────┬──────────────────────────┬──────────────┬──────┬────────────┐" << endl;
    cout << "│  ID  │       NOMBRE COMPLETO    │    CÉDULA    │ EDAD │ CONSULTAS  │" << endl;
    cout << "├──────┼──────────────────────────┼──────────────┼──────┼────────────┤" << endl;
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        Paciente* p = &hospital->pacientes[i];
        
        
        char nombreCompleto[100];
        snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", p->nombre, p->apellido);
        
        if (strlen(nombreCompleto) > 22) {
            nombreCompleto[19] = '.';
            nombreCompleto[20] = '.';
            nombreCompleto[21] = '.';
            nombreCompleto[22] = '\0';
        }
        
        printf("│ %4d │ %-24s │ %-12s │ %4d │ %10d │\n", 
               p->id, nombreCompleto, p->cedula, p->edad, p->cantidadConsultas);
    }
    
    cout << "└──────┴──────────────────────────┴──────────────┴──────┴────────────┘" << endl;

}

void agregarConsultaAlHistorial(Paciente* paciente, HistorialMedico consulta) {
  
    if (paciente->cantidadConsultas >= paciente->capacidadHistorial) {
 
        int nuevaCapacidad = paciente->capacidadHistorial * 2;

        HistorialMedico* nuevoHistorial = new HistorialMedico[nuevaCapacidad];
        
        for (int i = 0; i < paciente->cantidadConsultas; i++) {
            nuevoHistorial[i] = paciente->historial[i];
        }
        
        delete[] paciente->historial;
        
        paciente->historial = nuevoHistorial;
        paciente->capacidadHistorial = nuevaCapacidad;
        
        cout << "Historial médico redimensionado. Nueva capacidad: " << nuevaCapacidad << endl;
    }
    
    paciente->historial[paciente->cantidadConsultas] = consulta;
    
    paciente->cantidadConsultas++;
    
    cout << "Consulta agregada al historial. Total de consultas: " << paciente->cantidadConsultas << endl;
}

HistorialMedico* obtenerHistorialCompleto(Paciente* paciente, int* cantidad) {
    if (!paciente || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
    *cantidad = paciente->cantidadConsultas;
    return paciente->historial;
}

void mostrarHistorialMedico(Paciente* paciente) {
    if (!paciente) {
        cout << "Error: Paciente no válido." << endl;
        return;
    }
    
    if (paciente->cantidadConsultas == 0) {
        cout << "El paciente no tiene consultas en su historial médico." << endl;
        return;
    }
    
    cout << "\n=== HISTORIAL MÉDICO ===" << endl;
    cout << "Paciente: " << paciente->nombre << " " << paciente->apellido << endl;
    cout << "Cédula: " << paciente->cedula << " | Edad: " << paciente->edad << endl;
    cout << "Total de consultas: " << paciente->cantidadConsultas << endl;
    cout << endl;
    
    // Encabezado de la tabla
    cout << left << setw(12) << "FECHA" 
         << setw(8) << "HORA" 
         << setw(25) << "DIAGNÓSTICO" 
         << setw(8) << "DOCTOR"
         << setw(10) << "COSTO" 
         << endl;
    

    cout << string(65, '-') << endl;
    

    int cantidad;
    HistorialMedico* historial = obtenerHistorialCompleto(paciente, &cantidad);
    

    for (int i = 0; i < cantidad; i++) {
        
        char diagnosticoMostrar[26];
        strcpy(diagnosticoMostrar, historial[i].diagnostico);
        if (strlen(diagnosticoMostrar) > 23) {
            diagnosticoMostrar[20] = '.';
            diagnosticoMostrar[21] = '.';
            diagnosticoMostrar[22] = '.';
            diagnosticoMostrar[23] = '\0';
        }
        
        cout << left << setw(12) << historial[i].fecha
             << setw(8) << historial[i].hora
             << setw(25) << diagnosticoMostrar
             << setw(8) << historial[i].idDoctor
             << "$" << setw(9) << fixed << setprecision(2) << historial[i].costo
             << endl;
    }
    
    
    cout << string(65, '-') << endl;

    float costoTotal = 0;
    for (int i = 0; i < cantidad; i++) {
        costoTotal += historial[i].costo;
    }
    cout << "Costo total del historial: $" << fixed << setprecision(2) << costoTotal << endl;
}

HistorialMedico* obtenerUltimaConsulta(Paciente* paciente){
    if (!paciente || paciente->cantidadConsultas == 0) {
        return nullptr;
    }
    return &paciente->historial[paciente->cantidadConsultas - 1];
}


//MODULO DE DOCTORES

Doctor* crearDoctor(Hospital* hospital, const char* nombre,
                   const char* apellido, const char* cedula,
                   const char* especialidad, int aniosExperiencia,
                   float costoConsulta) {
    
   
    if (!hospital || !nombre || !apellido || !cedula || !especialidad) {
        cout << "Error: Parámetros inválidos." << endl;
        return nullptr;
    }
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (strcmp(hospital->doctores[i].cedula, cedula) == 0) {
            cout << "Error: Ya existe un doctor con la misma cédula " << cedula << endl;
            return nullptr;
        }
    }

    if (aniosExperiencia < 0) {
        cout << "Error: Los años de experiencia no pueden ser negativos." << endl;
        return nullptr;
    }
    
    if (costoConsulta < 0) {
        cout << "Error: El costo de consulta no puede ser negativo." << endl;
        return nullptr;
    }
    
    if (hospital->cantidadDoctores >= hospital->capacidadDoctores) {
        int nuevaCapacidad = hospital->capacidadDoctores * 2;
        Doctor* nuevosDoctores = new Doctor[nuevaCapacidad];
        
        
        for (int i = 0; i < hospital->cantidadDoctores; i++) {
            nuevosDoctores[i] = hospital->doctores[i];
        }
        
        delete[] hospital->doctores;
        hospital->doctores = nuevosDoctores;
        hospital->capacidadDoctores = nuevaCapacidad;
        
        cout << "Capacidad de doctores aumentada a " << nuevaCapacidad << endl;
    }
    
    int indice = hospital->cantidadDoctores;
    

    hospital->doctores[indice].id = hospital->siguienteIdDoctor++;
    strcpy(hospital->doctores[indice].nombre, nombre);
    strcpy(hospital->doctores[indice].apellido, apellido);
    strcpy(hospital->doctores[indice].cedula, cedula);
    strcpy(hospital->doctores[indice].especialidad, especialidad);
    hospital->doctores[indice].aniosExperiencia = aniosExperiencia;
    hospital->doctores[indice].costoConsulta = costoConsulta;
    
 
    hospital->doctores[indice].capacidadPacientes = 10;
    hospital->doctores[indice].pacientesAsignados = new int[10];
    hospital->doctores[indice].cantidadPacientes = 0;
    
    hospital->doctores[indice].capacidadCitas = 20;
    hospital->doctores[indice].citasAgendadas = new int[20];
    hospital->doctores[indice].cantidadCitas = 0;
    
    strcpy(hospital->doctores[indice].horarioAtencion, "L-V 8:00-17:00");
    strcpy(hospital->doctores[indice].telefono, "");
    strcpy(hospital->doctores[indice].email, "");
    
    
    hospital->doctores[indice].disponible = true;
    
    
    hospital->cantidadDoctores++;
    
    cout << "Doctor creado exitosamente. ID: " << hospital->doctores[indice].id << endl;
    cout << "Especialidad: " << hospital->doctores[indice].especialidad << endl;
    cout << "Años experiencia: " << hospital->doctores[indice].aniosExperiencia << endl;
    cout << "Costo consulta: $" << hospital->doctores[indice].costoConsulta << endl;
    
    return &hospital->doctores[indice];
}
Doctor* buscarDoctorPorId(Hospital* hospital, int id) {
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (hospital->doctores[i].id == id) {
            return &hospital->doctores[i];
        }
    }
    
    return nullptr; 
}

Doctor** buscardoctorporespecialidad(Hospital* hospital, const char* especialidad, int* cantidad) {
    Doctor** resultados = new Doctor*[hospital->cantidadDoctores];
    *cantidad = 0;
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (strstr(hospital->doctores[i].especialidad, especialidad) != nullptr) {
            resultados[*cantidad] = &hospital->doctores[i];
            (*cantidad)++;
        }
    }
    
    return resultados; 
}
Doctor** buscarDoctoresPorEspecialidad(Hospital* hospital,
                                      const char* especialidad,
                                      int* cantidad) {
    
    if (!hospital || !especialidad || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
   
    *cantidad = 0;
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (strcmp(hospital->doctores[i].especialidad, especialidad) == 0) {
            (*cantidad)++;
        }
    }
    
   
    if (*cantidad == 0) {
        cout << "No se encontraron doctores con especialidad: " << especialidad << endl;
        return nullptr;
    }
    
    
    Doctor** resultados = new Doctor*[*cantidad];
    
    
    int indice = 0;
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (strcmp(hospital->doctores[i].especialidad, especialidad) == 0) {
            resultados[indice] = &hospital->doctores[i];
            indice++;
        }
    }
    
    cout << "Encontrados " << *cantidad << " doctores en " << especialidad << endl;
    return resultados;
}
void liberarResultadosDoctores(Doctor** resultados) {
    if (resultados) {
        delete[] resultados;
    }
}
void mostrarDoctoresEspecialidad(Doctor** doctores, int cantidad) {
    if (!doctores || cantidad == 0) {
        cout << "No hay doctores para mostrar." << endl;
        return;
    }
    
    cout << "\n=== DOCTORES ENCONTRADOS ===" << endl;
    cout << left << setw(4) << "ID" 
         << setw(20) << "NOMBRE COMPLETO"
         << setw(15) << "ESPECIALIDAD"
         << setw(5) << "AÑOS"
         << setw(10) << "COSTO"
         << setw(10) << "ESTADO" 
         << endl;
    cout << string(70, '-') << endl;
    
    for (int i = 0; i < cantidad; i++) {
        Doctor* d = doctores[i];
        char nombreCompleto[100];
        snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", d->nombre, d->apellido);
        
        cout << left << setw(4) << d->id
             << setw(20) << nombreCompleto
             << setw(15) << d->especialidad
             << setw(5) << d->aniosExperiencia
             << setw(10) << d->costoConsulta
             << setw(10) << (d->disponible ? "Activo" : "Inactivo")
             << endl;
    }
    cout << string(70, '-') << endl;
}
bool asignarpacienteadoctor(Hospital* hospital, int idDoctor, int idPaciente) {
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No se encontró doctor con ID " << idDoctor << endl;
        return false;
    }
    
    Paciente* paciente = buscarPacientePorId(hospital, idPaciente);
    if (!paciente) {
        cout << "Error: No se encontró paciente con ID " << idPaciente << endl;
        return false;
    }
    
    for (int i = 0; i < doctor->cantidadPacientes; i++) {
        if (doctor->pacientesAsignados[i] == idPaciente) {
            cout << "El paciente ya está asignado al doctor." << endl;
            return false;
        }
    }
    
    if (doctor->cantidadPacientes >= doctor->capacidadPacientes) {
        int nuevaCapacidad = doctor->capacidadPacientes * 2;
        int* nuevosPacientes = new int[nuevaCapacidad];
        
        for (int i = 0; i < doctor->cantidadPacientes; i++) {
            nuevosPacientes[i] = doctor->pacientesAsignados[i];
        }
        
        delete[] doctor->pacientesAsignados;
        doctor->pacientesAsignados = nuevosPacientes;
        doctor->capacidadPacientes = nuevaCapacidad;
        
        cout << "Capacidad de pacientes del doctor aumentada a " << nuevaCapacidad << endl;
    }
    
    doctor->pacientesAsignados[doctor->cantidadPacientes] = idPaciente;
    doctor->cantidadPacientes++;
    
    cout << "Paciente ID " << idPaciente << " asignado al Doctor ID " << idDoctor << " correctamente." << endl;
    return true;
}
bool removerPacienteDeDoctor(Doctor* doctor, int idPaciente) {
        if (!doctor) {
        cout << "Error: Doctor no válido." << endl;
        return false;
    }
    
    if (doctor->cantidadPacientes == 0) {
        cout << "El doctor no tiene pacientes asignados." << endl;
        return false;
    }
    
    
    int indice = -1;
    for (int i = 0; i < doctor->cantidadPacientes; i++) {
        if (doctor->pacientesAsignados[i] == idPaciente) {
            indice = i;
            break;
        }
    }
    
    if (indice == -1) {
        cout << "El paciente con ID " << idPaciente << " no está asignado a este doctor." << endl;
        return false;
    }
    
    // Compactar array (mover elementos hacia adelante)
    for (int i = indice; i < doctor->cantidadPacientes - 1; i++) {
        doctor->pacientesAsignados[i] = doctor->pacientesAsignados[i + 1];
    }
    
    
    doctor->cantidadPacientes--;
    
    cout << "Paciente con ID " << idPaciente << " removido del doctor "
         << doctor->nombre << " " << doctor->apellido << endl;
    cout << "Pacientes restantes asignados: " << doctor->cantidadPacientes << endl;
    
    return true;
}
void listarpacientesdedoctor(Hospital* hospital, int idDoctor) {
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No se encontró doctor con ID " << idDoctor << endl;
        return;
    }
    
    if (doctor->cantidadPacientes == 0) {
        cout << "El doctor " << doctor->nombre << " " << doctor->apellido << " no tiene pacientes asignados." << endl;
        return;
    }
    
    cout << "\n=== PACIENTES ASIGNADOS AL DOCTOR " << doctor->nombre << " " << doctor->apellido << " ===" << endl;
    cout << left << setw(4) << "ID" 
         << setw(20) << "NOMBRE COMPLETO"
         << setw(12) << "CÉDULA"
         << setw(5) << "EDAD"
         << endl;
    cout << string( 50, '-') << endl;
    
    for (int i = 0; i < doctor->cantidadPacientes; i++) {
        int idPaciente = doctor->pacientesAsignados[i];
        Paciente* paciente = buscarPacientePorId(hospital, idPaciente);
        if (paciente) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", paciente->nombre, paciente->apellido);
            
            cout << left << setw(4) << paciente->id
                 << setw(20) << nombreCompleto
                 << setw(12) << paciente->cedula
                 << setw(5) << paciente->edad
                 << endl;
        }
    }
    cout << string( 50, '-') << endl;
}
void listardoctores(Hospital* hospital) {
    if (hospital->cantidadDoctores == 0) {
        cout << "No hay doctores registrados." << endl;
        return;
    }
    
    cout << "\nLISTA DE DOCTORES (" << hospital->cantidadDoctores << "):" << endl;
    cout << "┌──────┬──────────────────────────┬──────────────┬───────────────┬────────────┐" << endl;
    cout << "│  ID  │       NOMBRE COMPLETO    │  ESPECIALIDAD │ AÑOS EXPERIENCIA │  COSTO    │" << endl;
    cout << "├──────┼──────────────────────────┼──────────────┼───────────────┼────────────┤" << endl;
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* d = &hospital->doctores[i];
        
        
        char nombreCompleto[100];
        snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", d->nombre, d->apellido);
        
        if (strlen(nombreCompleto) > 22) {
            nombreCompleto[19] = '.';
            nombreCompleto[20] = '.';
            nombreCompleto[21] = '.';
            nombreCompleto[22] = '\0';
        }
        
        printf("│ %4d │ %-24s │ %-12s │     %4d     │ $%8.2f │\n", 
               d->id, nombreCompleto, d->especialidad, d->aniosExperiencia, d->costoConsulta);
    }
    
    cout << "└──────┴──────────────────────────┴──────────────┴───────────────┴────────────┘" << endl;

}
bool eliminarDoctor(Hospital* hospital, int id) {
    // Buscar el índice del doctor
    int indice = -1;
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (hospital->doctores[i].id == id) {
            indice = i;
            break;
        }
    }
    
    if (indice == -1) {
        cout << "Error: No se encontró doctor con ID " << id << endl;
        return false;
    }
    
    Doctor* doctor = &hospital->doctores[indice];
    // Liberar memoria de los arreglos dinámicos del doctor
    delete[] doctor->pacientesAsignados;
    delete[] doctor->citasAgendadas;
    
    
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == id) {
            strcpy(hospital->citas[i].estado, "CANCELADA");
            hospital->citas[i].atendida = false;
            
            
            for (int j = 0; j < hospital->cantidadPacientes; j++) {
                for (int k = 0; k < hospital->pacientes[j].cantidadCitas; k++) {
                    if (hospital->pacientes[j].citasAgendadas[k] == hospital->citas[i].id) {
                        // Mover citas restantes hacia adelante
                        for (int l = k; l < hospital->pacientes[j].cantidadCitas - 1; l++) {
                            hospital->pacientes[j].citasAgendadas[l] = hospital->pacientes[j].citasAgendadas[l + 1];
                        }
                        hospital->pacientes[j].cantidadCitas--;
                        break;
                    }
                }
            }
        }
    }
    
    
    //  Compactar el arreglo de doctores 
    for (int i = indice; i < hospital->cantidadDoctores - 1; i++) {
        hospital->doctores[i] = hospital->doctores[i + 1];
    }
    
    hospital->cantidadDoctores--;
    
    cout << "Doctor con ID " << id << " eliminado correctamente." << endl;
    cout << "Citas asociadas canceladas y memoria liberada." << endl;
    
    return true;
}
Cita* agendarCita(Hospital* hospital, int idPaciente, int idDoctor,
                 const char* fecha, const char* hora, const char* motivo) {
    
    // Validaciones básicas de parámetros
    if (!hospital || !fecha || !hora || !motivo) {
        cout << "Error: Parámetros inválidos." << endl;
        return nullptr;
    }
    Cita** obtenerCitasDePaciente(Hospital* hospital, int idPaciente, int* cantidad) {
    // Validar parámetros
    if (!hospital || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
    
    // Verificar que el paciente exista
    Paciente* paciente = buscarPacientePorId(hospital, idPaciente);
    if (!paciente) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        *cantidad = 0;
        return nullptr;
    }
    
    // Contar cuántas citas tiene el paciente
    *cantidad = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idPaciente == idPaciente) {
            (*cantidad)++;
        }
    }
    
    // Si no tiene citas, retornar nullptr
    if (*cantidad == 0) {
        cout << "El paciente no tiene citas registradas." << endl;
        return nullptr;
    }
    
    // Crear arreglo dinámico de punteros a Cita
    Cita** resultados = new Cita*[*cantidad];
    
    // Llenar el arreglo con punteros a las citas del paciente
    int indice = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idPaciente == idPaciente) {
            resultados[indice] = &hospital->citas[i];
            indice++;
        }
    }
    
    cout << "Encontradas " << *cantidad << " citas para el paciente ID " << idPaciente << endl;
    return resultados;
}
void liberarResultadosCitas(Cita** resultados) {
    if (resultados) {
        delete[] resultados;
    }
}
Cita** obtenerCitasDeDoctor(Hospital* hospital, int idDoctor, int* cantidad) {
    // Validar parámetros
    if (!hospital || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
    
    // Verificar que el doctor exista
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        *cantidad = 0;
        return nullptr;
    }
    
    // Contar cuántas citas tiene el doctor
    *cantidad = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == idDoctor) {
            (*cantidad)++;
        }
    }
    
    // Si no tiene citas, retornar nullptr
    if (*cantidad == 0) {
        cout << "El doctor no tiene citas registradas." << endl;
        return nullptr;
    }
    
    // Crear arreglo dinámico de punteros a Cita
    Cita** resultados = new Cita*[*cantidad];
    
    // Llenar el arreglo con punteros a las citas del doctor
    int indice = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == idDoctor) {
            resultados[indice] = &hospital->citas[i];
            indice++;
        }
    }
    
    cout << "Encontradas " << *cantidad << " citas para el doctor ID " << idDoctor << endl;
    return resultados;
}
Cita** obtenerCitasPorFecha(Hospital* hospital, const char* fecha, int* cantidad) {
    // Validar parámetros
    if (!hospital || !fecha || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
    
    // Validar formato de fecha
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha inválido. Use YYYY-MM-DD" << endl;
        *cantidad = 0;
        return nullptr;
    }
    
    // Contar cuántas citas hay para esa fecha
    *cantidad = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (strcmp(hospital->citas[i].fecha, fecha) == 0) {
            (*cantidad)++;
        }
    }
    
    // Si no hay citas para esa fecha, retornar nullptr
    if (*cantidad == 0) {
        cout << "No hay citas registradas para la fecha " << fecha << endl;
        return nullptr;
    }
    
    // Crear array dinámico de punteros a Cita
    Cita** resultados = new Cita*[*cantidad];
    
    // Llenar el array con punteros a las citas de la fecha
    int indice = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (strcmp(hospital->citas[i].fecha, fecha) == 0) {
            resultados[indice] = &hospital->citas[i];
            indice++;
        }
    }
    
    cout << "Encontradas " << *cantidad << " citas para la fecha " << fecha << endl;
    return resultados;
}

void listarCitasPendientes(Hospital* hospital) {
    if (!hospital || hospital->cantidadCitas == 0) {
        cout << "No hay citas en el sistema." << endl;
        return;
    }
    
    int cantidadPendientes = 0;
    
    // Primero contar
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (strcmp(hospital->citas[i].estado, "AGENDADA") == 0) {
            cantidadPendientes++;
        }
    }
    
    if (cantidadPendientes == 0) {
        cout << "No hay citas pendientes." << endl;
        return;
    }
    
    cout << "\n══════════════════════════════════════════════════════════════════════════════" << endl;
    cout << "                         CITAS PENDIENTES - TOTAL: " << cantidadPendientes << endl;
    cout << "══════════════════════════════════════════════════════════════════════════════" << endl;
    
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (strcmp(hospital->citas[i].estado, "AGENDADA") == 0) {
            Cita* c = &hospital->citas[i];
            Paciente* p = buscarPacientePorId(hospital, c->idPaciente);
            Doctor* d = buscarDoctorPorId(hospital, c->idDoctor);
            
            cout << "┌── CITA #" << c->id << " ───────────────────────────────────────────────────────────┐" << endl;
            cout << "│ Fecha: " << setw(12) << left << c->fecha << " Hora: " << setw(8) << c->hora;
            cout << " Especialidad: " << setw(15) << (d ? d->especialidad : "N/A") << " │" << endl;
            cout << "│ Paciente: " << setw(25) << (p ? string(p->nombre) + " " + p->apellido : "No encontrado");
            cout << " Cédula: " << setw(12) << (p ? p->cedula : "N/A") << " │" << endl;
            cout << "│ Doctor: " << setw(27) << (d ? "Dr. " + string(d->nombre) + " " + d->apellido : "No encontrado");
            cout << " Costo: $" << setw(8) << fixed << setprecision(2) << (d ? d->costoConsulta : 0) << " │" << endl;
            cout << "│ Motivo: " << setw(58) << c->motivo << " │" << endl;
            cout << "└────────────────────────────────────────────────────────────────────────────────────┘" << endl;
        }
    }
    
    cout << "══════════════════════════════════════════════════════════════════════════════" << endl;
}

bool verificarDisponibilidad(Hospital* hospital, int idDoctor,
                            const char* fecha, const char* hora) {
    // Validar parámetros
    if (!hospital || !fecha || !hora) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }
    
    // Verificar que el doctor exista
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return false;
    }
    
    // Verificar que el doctor esté disponible
    if (!doctor->disponible) {
        cout << "El doctor no está disponible para consultas." << endl;
        return false;
    }
    
    // Validar formato de fecha
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha inválido." << endl;
        return false;
    }
    
    // Validar formato de hora
    if (strlen(hora) != 5 || hora[2] != ':') {
        cout << "Error: Formato de hora inválido." << endl;
        return false;
    }
    
    // Verificar si el doctor ya tiene una cita a esa fecha/hora
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == idDoctor &&
            strcmp(hospital->citas[i].fecha, fecha) == 0 &&
            strcmp(hospital->citas[i].hora, hora) == 0 &&
            strcmp(hospital->citas[i].estado, "AGENDADA") == 0) {
            
            // Encontrar información del paciente para el mensaje
            Paciente* paciente = buscarPacientePorId(hospital, hospital->citas[i].idPaciente);
            cout << "El doctor ya tiene una cita agendada para " << fecha << " a las " << hora << endl;
            if (paciente) {
                cout << "Cita con: " << paciente->nombre << " " << paciente->apellido << endl;
            }
            return false;
        }
    }
    
    // Si llegamos aquí, el doctor está disponible
    cout << "Doctor disponible el " << fecha << " a las " << hora << endl;
    cout << "Doctor: " << doctor->nombre << " " << doctor->apellido << endl;
    cout << "Especialidad: " << doctor->especialidad << endl;
    
    return true;
}
    
    // Verificar que el paciente exista
    Paciente* paciente = buscarPacientePorId(hospital, idPaciente);
    if (!paciente) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        return nullptr;
    }
    
    //  Verificar que el doctor exista
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return nullptr;
    }
    
    // Verificar que el doctor esté disponible
    if (!doctor->disponible) {
        cout << "Error: El doctor no está disponible." << endl;
        return nullptr;
    }
    
    // Validar formato de fecha (YYYY-MM-DD)
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha inválido. Use YYYY-MM-DD" << endl;
        return nullptr;
    }
    
    // Validar que los componentes de la fecha sean números
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(fecha[i])) {
            cout << "Error: La fecha debe contener solo números y guiones." << endl;
            return nullptr;
        }
    }
    
    //  Validar formato de hora (HH:MM)
    if (strlen(hora) != 5 || hora[2] != ':') {
        cout << "Error: Formato de hora inválido. Use HH:MM" << endl;
        return nullptr;
    }
    
    // Validar que los componentes de la hora sean números
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) {
            cout << "Error: La hora debe contener solo números y dos puntos." << endl;
            return nullptr;
        }
    }
    
    // Validar rango de hora (00-23:00-59)
    int horas = atoi(hora);
    int minutos = atoi(hora + 3);
    if (horas < 0 || horas > 23 || minutos < 0 || minutos > 59) {
        cout << "Error: Hora fuera de rango. Use HH:MM entre 00:00 y 23:59" << endl;
        return nullptr;
    }
    
    //Verificar disponibilidad del doctor (no deberia tener otra cita a esa hora/fecha)
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == idDoctor &&
            strcmp(hospital->citas[i].fecha, fecha) == 0 &&
            strcmp(hospital->citas[i].hora, hora) == 0 &&
            strcmp(hospital->citas[i].estado, "AGENDADA") == 0) {
            cout << "Error: El doctor ya tiene una cita agendada para " << fecha << " a las " << hora << endl;
            return nullptr;
        }
    }
    
    // Verificar que el paciente no tenga otra cita a la misma hora
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idPaciente == idPaciente &&
            strcmp(hospital->citas[i].fecha, fecha) == 0 &&
            strcmp(hospital->citas[i].hora, hora) == 0 &&
            strcmp(hospital->citas[i].estado, "AGENDADA") == 0) {
            cout << "Error: El paciente ya tiene una cita agendada para " << fecha << " a las " << hora << endl;
            return nullptr;
        }
    }
    
    // Redimensionar arreglo de citas si está lleno
    if (hospital->cantidadCitas >= hospital->capacidadCitas) {
        int nuevaCapacidad = hospital->capacidadCitas * 2;
        Cita* nuevasCitas = new Cita[nuevaCapacidad];
        
        
        for (int i = 0; i < hospital->cantidadCitas; i++) {
            nuevasCitas[i] = hospital->citas[i];
        }
        delete[] hospital->citas;
        hospital->citas = nuevasCitas;
        hospital->capacidadCitas = nuevaCapacidad;
        
        cout << "Capacidad de citas aumentada a " << nuevaCapacidad << endl;
    }
    
    // Obtener índice de la nueva cita
    int indice = hospital->cantidadCitas;
    
    // Crear estructura Cita
    hospital->citas[indice].id = hospital->siguienteIdCita++;
    hospital->citas[indice].idPaciente = idPaciente;
    hospital->citas[indice].idDoctor = idDoctor;
    strcpy(hospital->citas[indice].fecha, fecha);
    strcpy(hospital->citas[indice].hora, hora);
    strcpy(hospital->citas[indice].motivo, motivo);
    strcpy(hospital->citas[indice].estado, "AGENDADA");
    strcpy(hospital->citas[indice].observaciones, "");
    hospital->citas[indice].atendida = false;
    
    // Agregar ID de cita al array del paciente
    if (paciente->cantidadCitas >= paciente->capacidadCitas) {
        int nuevaCapacidad = paciente->capacidadCitas * 2;
        int* nuevasCitasPaciente = new int[nuevaCapacidad];
        
        for (int i = 0; i < paciente->cantidadCitas; i++) {
            nuevasCitasPaciente[i] = paciente->citasAgendadas[i];
        }
        
        delete[] paciente->citasAgendadas;
        paciente->citasAgendadas = nuevasCitasPaciente;
        paciente->capacidadCitas = nuevaCapacidad;
    }
    paciente->citasAgendadas[paciente->cantidadCitas++] = hospital->citas[indice].id;
    
    // Agregar ID de cita al arreglo del doctor
    if (doctor->cantidadCitas >= doctor->capacidadCitas) {
        int nuevaCapacidad = doctor->capacidadCitas * 2;
        int* nuevasCitasDoctor = new int[nuevaCapacidad];
        
        for (int i = 0; i < doctor->cantidadCitas; i++) {
            nuevasCitasDoctor[i] = doctor->citasAgendadas[i];
        }
        
        delete[] doctor->citasAgendadas;
        doctor->citasAgendadas = nuevasCitasDoctor;
        doctor->capacidadCitas = nuevaCapacidad;
    }
    doctor->citasAgendadas[doctor->cantidadCitas++] = hospital->citas[indice].id;
    
    
    hospital->cantidadCitas++;
    
    cout << "Cita agendada exitosamente. ID: " << hospital->citas[indice].id << endl;
    cout << "Paciente: " << paciente->nombre << " " << paciente->apellido << endl;
    cout << "Doctor: " << doctor->nombre << " " << doctor->apellido << endl;
    cout << "Fecha: " << fecha << " " << hora << endl;
    cout << "Motivo: " << motivo << endl;
    
    return &hospital->citas[indice];
}

Hospital* inicializarHospital(const char* nombre = "Hospital Central", 
                             int capacidadPacientes = 10,
                             int capacidadDoctores = 5,
                             int capacidadCitas = 20) {
    
    if (!nombre || capacidadPacientes <= 0 || capacidadDoctores <= 0 || capacidadCitas <= 0) {
        cout << "Error: Parámetros inválidos para inicializar hospital." << endl;
        return nullptr;
    }
    
    
    Hospital* hospital = new Hospital;
    
    strcpy(hospital->nombre, nombre);
    strcpy(hospital->direccion, "Por definir");
    strcpy(hospital->telefono, "Por definir");
    
   
    hospital->capacidadPacientes = capacidadPacientes;
    hospital->pacientes = new Paciente[capacidadPacientes];
    hospital->cantidadPacientes = 0;
    
    hospital->capacidadDoctores = capacidadDoctores;
    hospital->doctores = new Doctor[capacidadDoctores];
    hospital->cantidadDoctores = 0;
    
    hospital->capacidadCitas = capacidadCitas;
    hospital->citas = new Cita[capacidadCitas];
    hospital->cantidadCitas = 0;
    
  
    hospital->siguienteIdPaciente = 1;
    hospital->siguienteIdDoctor = 1;
    hospital->siguienteIdCita = 1;
    hospital->siguienteIdConsulta = 1;
    
    cout << "    Hospital '" << nombre << "' inicializado correctamente" << endl;
    cout << "   Capacidades configuradas:" << endl;
    cout << "    Pacientes: " << capacidadPacientes << endl;
    cout << "    Doctores: " << capacidadDoctores << endl;
    cout << "    Citas: " << capacidadCitas << endl;
    cout << endl;
    
    return hospital;
}
