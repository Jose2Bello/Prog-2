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

Hospital* inicializarHospital() {
    Hospital* hospital = new Hospital;
    
    // Inicializar datos básicos del hospital
    strcpy(hospital->nombre, "Hospital El Callao");
    strcpy(hospital->direccion, "Av. El milagro");
    strcpy(hospital->telefono, "0424-6292319");
    
    // Inicializar arrays dinámicos
    hospital->capacidadPacientes = 10;
    hospital->pacientes = new Paciente[hospital->capacidadPacientes];
    hospital->cantidadPacientes = 0;
    
    hospital->capacidadDoctores = 5;
    hospital->doctores = new Doctor[hospital->capacidadDoctores];
    hospital->cantidadDoctores = 0;
    
    hospital->capacidadCitas = 20;
    hospital->citas = new Cita[hospital->capacidadCitas];
    hospital->cantidadCitas = 0;
    
    // Inicializar contadores de ID
    hospital->siguienteIdPaciente = 1;
    hospital->siguienteIdDoctor = 1;
    hospital->siguienteIdCita = 1;
    hospital->siguienteIdConsulta = 1;
    
    cout << "Hospital inicializado correctamente" << endl;
    cout << "Capacidad inicial - Pacientes: " << hospital->capacidadPacientes;
    cout << ", Doctores: " << hospital->capacidadDoctores;
    cout << ", Citas: " << hospital->capacidadCitas << endl;
    
    return hospital;
}

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
