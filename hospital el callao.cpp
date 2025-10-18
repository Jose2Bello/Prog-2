#include <iostream>
#include <locale>
#include <iomanip>
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
    

    strcpy(hospital->nombre, "Hospital El Callao");
    strcpy(hospital->direccion, "Av. El milagro");
    strcpy(hospital->telefono, "0424-6292319");

    hospital->capacidadPacientes = 10;
    hospital->pacientes = new Paciente[hospital->capacidadPacientes];
    hospital->cantidadPacientes = 0;
    
    hospital->capacidadDoctores = 5;
    hospital->doctores = new Doctor[hospital->capacidadDoctores];
    hospital->cantidadDoctores = 0;
    
    hospital->capacidadCitas = 20;
    hospital->citas = new Cita[hospital->capacidadCitas];
    hospital->cantidadCitas = 0;
    
  
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

    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        delete[] hospital->pacientes[i].historial;
        delete[] hospital->pacientes[i].citasAgendadas;
    }
   
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        delete[] hospital->doctores[i].pacientesAsignados;
        delete[] hospital->doctores[i].citasAgendadas;
    }
 
    delete[] hospital->pacientes;
    delete[] hospital->doctores;
    delete[] hospital->citas;
    
    
    delete hospital;
    
    cout << "Memoria del hospital liberada correctamente" << endl;
}


