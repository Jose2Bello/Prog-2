Sistema de Gestión Hospitalaria de el Callao
Sistema completo de gestión para hospitales y clínicas, desarrollado en C++ con almacenamiento persistente en archivos binarios.

1.  Características Principales
Gestión Integral

Pacientes - Historia clínico completa con seguimiento de consultas

Doctores - Especialidades, horarios y costos de consulta

Citas - Agendamiento, cancelación y atención médica

Hospital - Configuración institucional y estadísticas

2. Sistema Avanzado
IDs autoincrementales controlados por el sistema

Borrado lógico (no destructivo) de registros

Historial completo de modificaciones

Validación de datos en tiempo real

Archivos de configuración persistentes

3. Seguridad y Control
Validación de cédulas, emails y teléfonos

Control de versiones de archivos

Timestamps de creación y modificación

Backup automático de datos

4. Arquitectura del Sistema
Clases Principales
Hospital:	Configuración y estadísticas del hospital
Paciente:	Información médica y personal del paciente
Doctor:	Datos profesionales y especialidades
Cita:	Gestión de citas médicas
Validaciones:	Validación de datos de entrada
Formatos:	Formateo de salida y presentación
EntradaUsuario:	Entrada de datos segura
Menus:	Sistema de menús interactivos

5. Compilación
Requisitos:

Compilador C++11 o superior

Sistema operativo Windows/Linux/macOS

50 MB de espacio libre

Compilación con g++
bash
g++ -o hospital.exe src/*.cpp -Iinclude -std=c++11
Compilación con Makefile
bash
make
make clean     # Limpiar archivos compilados
🚀 Ejecución
bash
./hospital.exe
Al iniciar por primera vez, el sistema creará:

Carpeta datos/ si no existe

Archivos binarios con headers inicializados

Configuración predeterminada del hospital


6. Sistema de IDs
El hospital maneja IDs autoincrementales independientes:

Entidad	Rango	Controlado por
Pacientes	1000+	Hospital::siguienteIDPaciente
Doctores	2000+	Hospital::siguienteIDDoctor
Citas	3000+	Hospital::siguienteIDCita
Consultas	4000+	Hospital::siguienteIDConsulta


📋 Menús Disponibles
1. Menú Principal
[1] Gestión de Pacientes
[2] Gestión de Doctores  
[3] Gestión de Citas
[4] Historial Médico
[5] Mantenimiento del Sistema
[0] Salir



7. Gestión de Pacientes
Registrar nuevo paciente

Buscar/editar paciente

Listar pacientes activos

Ver historial médico

Buscar por cedula/nombre/id

3. Gestión de Doctores

Registrar nuevo doctor

Buscar por nombre/cedula/especialidad

Ver disponibilidad

4. Gestión de Citas
5. 
Agendar nueva cita

Atender cita (con diagnóstico/tratamiento)

Cancelar cita

Ver citas pendientes

Reportes por fecha/paciente/doctor

5. Historial Médico
agregar consulta al historial

Consultar historial completo

Agregar observaciones

Ver alergias y condiciones

6. Mantenimiento
Verificar integridad de archivos

Reconstruir archivos corruptos

Ver estadísticas del sistema

Realizar backup

🔐 Validaciones Implementadas
Pacientes
Cédula (formato nacional)

Email (sintaxis válida)

Teléfono (10 dígitos mínimo)

Edad (0-120 años)

Sexo (M/F/O)

Citas
Fecha (YYYY-MM-DD, futura)

Hora (HH:MM, horario laboral)

Disponibilidad del doctor

Paciente no duplicado en mismo horario

Doctores
Costo de consulta (positivo)

Especialidad (lista predefinida)

Horario (formato 24h)

💾 Sistema de Archivos
Estructura de Archivos Binarios
cpp
struct ArchivoHeader {
    int cantidadRegistros;        // Registros físicos
    int proximoID;                // Siguiente ID disponible  
    int registrosActivos;         // Registros no eliminados
    int version;                  // Versión del formato
    time_t fechaCreacion;         // Timestamp de creación
    time_t fechaUltimaModificacion;
    char tipoArchivo[20];         // Identificador
};

Archivos Generados
Archivo	Contenido	Tamaño Aprox
hospital.bin	Configuración y contadores	1 KB
pacientes.bin	Todos los pacientes	10 KB/paciente
doctores.bin	Doctores registrados	5 KB/doctor
citas.bin	Historial de citas	2 KB/cita
⚠️ Solución de Problemas
Error común: "No se puede encontrar cita con ID X"
Ejecutar Verificar integridad desde Mantenimiento

Si persiste, usar Reconstruir archivos

Verificar permisos de la carpeta datos/

Archivos corruptos
cpp
// Desde el menú de Mantenimiento:
[5] Mantenimiento del Sistema
  → [3] Verificar integridad de archivos
  → [4] Reconstruir archivos corruptos
Problemas de compilación
bash
# Asegurar estándar C++11
g++ -std=c++11 -o hospital.exe src/*.cpp -Iinclude

# Verificar includes
#include "Hospital.h"
#include "Paciente.h"
// ...
📈 Estadísticas del Sistema
El hospital lleva registro de:

✅ Total de pacientes registrados

✅ Total de doctores activos

✅ Citas agendadas (histórico)

✅ Consultas realizadas

✅ Próximos IDs disponibles
