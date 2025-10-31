1. Descripción del Proyecto

Objetivo General:
Desarrollar un sistema integral de gestión hospitalaria que permita administrar pacientes, doctores y citas médicas de manera eficiente, manteniendo un historial médico completo y facilitando la coordinación entre los diferentes actores del sistema de salud.

Funcionalidades Principales

Gestión completa de pacientes (registro, búsqueda, actualización, eliminación)

Administración de doctores y sus especialidades

Sistema de citas médicas con validación de disponibilidad

Historial médico digital por paciente

Asignación de pacientes a doctores

Control de costos y consultas médicas

Instrucciones de Compilación
bash

g++ -std=c++11 -o hospital hospital.cpp

Requisitos:

Compilador C++11 o superior

Sistema operativo: Windows/Linux/macOS

Suficiente memoria RAM para la ejecución (1mb va sobrao)

3. Instrucciones de Ejecución
bash
./hospital

El sistema mostrará un menú principal con las siguientes opciones:

Gestión de Pacientes

Gestión de Doctores

Gestión de Citas

Salir

4. Estructuras de Datos
Hospital (Struct Principal)
cpp
struct Hospital {
    char nombre[100], direccion[150], telefono[15];
    Paciente* pacientes;           // Array dinámico
    Doctor* doctores;              // Array dinámico  
    Cita* citas;                   // Array dinámico
    int cantidadPacientes, capacidadPacientes;
    int cantidadDoctores, capacidadDoctores;
    int cantidadCitas, capacidadCitas;
    int siguienteIdPaciente, siguienteIdDoctor;
    int siguienteIdCita, siguienteIdConsulta;
};
Paciente
cpp
struct Paciente {
    int id, edad;
    char nombre[50], apellido[50], cedula[20];
    char sexo, tipoSangre[5], telefono[15];
    char direccion[100], email[50];
    HistorialMedico* historial;    // Array dinámico
    int* citasAgendadas;           // Array dinámico
    int cantidadConsultas, capacidadHistorial;
    int cantidadCitas, capacidadCitas;
    char alergias[500], observaciones[500];
    bool activo;
};
Doctor
cpp
struct Doctor {
    int id, aniosExperiencia;
    char nombre[50], apellido[50], cedula[20];
    char especialidad[50], horarioAtencion[50];
    char telefono[15], email[50];
    float costoConsulta;
    int* pacientesAsignados;       // Array dinámico
    int* citasAgendadas;           // Array dinámico
    int cantidadPacientes, capacidadPacientes;
    int cantidadCitas, capacidadCitas;
    bool disponible;
};
Cita
cpp
struct Cita {
    int id, idPaciente, idDoctor;
    char fecha[11], hora[6], motivo[150];
    char estado[20], observaciones[200];
    bool atendida;
};
Historial Médico
cpp
struct HistorialMedico {
    int idConsulta, idDoctor;
    char fecha[11], hora[6];
    char diagnostico[200], tratamiento[200];
    char medicamentos[150];
    float costo;
};
Relaciones entre Estructuras
Hospital contiene arrays de Pacientes, Doctores y Citas

Paciente tiene un historial médico y referencias a citas

Doctor mantiene listas de pacientes asignados y citas

Cita conecta Paciente y Doctor

Historial Médico está asociado a un Paciente y un Doctor

5. Funciones Principales
Módulo de Hospital
inicializarHospital(): Crea y configura la estructura principal

liberarHospital(): Libera toda la memoria dinámica

destruirHospital(): Limpieza completa del sistema

Módulo de Pacientes
crearPaciente(): Registra nuevo paciente

buscarPacientePorCedula(), buscarPacientePorId(), buscarPacientesPorNombre()

actualizarPaciente(): Modifica datos existentes

eliminarPaciente(): Elimina con limpieza de referencias

listarPacientes(): Muestra lista formateada

Módulo de Historial Médico
agregarConsultaAlHistorial(): Añade consulta con redimensionamiento

mostrarHistorialMedico(): Visualización completa

obtenerHistorialCompleto(): Retorna todo el historial

obtenerUltimaConsulta(): Última consulta realizada

Módulo de Doctores
crearDoctor(): Registra nuevo doctor

buscarDoctorPorId(), buscarDoctoresPorEspecialidad()

asignarPacienteADoctor(): Establece relación doctor-paciente

listarDoctores(): Muestra lista formateada

eliminarDoctor(): Elimina con limpieza de referencias

Módulo de Citas
agendarCita(): Crea nueva cita con validaciones

cancelarCita(): Cancela cita pendiente

atenderCita(): Marca cita como atendida y crea historial

obtenerCitasDePaciente(), obtenerCitasDeDoctor(), obtenerCitasPorFecha()

listarCitasPendientes(): Muestra citas agendadas

verificarDisponibilidad(): Valida horario disponible

Módulo de Validaciones
validarFecha(), validarHora(): Validación formatos

validarEmail(): Verificación formato email

compararFechas(): Comparación de fechas

Módulo de Utilidades
limpiarBuffer(): Limpia buffer de entrada

pausarPantalla(): Pausa para continuar

leerOpcion(): Lectura validada de opciones

copiarString(), copiarPaciente(): Funciones de copia

Módulo de Interfaz
mostrarMenuPrincipal(): Menú principal

menuGestionPacientes(): Submenú pacientes

menuGestionDoctores(): Submenú doctores

menuGestionCitas(): Submenú citas

6. Decisiones de Diseño
Por qué se eligieron arrays dinámicos?
Flexibilidad para el proyecto, permiten manejar una mayor cantidad de datos de manera mejor estructurada y optimizada
El redimensionamiento en los arrays dinamicos permite que el codigo sea menos suceptible a fallar por falta de espacio
al solo requerir reservar exclusivamente el que va a utilizar o viceversa.
