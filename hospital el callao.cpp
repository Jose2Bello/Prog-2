#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <thread>
#include <fstream>
#include <string>
#include <windows.h>


struct Paciente;
struct Doctor;
struct Cita;
struct HistorialMedico;

using namespace std;

const char* obtenerNombreMes(int mes);
void limpiarBuffer();
void pausarPantalla();

struct ArchivoHeader {
  int cantidadRegistros;  // Total de registros físicos
  int proximoID;          // Siguiente ID disponible
  int registrosActivos;   // Registros no eliminados
  int version;            // Versión del formato
  time_t fechaCreacion;
  time_t fechaUltimaModificacion;
  char tipoArchivo[20];  // "PACIENTES", "DOCTORES", etc.
  int ultimoIDUtilizado;  
};
// strucs de hospital, paciente, historial medico, doctor y cita

struct Hospital {
  char nombre[100];
  char direccion[150];
  char telefono[15];

  int siguienteIDPaciente;
  int siguienteIDDoctor;
  int siguienteIDCita;
  int siguienteIDConsulta;

  int totalPacientesRegistrados;
  int totalDoctoresRegistrados;
  int totalCitasAgendadas;
  int totalConsultasRealizadas;
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
  char alergias[500];
  char observaciones[500];
  bool activo;
  int cantidadConsultas;
  int primerConsultaID;
  int cantidadCitas;
  int citasIDs[20];
  bool eliminado;
  time_t fechaCreacion;
  time_t fechaModificacion;
};

struct HistorialMedico {
  int id;                    
  int pacienteID;              
  bool eliminado;            
  time_t fechaRegistro;      
  int siguienteConsultaID; 
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

    
    int pacientesIDs[50];        
    int cantidadPacientes;
    
    int citasIDs[30];           
    int cantidadCitas;

    bool disponible;
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
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
  bool eliminado;
};


void limpiarBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool archivoExiste(const char* nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary);
    bool existe = archivo.is_open();
    if (existe) {
        archivo.close();
    }
    return existe;
}

const char* formatearFecha(time_t tiempo) {
  static char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", localtime(&tiempo));
  return buffer;
}

Cita leerCitaPorIndice(int indice) {
    Cita cita;
    memset(&cita, 0, sizeof(Cita));
    cita.id = -1;  // Marcador de error

    if (indice < 0) return cita;

    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) return cita;

    // Leer header para verificar límites
    ArchivoHeader header;
    if (!archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader))) {
        archivo.close();
        return cita;
    }

    if (indice >= header.cantidadRegistros) {
        archivo.close();
        return cita;
    }

    // Calcular posición y leer
    long posicion = sizeof(ArchivoHeader) + (indice * sizeof(Cita));
    archivo.seekg(posicion);
    archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita));

    archivo.close();
    return cita;
}
bool verificarArchivo(const char* nombreArchivo) {
  
  ifstream archivo(nombreArchivo, ios::binary);
  if (!archivo.is_open()) {
    cout << "? Error: No se puede abrir el archivo " << nombreArchivo << endl;
    return false;
  }

  // Verificar que el archivo tenga al menos el tamaño del header
  archivo.seekg(0, ios::end);
  streampos fileSize = archivo.tellg();
  archivo.seekg(0, ios::beg);

  if (fileSize < static_cast<streampos>(sizeof(ArchivoHeader))) {
    cout << "? Error: Archivo " << nombreArchivo
         << " está corrupto (tamaño insuficiente: " << fileSize << " bytes)"
         << endl;
    archivo.close();
    return false;
  }

  // Leer header
  ArchivoHeader header;
  archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

  if (archivo.fail()) {
    cout << "? Error: No se pudo leer el header de " << nombreArchivo << endl;
    archivo.close();
    return false;
  }

  archivo.close();

  //  Validar campos críticos del header
  bool valido = true;
  string errores;

  // Validar versión
  if (header.version != 1) {
    errores +=
        "Versión inválida (" + to_string(header.version) + "), esperada 1. ";
    valido = false;
  }

  // Validar contadores no negativos
  if (header.cantidadRegistros < 0) {
    errores += "cantidadRegistros negativo. ";
    valido = false;
  }

  if (header.proximoID < 1) {
    errores += "proximoID inválido. ";
    valido = false;
  }

  if (header.registrosActivos < 0 ||
      header.registrosActivos > header.cantidadRegistros) {
    errores += "registrosActivos inconsistente. ";
    valido = false;
  }

  // Validar fechas (no pueden ser futuras)
  time_t ahora = time(nullptr);
  if (header.fechaCreacion > ahora) {
    errores += "fechaCreación en futuro. ";
    valido = false;
  }

  if (header.fechaUltimaModificacion > ahora) {
    errores += "fechaModificación en futuro. ";
    valido = false;
  }

  // Validar tipo de archivo
  if (strlen(header.tipoArchivo) == 0 ||
      strlen(header.tipoArchivo) >= sizeof(header.tipoArchivo)) {
    errores += "tipoArchivo inválido. ";
    valido = false;
  }

  // Mostrar resultado
  if (valido) {
    cout << "? Archivo válido: " << nombreArchivo << endl;
    cout << "   Tipo: " << header.tipoArchivo
         << " | Registros: " << header.registrosActivos << "/"
         << header.cantidadRegistros << " | Próximo ID: " << header.proximoID
         << endl;
  } else {
    cout << "? Archivo inválido: " << nombreArchivo << " - " << errores << endl;
  }

  return valido;
}

const char* determinarTipoArchivo(const char* nombreArchivo) {
  if (strstr(nombreArchivo, "pacientes") != nullptr) return "PACIENTES";
  if (strstr(nombreArchivo, "doctores") != nullptr) return "DOCTORES";
  if (strstr(nombreArchivo, "citas") != nullptr) return "CITAS";
  if (strstr(nombreArchivo, "historiales") != nullptr) return "HISTORIALES";
  if (strstr(nombreArchivo, "hospital") != nullptr) return "HOSPITAL";
  return "GENERAL";
}
void liberarHistorial(HistorialMedico* historial) {
    if (historial != nullptr) {
        delete[] historial;
    }
}
bool inicializarArchivo(const char* nombreArchivo) {
  ofstream archivo(nombreArchivo, ios::binary | ios::out);
  if (!archivo.is_open()) {
    cout << "Error: No se pudo crear el archivo " << nombreArchivo << endl;
    return false;
  }

  // Determinar tipo de archivo basado en el nombre
  const char* tipo = determinarTipoArchivo(nombreArchivo);

  // Crear header con valores iniciales
  ArchivoHeader header;
  header.cantidadRegistros = 0;
  header.proximoID = 1;
  header.registrosActivos = 0;
  header.version = 1;
  header.fechaCreacion = time(nullptr);
  header.fechaUltimaModificacion = time(nullptr);
  header.ultimoIDUtilizado = 0;

  // Copiar tipo de archivo
  strncpy(header.tipoArchivo, tipo, sizeof(header.tipoArchivo) - 1);
  header.tipoArchivo[sizeof(header.tipoArchivo) - 1] = '\0';

  // Escribir header al archivo
  archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));

  //  Verificar que se escribió correctamente
  if (archivo.fail()) {
    cout << "Error: No se pudo escribir el header en " << nombreArchivo << endl;
    archivo.close();
    return false;
  }

  archivo.close();

  cout << "Archivo inicializado: " << nombreArchivo << " (Tipo: " << tipo << ")"
       << endl;
  return true;
}


ArchivoHeader leerHeader(const char* nombreArchivo, bool mostrarInfo = false) {
  ArchivoHeader header;

  // Valores por defecto para header inválido
  memset(&header, 0, sizeof(header));
  header.proximoID = 1;
  header.version = 1;
  header.fechaCreacion = time(nullptr);
  header.fechaUltimaModificacion = header.fechaCreacion;
  strcpy(header.tipoArchivo, "INVALIDO");

  // Intentar leer archivo
  ifstream archivo(nombreArchivo, ios::binary);
  if (!archivo.is_open()) {
    if (mostrarInfo) {
      cout << "? No existe: " << nombreArchivo << endl;
    }
    return header;
  }

  // Verificar tamaño
  archivo.seekg(0, ios::end);
  long tamanio = archivo.tellg();
  archivo.seekg(0, ios::beg);

  if (tamanio < static_cast<long>(sizeof(ArchivoHeader))) {
    if (mostrarInfo) {
      cout << " Tamaño insuficiente: " << nombreArchivo << " (" << tamanio
           << "/" << sizeof(ArchivoHeader) << " bytes)" << endl;
    }
    archivo.close();
    return header;
  }

  // Leer header
  if (!archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader))) {
    if (mostrarInfo) {
      cout << "? Error de lectura: " << nombreArchivo << endl;
    }
    archivo.close();
    return header;
  }

  archivo.close();

  // Mostrar información si se solicita
  if (mostrarInfo) {
    cout << "?? HEADER de " << nombreArchivo << ":" << endl;
    cout << "   +-- Tipo: " << header.tipoArchivo << endl;
    cout << "   +-- Versión: " << header.version << endl;
    cout << "   +-- Registros: " << header.registrosActivos << "/"
         << header.cantidadRegistros << endl;
    cout << "   +-- Próximo ID: " << header.proximoID << endl;

    // Formatear fechas
    char fechaCreacion[20], fechaModificacion[20];
    strftime(fechaCreacion, sizeof(fechaCreacion), "%Y-%m-%d %H:%M",
             localtime(&header.fechaCreacion));
    strftime(fechaModificacion, sizeof(fechaModificacion), "%Y-%m-%d %H:%M",
             localtime(&header.fechaUltimaModificacion));

    cout << "   +-- Creado: " << fechaCreacion << endl;
    cout << "   +-- Modificado: " << fechaModificacion << endl;
  }

  return header;
}
HistorialMedico crearConsultaVacia() {
  HistorialMedico vacia;
  memset(&vacia, 0, sizeof(HistorialMedico));
  vacia.id = -1;
  vacia.siguienteConsultaID = -1;
  return vacia;
}
HistorialMedico buscarConsultaPorID(int consultaID) {
  ifstream archivo("historiales.bin", ios::binary);
  if (!archivo.is_open()) return crearConsultaVacia();

  archivo.seekg(sizeof(ArchivoHeader));
  HistorialMedico consulta;

  while (archivo.read(reinterpret_cast<char*>(&consulta),
                      sizeof(HistorialMedico))) {
    if (consulta.id == consultaID && !consulta.eliminado) {
      archivo.close();
      return consulta;
    }
  }

  archivo.close();
  return crearConsultaVacia();
}


bool actualizarHeader(const char* nombreArchivo, ArchivoHeader header) {
  //  Verificar que el archivo existe y es accesible
  if (!archivoExiste(nombreArchivo)) {
    cout << "? Error: " << nombreArchivo << " no existe" << endl;
    return false;
  }

  //  Abrir en modo lectura/escritura
  fstream archivo(nombreArchivo, ios::binary | ios::in | ios::out);
  if (!archivo.is_open()) {
    cout << "? Error: No se puede abrir " << nombreArchivo << " para escritura"
         << endl;
    return false;
  }

  //  Validar header antes de escribir
  if (header.proximoID < 1) {
    cout << "??  Advertencia: próximoID inválido (" << header.proximoID
         << "), ajustando a 1" << endl;
    header.proximoID = 1;
  }

  if (header.registrosActivos < 0) {
    cout << "??  Advertencia: registrosActivos negativo, ajustando a 0" << endl;
    header.registrosActivos = 0;
  }

  if (header.registrosActivos > header.cantidadRegistros) {
    cout << "??  Advertencia: registrosActivos > cantidadRegistros, ajustando"
         << endl;
    header.registrosActivos = header.cantidadRegistros;
  }

  // Actualizar metadata
  header.fechaUltimaModificacion = time(nullptr);

  //  Posicionarse al inicio y escribir
  archivo.seekp(0, ios::beg);
  archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));

  // Forzar escritura a disco
  archivo.flush();

  bool exito = !archivo.fail();
  archivo.close();

  if (exito) {
    cout << "? Header actualizado: " << nombreArchivo << endl;
    cout << "   ?? Nuevos valores - Registros: " << header.registrosActivos
         << "/" << header.cantidadRegistros
         << " | Próximo ID: " << header.proximoID << endl;
  } else {
    cout << "? Error crítico: No se pudo escribir header en " << nombreArchivo
         << endl;
  }

  return exito;
}

int buscarIndicePorID(int id) {
  if (id <= 0) return -1;

  ifstream archivo("pacientes.bin", ios::binary);
  if (!archivo.is_open()) return -1;

  // Leer header
  ArchivoHeader header;
  archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

  // Búsqueda secuencial por índice
  Paciente p;
  int indice = 0;

  while (indice < header.cantidadRegistros &&
         archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente))) {
    if (p.id == id) {
      archivo.close();
      return indice;
    }
    indice++;
  }

  archivo.close();
  return -1;
}


bool inicializarSistemaArchivos() {
    cout << "==========================================" << endl;
    cout << "   INICIALIZANDO SISTEMA DE ARCHIVOS" << endl;
    cout << "==========================================" << endl;
    
    const char* archivos[] = {
        "pacientes.bin", "doctores.bin", "citas.bin", "historiales.bin"
    };
    
    bool todosExitosos = true;
    
    for (int i = 0; i < 4; i++) {
        const char* archivo = archivos[i];
        
        if (!archivoExiste(archivo)) {
            cout << " - Creando: " << archivo;
            if (inicializarArchivo(archivo)) {
                cout << " [OK]" << endl;
            } else {
                cout << " [ERROR]" << endl;
                todosExitosos = false;
            }
        } else {
            cout << " - Verificando: " << archivo;
            if (verificarArchivo(archivo)) {
                cout << " [OK]" << endl;
            } else {
                cout << " [PROBLEMAS]" << endl;
                // Continuamos aunque tenga problemas
            }
        }
    }
    
    if (todosExitosos) {
        cout << "Sistema de archivos listo" << endl;
    } else {
        cout << "Algunos archivos tienen problemas" << endl;
    }
    cout << "==========================================" << endl;
    
    return todosExitosos;
}

long calcularPosicion(int indice) {
  return sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
}

int buscarIndiceCitaPorID(int idCita) {
    if (idCita <= 0) return -1;

    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) return -1;

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Buscar cita por ID
    Cita cita;
    int indice = 0;
    
    while (indice < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
        if (cita.id == idCita && !cita.eliminado) {
            archivo.close();
            return indice;
        }
        indice++;
    }

    archivo.close();
    return -1;
}
Paciente buscarPacientePorID(int id) {
  Paciente paciente;
  // Inicializar paciente vacío en caso de no encontrarlo
  memset(&paciente, 0, sizeof(Paciente));
  paciente.id = -1;  // Marcar como no encontrado

  //  Abrir pacientes.bin
  ifstream archivo("pacientes.bin", ios::binary);
  if (!archivo.is_open()) {
    cout << "? Error: No se puede abrir pacientes.bin" << endl;
    return paciente;
  }

  //  Leer header para saber cantidad de registros
  ArchivoHeader header;
  archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

  if (archivo.fail()) {
    cout << "? Error: No se puede leer header de pacientes.bin" << endl;
    archivo.close();
    return paciente;
  }

  // Verificar si hay registros
  if (header.cantidadRegistros == 0) {
    cout << "??  No hay pacientes registrados" << endl;
    archivo.close();
    return paciente;
  }

  //  Saltar header: seekg(sizeof(ArchivoHeader))
  archivo.seekg(sizeof(ArchivoHeader), ios::beg);

  // Bucle: leer cada paciente hasta encontrar ID o EOF
  bool encontrado = false;
  int pacientesLeidos = 0;

  while (pacientesLeidos < header.cantidadRegistros &&
         archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
    // Verificar si es el paciente buscado y está activo
    if (paciente.id == id && !paciente.eliminado) {
      encontrado = true;
      break;
    }

    pacientesLeidos++;
  }

  archivo.close();

  //  Retornar paciente encontrado (o paciente vacío si no existe)
  if (!encontrado) {
    memset(&paciente, 0, sizeof(Paciente));
    paciente.id = -1;
    cout << "? Paciente con ID " << id << " no encontrado" << endl;
  } else {
    cout << "? Paciente encontrado: " << paciente.nombre << " "
         << paciente.apellido << endl;
  }

  return paciente;
}

bool actualizarPaciente(Paciente pacienteModificado) {
  //  Buscar posición del paciente por ID
  int indice = buscarIndicePorID(pacienteModificado.id);
  if (indice == -1) {
    cout << "? Error: No se puede encontrar paciente con ID "
         << pacienteModificado.id << endl;
    return false;
  }

  //  Abrir archivo en modo lectura/escritura
  fstream archivo("pacientes.bin", ios::binary | ios::in | ios::out);
  if (!archivo.is_open()) {
    cout << "? Error: No se puede abrir pacientes.bin para actualización"
         << endl;
    return false;
  }

  //  Posicionarse en la ubicación exacta (seekp)
  long posicion = calcularPosicion(indice);
  archivo.seekp(posicion);

  if (archivo.fail()) {
    cout << "? Error: No se puede posicionar en archivo" << endl;
    archivo.close();
    return false;
  }

  //  Actualizar fechaModificacion
  pacienteModificado.fechaModificacion = time(nullptr);

  //  Escribir estructura completa sobre el registro existente
  archivo.write(reinterpret_cast<const char*>(&pacienteModificado),
                sizeof(Paciente));

  // Forzar escritura inmediata
  archivo.flush();

  bool exito = !archivo.fail();

  //  Cerrar archivo
  archivo.close();

  //  Retornar true si exitoso
  if (exito) {
    cout << "? Paciente ID " << pacienteModificado.id
         << " actualizado exitosamente" << endl;
    cout << "   ?? Última modificación: "
         << formatearFecha(pacienteModificado.fechaModificacion) << endl;
  } else {
    cout << "? Error al actualizar paciente ID " << pacienteModificado.id
         << endl;
  }

  return exito;
}


bool eliminarPaciente(int id, bool confirmar = true) {
  // Validaciones iniciales
  if (id <= 0) {
    cout << "? Error: ID de paciente inválido" << endl;
    return false;
  }

  // Buscar paciente para mostrar información
  Paciente paciente = buscarPacientePorID(id);
  if (paciente.id == -1) {
    cout << "? Error: Paciente ID " << id << " no existe" << endl;
    return false;
  }

  if (paciente.eliminado) {
    cout << "??  Paciente ID " << id << " ya está eliminado" << endl;
    return false;
  }

  // Confirmación de eliminación
  if (confirmar) {
    cout << "?? ELIMINACIÓN DE PACIENTE - CONFIRMACIÓN REQUERIDA" << endl;
    cout << "   ?? Datos del paciente:" << endl;
    cout << "   +-- ID: " << paciente.id << endl;
    cout << "   +-- Nombre: " << paciente.nombre << " " << paciente.apellido
         << endl;
    cout << "   +-- Cédula: " << paciente.cedula << endl;
    cout << "   +-- Edad: " << paciente.edad << endl;
    cout << "   +-- Consultas activas: " << paciente.cantidadConsultas << endl;

    cout << "? ¿Está seguro de eliminar este paciente? (s/n): ";
    char respuesta;
    cin >> respuesta;
    cin.ignore();  // Limpiar buffer

    if (tolower(respuesta) != 's') {
      cout << "? Eliminación cancelada por el usuario" << endl;
      return false;
    }

    // Doble confirmación si tiene consultas o citas
    if (paciente.cantidadConsultas > 0 || paciente.cantidadCitas > 0) {
      cout << "??  ADVERTENCIA: Este paciente tiene "
           << paciente.cantidadConsultas << " consultas y "
           << paciente.cantidadCitas << " citas" << endl;
      cout << "? ¿Continuar con la eliminación? (s/n): ";
      cin >> respuesta;
      cin.ignore();

      if (tolower(respuesta) != 's') {
        cout << "? Eliminación cancelada" << endl;
        return false;
      }
    }
  }

  // Proceder con eliminación lógica
  int indice = buscarIndicePorID(id);
  if (indice == -1) return false;

  fstream archivo("pacientes.bin", ios::binary | ios::in | ios::out);
  if (!archivo.is_open()) return false;

  long posicion = calcularPosicion(indice);

  // Leer y modificar
  archivo.seekg(posicion);
  archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente));

  paciente.eliminado = true;
  paciente.fechaModificacion = time(nullptr);

  archivo.seekp(posicion);
  archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
  archivo.flush();

  bool exito = archivo.good();
  archivo.close();

  if (exito) {
    // Actualizar header - CORREGIDO
    ArchivoHeader header = leerHeader("pacientes.bin");
    
    // Verificar si el header es válido (no es el header por defecto)
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
      header.registrosActivos = max(0, header.registrosActivos - 1);
      header.fechaUltimaModificacion = time(nullptr);
      actualizarHeader("pacientes.bin", header);
    }

    cout << "? ELIMINACIÓN COMPLETADA" << endl;
    cout << "  Paciente: " << paciente.nombre << " " << paciente.apellido
         << endl;
    cout << "  Eliminado: " << formatearFecha(paciente.fechaModificacion)
         << endl;

    // Mostrar impacto
    if (paciente.cantidadConsultas > 0) {
      cout << "    " << paciente.cantidadConsultas
           << " consultas en historial (no eliminadas)" << endl;
    }
    if (paciente.cantidadCitas > 0) {
      cout << "     " << paciente.cantidadCitas
           << " citas pendientes (no eliminadas)" << endl;
    }
  } else {
    cout << "? ERROR: No se pudo completar la eliminación" << endl;
  }

  return exito;
}

bool restaurarPaciente(int id) {
  int indice = buscarIndicePorID(id);
  if (indice == -1) {
    cout << " Error: No se puede encontrar paciente con ID " << id << endl;
    return false;
  }

  fstream archivo("pacientes.bin", ios::binary | ios::in | ios::out);
  if (!archivo.is_open()) return false;

  long posicion = calcularPosicion(indice);

  // Leer paciente
  Paciente paciente;
  archivo.seekg(posicion);
  archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente));

  if (!paciente.eliminado) {
    cout << " Paciente ID " << id << " no está eliminado" << endl;
    archivo.close();
    return false;
  }

  // Restaurar
  paciente.eliminado = false;
  paciente.fechaModificacion = time(nullptr);

  archivo.seekp(posicion);
  archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
  archivo.flush();

  bool exito = archivo.good();
  archivo.close();

  if (exito) {
    // Actualizar header - CORREGIDO
    ArchivoHeader header = leerHeader("pacientes.bin");
    
    // Verificar si el header es válido (no es el header por defecto)
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
      header.registrosActivos++;
      header.fechaUltimaModificacion = time(nullptr);
      actualizarHeader("pacientes.bin", header);

      cout << " PACIENTE RESTAURADO" << endl;
      cout << " Paciente ID " << id << " reactivado exitosamente" << endl;
      cout << " Registros activos: " << header.registrosActivos << endl;
    } else {
      cout << " PACIENTE RESTAURADO" << endl;
      cout << " Paciente ID " << id << " reactivado exitosamente" << endl;
      cout << " Nota: No se pudo actualizar el header del archivo" << endl;
    }
  }

  return exito;
}

int contarPacientesEliminados() {
  ArchivoHeader header = leerHeader("pacientes.bin");
  return header.cantidadRegistros - header.registrosActivos;
}

void listarPacientesEliminados() {
  ArchivoHeader header = leerHeader("pacientes.bin");

  if (header.registrosActivos == header.cantidadRegistros) {
    cout << " No hay pacientes eliminados" << endl;
    return;
  }

  cout << "  PACIENTES ELIMINADOS ("
       << (header.cantidadRegistros - header.registrosActivos) << "):" << endl;
  cout << "=========================================" << endl;

  ifstream archivo("pacientes.bin", ios::binary);
  if (!archivo.is_open()) return;

  archivo.seekg(sizeof(ArchivoHeader));

  Paciente p;
  int contador = 0;

  for (int i = 0; i < header.cantidadRegistros; i++) {
    archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente));
    if (p.eliminado) {
      cout << "ID: " << p.id << " | " << p.nombre << " " << p.apellido;

      char fechaStr[20];
      strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d",
               localtime(&p.fechaModificacion));
      cout << " | Eliminado: " << fechaStr << endl;

      contador++;
    }
  }

  archivo.close();
  cout << "=========================================" << endl;
  cout << "Total: " << contador << " pacientes eliminados" << endl;
}

HistorialMedico obtenerUltimaConsulta(int pacienteID) {
    HistorialMedico ultimaConsulta;
    // Inicializar con valores por defecto
    ultimaConsulta.id = -1;
    
    // Primero buscar el paciente para saber cuántas consultas tiene
    Paciente paciente = buscarPacientePorID(pacienteID);
    if (paciente.id == -1 || paciente.cantidadConsultas == 0) {
        return ultimaConsulta; // Retorna historial con id = -1
    }
    
    // Abrir archivo de historial médico
    ifstream archivo("historial_medico.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de historial médico" << endl;
        return ultimaConsulta;
    }
    
    // Buscar la última consulta del paciente
    HistorialMedico consulta;
    bool encontrada = false;
    int consultaIndex = 0;
    
    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (consulta.pacienteID == pacienteID && !consulta.eliminado) {
            consultaIndex++;
            // Si es la última consulta (basado en el orden en el archivo o fecha)
            if (consultaIndex == paciente.cantidadConsultas) {
                ultimaConsulta = consulta;
                encontrada = true;
                break;
            }
        }
    }
    
    archivo.close();
    
    if (!encontrada) {
        cout << "Error: No se pudo encontrar la última consulta del paciente" << endl;
        ultimaConsulta.id = -1;
    }
    
    return ultimaConsulta;
}

bool actualizarConsulta(HistorialMedico consulta) {
  
  fstream archivo("historiales.bin", ios::binary | ios::in | ios::out);
  if (!archivo.is_open()) return false;

  archivo.seekg(sizeof(ArchivoHeader));
  HistorialMedico temp;
  long posicion = sizeof(ArchivoHeader);

  while (
      archivo.read(reinterpret_cast<char*>(&temp), sizeof(HistorialMedico))) {
    if (temp.id == consulta.id) {
      archivo.seekp(posicion);
      archivo.write(reinterpret_cast<const char*>(&consulta),
                    sizeof(HistorialMedico));
      archivo.close();
      return true;
    }
    posicion = archivo.tellg();
  }

  archivo.close();
  return false;
}
bool agregarConsulta(HistorialMedico consulta) {
  ofstream archivo("historiales.bin", ios::binary | ios::app);
  if (!archivo.is_open()) return false;

  archivo.write(reinterpret_cast<const char*>(&consulta),
                sizeof(HistorialMedico));
  bool exito = !archivo.fail();
  archivo.close();

  if (exito) {
    // Actualizar header
    ArchivoHeader header = leerHeader("historiales.bin");
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    header.fechaUltimaModificacion = time(nullptr);
    actualizarHeader("historiales.bin", header);
  }

  return exito;
}

bool agregarConsultaAlHistorial(int pacienteID, HistorialMedico nuevaConsulta) {
  // Leer paciente de pacientes.bin
  Paciente paciente = buscarPacientePorID(pacienteID);
  if (paciente.id == -1) {
    cout << "Error: Paciente ID " << pacienteID << " no encontrado" << endl;
    return false;
  }

  if (paciente.eliminado) {
    cout << " Error: No se puede agregar consulta a paciente eliminado"
         << endl;
    return false;
  }

  // Asignar ID a la nueva consulta
  ArchivoHeader headerHistorial = leerHeader("historiales.bin");
  nuevaConsulta.id = headerHistorial.proximoID;
  nuevaConsulta.pacienteID = pacienteID;
  nuevaConsulta.eliminado = false;
  nuevaConsulta.fechaRegistro = time(nullptr);

  //  Si es su primera consulta:
  if (paciente.primerConsultaID == -1) {
    nuevaConsulta.siguienteConsultaID = -1;  // Será la única consulta
    paciente.primerConsultaID = nuevaConsulta.id;

    cout << "Primera consulta para el paciente" << endl;
  }
  // 3. Si ya tiene consultas:
  else {
    // Buscar última consulta recorriendo la lista enlazada
    HistorialMedico ultimaConsulta = obtenerUltimaConsulta(pacienteID);
    if (ultimaConsulta.id == -1) {
      cout << " Error: No se puede encontrar la última consulta" << endl;
      return false;
    }

    // Actualizar la última consulta para que apunte a la nueva
    ultimaConsulta.siguienteConsultaID = nuevaConsulta.id;
    if (!actualizarConsulta(ultimaConsulta)) {
      cout << " Error: No se puede actualizar última consulta" << endl;
      return false;
    }

    // La nueva consulta será la última
    nuevaConsulta.siguienteConsultaID = -1;

    cout << " Consulta #" << (paciente.cantidadConsultas + 1)
         << " para el paciente" << endl;
  }

  //  Agregar nuevaConsulta a historiales.bin
  if (!agregarConsulta(nuevaConsulta)) {
    cout << " Error: No se puede guardar la nueva consulta" << endl;
    return false;
  }

  //  Actualizar paciente.cantidadConsultas++
  paciente.cantidadConsultas++;
  paciente.fechaModificacion = time(nullptr);

  // Guardar paciente modificado
  if (!actualizarPaciente(paciente)) {
    cout << " Error: No se puede actualizar datos del paciente" << endl;
    return false;
  }

  cout << " CONSULTA AGREGADA EXITOSAMENTE" << endl;
  cout << "  Paciente: " << paciente.nombre << " " << paciente.apellido << endl;
  cout << "  Consulta ID: " << nuevaConsulta.id << endl;
  cout << "  Diagnóstico: " << nuevaConsulta.diagnostico << endl;
  cout << "  Costo: $" << nuevaConsulta.costo << endl;

  return true;
}

Paciente buscarPacientePorCedula(const char* cedula) {
    Paciente paciente;
    memset(&paciente, 0, sizeof(Paciente));
    paciente.id = -1;  

    if (!cedula || strlen(cedula) == 0) {
        cout << "Error: Cédula inválida" << endl;
        return paciente;
    }

    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de pacientes" << endl;
        return paciente;
    }

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    
    if (archivo.fail()) {
        cout << "Error: No se puede leer header de pacientes" << endl;
        archivo.close();
        return paciente;
    }

    // Buscar paciente por cédula
    bool encontrado = false;
    int pacientesLeidos = 0;

    while (pacientesLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        
        // Verificar si es el paciente buscado (misma cédula y no eliminado)
        if (strcmp(paciente.cedula, cedula) == 0 && !paciente.eliminado) {
            encontrado = true;
            break;
        }
        pacientesLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        memset(&paciente, 0, sizeof(Paciente));
        paciente.id = -1;
        cout << "Paciente con cédula " << cedula << " no encontrado" << endl;
    } else {
        cout << "Paciente encontrado: " << paciente.nombre << " " << paciente.apellido << endl;
    }

    return paciente;
}

int leerTodosPacientes(Paciente* pacientes, int maxPacientes,
                       bool soloActivos) {
  if (!pacientes || maxPacientes <= 0) return 0;

  ifstream archivo("pacientes.bin", ios::binary);
  if (!archivo.is_open()) return 0;

  // Leer header
  ArchivoHeader header;
  archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

  // Saltar header y leer pacientes
  archivo.seekg(sizeof(ArchivoHeader), ios::beg);

  int contador = 0;
  Paciente temp;

  while (contador < maxPacientes && contador < header.cantidadRegistros &&
         archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente))) {
    if (!soloActivos || !temp.eliminado) {
      pacientes[contador] = temp;
      contador++;
    }
  }

  archivo.close();
  return contador;
}


void mostrarResumenPaciente(const Paciente& paciente) {
  cout << "    RESUMEN DEL PACIENTE:" << endl;
  cout << "   +-- ID: " << paciente.id << endl;
  cout << "   +-- Nombre: " << paciente.nombre << " " << paciente.apellido
       << endl;
  cout << "   +-- Cédula: " << paciente.cedula << endl;
  cout << "   +-- Edad: " << paciente.edad << " años" << endl;
  cout << "   +-- Sexo: " << (paciente.sexo == 'M' ? "Masculino" : "Femenino")
       << endl;
  cout << "   +-- Teléfono: " << paciente.telefono << endl;

  // Mostrar fecha de creación formateada
  char fechaStr[20];
  strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d %H:%M",
           localtime(&paciente.fechaCreacion));
  cout << "    Registrado: " << fechaStr << endl;
}
void inicializarPaciente(Paciente& paciente) {
  paciente.eliminado = false;
  paciente.cantidadConsultas = 0;
  paciente.primerConsultaID = -1;
  paciente.cantidadCitas = 0;

  // Inicializar array de citas
  for (int i = 0; i < 20; i++) {
    paciente.citasIDs[i] = -1;
  }

  // Timestamps
  time_t ahora = time(nullptr);
  paciente.fechaCreacion = ahora;
  paciente.fechaModificacion = ahora;

  // Validar y ajustar campos
  if (paciente.edad < 0 || paciente.edad > 150) {
    cout << "??  Edad inválida (" << paciente.edad << "), ajustando a 0"
         << endl;
    paciente.edad = 0;
  }

  // Asegurar que sexo sea válido
  if (paciente.sexo != 'M' && paciente.sexo != 'F') {
    cout << " Sexo inválido (" << paciente.sexo << "), ajustando a 'M'" << endl;
    paciente.sexo = 'M';
  }
}

Paciente leerPacientePorIndice(int indice) {
  Paciente p;
  memset(&p, 0, sizeof(Paciente));
  p.id = -1;  // Marcador de error

  // Validar índice
  if (indice < 0) {
    cout << "? Error: Índice negativo (" << indice << ")" << endl;
    return p;
  }

  //  Abrir archivo
  ifstream archivo("pacientes.bin", ios::binary);
  if (!archivo.is_open()) {
    cout << " Error: No se puede abrir pacientes.bin" << endl;
    return p;
  }

  //  Leer header para verificar límites
  ArchivoHeader header;
  if (!archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader))) {
    cout << " Error: No se puede leer header" << endl;
    archivo.close();
    return p;
  }

  // Verificar que el índice esté dentro de los límites
  if (indice >= header.cantidadRegistros) {
    cout << " Error: Índice " << indice
         << " fuera de rango (máximo: " << header.cantidadRegistros - 1 << ")"
         << endl;
    archivo.close();
    return p;
  }

  //  Calcular posición exacta
  long posicion = calcularPosicion(indice);

  // Ir directamente a esa posición
  archivo.seekg(posicion);
  if (archivo.fail()) {
    cout << "? Error: No se puede posicionar en índice " << indice << endl;
    archivo.close();
    return p;
  }

  //  Leer estructura completa
  archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente));

  if (archivo.fail()) {
    cout << "Error: Fallo al leer paciente en índice " << indice << endl;
    memset(&p, 0, sizeof(Paciente));
    p.id = -1;
  } else if (p.eliminado) {
    cout << " Paciente en índice " << indice << " está eliminado" << endl;
    p.id = -2;  // Código especial para eliminado
  } else {
    cout << " Paciente leído por índice " << indice << ": " << p.nombre << " "
         << p.apellido << endl;
  }

  archivo.close();
  return p;
}
bool existePacienteConCedula(const char* cedula) {
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    
    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));
    
    Paciente paciente;
    while (archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        if (!paciente.eliminado && strcmp(paciente.cedula, cedula) == 0) {
            archivo.close();
            return true;
        }
    }
    
    archivo.close();
    return false;
}
bool agregarPaciente() {
    Paciente nuevoPaciente;
    
    // Inicializar paciente
    inicializarPaciente(nuevoPaciente);
    
    // Obtener próximo ID desde el header
    ArchivoHeader header = leerHeader("pacientes.bin");
    int proximoID = 1;
    
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
        proximoID = header.proximoID;
    }
    
    nuevoPaciente.id = proximoID;
    
    // Solicitar datos del paciente
    cout << "=== AGREGAR NUEVO PACIENTE ===" << endl;
    cout << "ID asignado: " << nuevoPaciente.id << endl;
    
    cout << "Nombre: ";
    cin.getline(nuevoPaciente.nombre, 50);
    
    cout << "Apellido: ";
    cin.getline(nuevoPaciente.apellido, 50);
    
    cout << "Cédula: ";
    cin.getline(nuevoPaciente.cedula, 20);
    
    cout << "Edad: ";
    cin >> nuevoPaciente.edad;
    cin.ignore();
    
    cout << "Sexo (M/F): ";
    cin >> nuevoPaciente.sexo;
    cin.ignore();
    
    cout << "Teléfono: ";
    cin.getline(nuevoPaciente.telefono, 20);
    
    cout << "Email: ";
    cin.getline(nuevoPaciente.email, 50);
    
    cout << "Dirección: ";
    cin.getline(nuevoPaciente.direccion, 100);
    
    // Validaciones adicionales
    if (strlen(nuevoPaciente.nombre) == 0 || strlen(nuevoPaciente.apellido) == 0) {
        cout << "Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }
    
    // Verificar si ya existe paciente con misma cédula
    if (existePacienteConCedula(nuevoPaciente.cedula)) {
        cout << "Error: Ya existe un paciente con la cédula " << nuevoPaciente.cedula << endl;
        return false;
    }
    
    // Guardar en archivo
    ofstream archivo("pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo de pacientes" << endl;
        return false;
    }
    
    // Si el archivo está vacío, escribir header primero
    archivo.seekp(0, ios::end);
    if (archivo.tellp() == 0) {
        // Archivo vacío, crear header
        ArchivoHeader nuevoHeader;
        strcpy(nuevoHeader.tipoArchivo, "PACIENTES");
        nuevoHeader.version = 1;
        nuevoHeader.cantidadRegistros = 1;
        nuevoHeader.registrosActivos = 1;
        nuevoHeader.proximoID = proximoID + 1;
        nuevoHeader.fechaCreacion = time(nullptr);
        nuevoHeader.fechaUltimaModificacion = nuevoHeader.fechaCreacion;
        
        archivo.seekp(0);
        archivo.write(reinterpret_cast<const char*>(&nuevoHeader), sizeof(ArchivoHeader));
    }
    
    // Escribir paciente
    archivo.write(reinterpret_cast<const char*>(&nuevoPaciente), sizeof(Paciente));
    bool exito = archivo.good();
    archivo.close();
    
    if (exito) {
        // Actualizar header
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.cantidadRegistros++;
            header.registrosActivos++;
            header.proximoID++;
            header.fechaUltimaModificacion = time(nullptr);
            actualizarHeader("pacientes.bin", header);
        }
        
        cout << "? PACIENTE AGREGADO EXITOSAMENTE" << endl;
        cout << "  ID: " << nuevoPaciente.id << endl;
        cout << "  Nombre: " << nuevoPaciente.nombre << " " << nuevoPaciente.apellido << endl;
        cout << "  Cédula: " << nuevoPaciente.cedula << endl;
        cout << "  Fecha: " << formatearFecha(nuevoPaciente.fechaCreacion) << endl;
    } else {
        cout << "? ERROR: No se pudo agregar el paciente" << endl;
    }
    
    return exito;
}


Paciente leerPacientePorID(int id) { return buscarPacientePorID(id); }

int buscarPacientesPorNombre(const char* nombre, Paciente* resultados,
                             int maxResultados) {
  if (!nombre || strlen(nombre) == 0 || !resultados || maxResultados <= 0) {
    cout << " Error: Parámetros de búsqueda inválidos" << endl;
    return 0;
  }

  //  Leer todos los pacientes activos
  const int MAX_PACIENTES = 1000;
  Paciente todosPacientes[MAX_PACIENTES];
  int cantidadTotal = leerTodosPacientes(todosPacientes, MAX_PACIENTES, true);

  if (cantidadTotal == 0) {
    cout << " No hay pacientes registrados" << endl;
    return 0;
  }


  char* nombreLower = new char[strlen(nombre) + 1];
  for (int i = 0; nombre[i]; i++) {
    nombreLower[i] = tolower(nombre[i]);
  }
  nombreLower[strlen(nombre)] = '\0';

  // Realizar búsqueda
  int encontrados = 0;

  for (int i = 0; i < cantidadTotal && encontrados < maxResultados; i++) {
    Paciente* p = &todosPacientes[i];

    if (p->eliminado) continue;

    // Convertir nombre del paciente a minúsculas
    char nombrePacienteLower[50];
    strcpy(nombrePacienteLower, p->nombre);
    for (int j = 0; nombrePacienteLower[j]; j++) {
      nombrePacienteLower[j] = tolower(nombrePacienteLower[j]);
    }

    // Buscar coincidencia parcial
    if (strstr(nombrePacienteLower, nombreLower) != nullptr) {
      resultados[encontrados] = *p;
      encontrados++;
    }
  }

  delete[] nombreLower;

  // Mostrar resultados
  if (encontrados > 0) {
    cout << " Encontrados " << encontrados << " pacientes con '" << nombre
         << "'" << endl;
  } else {
    cout << " No se encontraron pacientes con '" << nombre << "'" << endl;
  }

  return encontrados;
}
void liberarResultadosBusqueda(Paciente** resultados) {
  if (resultados) {
    delete[] resultados;
  }
}

bool crearPaciente(const char* nombre, const char* apellido, const char* cedula, int edad, char sexo) {
    // Validaciones iniciales
    if (strlen(nombre) == 0 || strlen(apellido) == 0) {
        cout << "Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }

    if (strlen(cedula) == 0) {
        cout << "Error: Cédula es obligatoria" << endl;
        return false;
    }

    // Verificar que no exista paciente con misma cédula
    Paciente existente = buscarPacientePorCedula(cedula);
    if (existente.id != -1) {
        cout << "Error: Ya existe un paciente con cédula " << cedula << endl;
        return false;
    }

    // Validar edad
    if (edad < 0 || edad > 150) {
        cout << "Error: Edad inválida. Debe estar entre 0 y 150 años" << endl;
        return false;
    }

    // Validar sexo
    if (sexo != 'M' && sexo != 'F') {
        cout << "Error: Sexo inválido. Use 'M' o 'F'" << endl;
        return false;
    }

    // Leer header actual
    ArchivoHeader header = leerHeader("pacientes.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "Error: No se puede leer header de pacientes.bin" << endl;
        return false;
    }

    // Crear estructura Paciente
    Paciente nuevoPaciente;
    memset(&nuevoPaciente, 0, sizeof(Paciente));

    // Asignar ID
    nuevoPaciente.id = header.proximoID;

    // Copiar datos básicos
    strncpy(nuevoPaciente.nombre, nombre, sizeof(nuevoPaciente.nombre) - 1);
    strncpy(nuevoPaciente.apellido, apellido, sizeof(nuevoPaciente.apellido) - 1);
    strncpy(nuevoPaciente.cedula, cedula, sizeof(nuevoPaciente.cedula) - 1);
    nuevoPaciente.edad = edad;
    nuevoPaciente.sexo = sexo;

    // Inicializar campos del nuevo paciente
    nuevoPaciente.eliminado = false;
    nuevoPaciente.cantidadConsultas = 0;
    nuevoPaciente.primerConsultaID = -1;
    nuevoPaciente.cantidadCitas = 0;

    // Inicializar array de citas
    for (int i = 0; i < 20; i++) {
        nuevoPaciente.citasIDs[i] = -1;
    }

    // Timestamps
    time_t ahora = time(nullptr);
    nuevoPaciente.fechaCreacion = ahora;
    nuevoPaciente.fechaModificacion = ahora;

    // Campos opcionales vacíos
    strcpy(nuevoPaciente.tipoSangre, "");
    strcpy(nuevoPaciente.telefono, "");
    strcpy(nuevoPaciente.direccion, "");
    strcpy(nuevoPaciente.email, "");
    strcpy(nuevoPaciente.alergias, "");
    strcpy(nuevoPaciente.observaciones, "");
    nuevoPaciente.activo = true;

    // Guardar en archivo
    ofstream archivo("pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir pacientes.bin en modo append" << endl;
        return false;
    }

    // Si el archivo está vacío, escribir header primero
    archivo.seekp(0, ios::end);
    if (archivo.tellp() == 0) {
        // Archivo vacío, crear header nuevo
        ArchivoHeader nuevoHeader;
        strcpy(nuevoHeader.tipoArchivo, "PACIENTES");
        nuevoHeader.version = 1;
        nuevoHeader.cantidadRegistros = 1;
        nuevoHeader.registrosActivos = 1;
        nuevoHeader.proximoID = nuevoPaciente.id + 1;
        nuevoHeader.fechaCreacion = time(nullptr);
        nuevoHeader.fechaUltimaModificacion = nuevoHeader.fechaCreacion;
        
        archivo.seekp(0);
        archivo.write(reinterpret_cast<const char*>(&nuevoHeader), sizeof(ArchivoHeader));
    }

    archivo.write(reinterpret_cast<const char*>(&nuevoPaciente), sizeof(Paciente));
    bool exitoEscritura = archivo.good();
    archivo.close();

    if (!exitoEscritura) {
        cout << "Error crítico: Fallo en escritura de paciente" << endl;
        return false;
    }

    // Actualizar header 
    header = leerHeader("pacientes.bin"); // Leer header actualizado
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
        header.cantidadRegistros++;
        header.proximoID++;
        header.registrosActivos++;
        header.fechaUltimaModificacion = time(nullptr);

        // Actualizar header en archivo
        if (actualizarHeader("pacientes.bin", header)) {
            cout << "PACIENTE CREADO EXITOSAMENTE" << endl;
            cout << "ID: " << nuevoPaciente.id << endl;
            cout << "Nombre: " << nuevoPaciente.nombre << " " << nuevoPaciente.apellido << endl;
            cout << "Cédula: " << nuevoPaciente.cedula << endl;
            cout << "Edad: " << nuevoPaciente.edad << " años" << endl;
            cout << "Sexo: " << (nuevoPaciente.sexo == 'M' ? "Masculino" : "Femenino") << endl;
            
            // Mostrar fecha de creación formateada
            char fechaStr[20];
            strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d %H:%M", localtime(&nuevoPaciente.fechaCreacion));
            cout << "Registrado: " << fechaStr << endl;
            
            return true;
        }
    }

    cout << "ERROR: Paciente escrito pero header corrupto" << endl;
    return false;
}

bool actualizarPacienteEnArchivo(const Paciente& paciente) {
    fstream archivo("pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        return false;
    }
    
    // Buscar la posición del paciente en el archivo
    int indice = buscarIndicePorID(paciente.id);
    if (indice == -1) {
        archivo.close();
        return false;
    }
    
    // Calcular posición en el archivo
    long posicion = sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
    
    // Escribir el paciente actualizado
    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
    archivo.flush();
    
    bool exito = archivo.good();
    archivo.close();
    
    // Actualizar header si fue exitoso
    if (exito) {
        ArchivoHeader header = leerHeader("pacientes.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.fechaUltimaModificacion = time(nullptr);
            actualizarHeader("pacientes.bin", header);
        }
    }
    
    return exito;
}
bool actualizarPaciente(int id) {
    // Buscar paciente en el archivo
    Paciente paciente = buscarPacientePorID(id);
    
    if (paciente.id == -1) {
        cout << "Error: No se encontró paciente con ID " << id << endl;
        return false;
    }
    
    if (paciente.eliminado) {
        cout << "Error: No se puede actualizar un paciente eliminado" << endl;
        return false;
    }
    
    // Mostrar datos actuales
    cout << "=== ACTUALIZAR PACIENTE ===" << endl;
    cout << "ID: " << paciente.id << " (no modificable)" << endl;
    cout << "Datos actuales:" << endl;
    cout << "Nombre: " << paciente.nombre << " " << paciente.apellido << endl;
    cout << "Cédula: " << paciente.cedula << endl;
    cout << "Edad: " << paciente.edad << endl;
    cout << "Sexo: " << paciente.sexo << endl;
    cout << "Teléfono: " << paciente.telefono << endl;
    cout << "Email: " << paciente.email << endl;
    cout << "Dirección: " << paciente.direccion << endl;
    cout << "--------------------------------" << endl;
    
    // Solicitar nuevos datos
    cout << "Nuevos datos (presione Enter para mantener valor actual):" << endl;
    
    cout << "Nombre (" << paciente.nombre << "): ";
    char input[100];
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        strcpy(paciente.nombre, input);
    }
    
    cout << "Apellido (" << paciente.apellido << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        strcpy(paciente.apellido, input);
    }
    
    cout << "Cédula (" << paciente.cedula << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        strcpy(paciente.cedula, input);
    }
    
    cout << "Edad (" << paciente.edad << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        int nuevaEdad = atoi(input);
        if (nuevaEdad >= 0 && nuevaEdad <= 120) {
            paciente.edad = nuevaEdad;
        } else {
            cout << "?? Edad inválida, manteniendo valor actual" << endl;
        }
    }
    
    cout << "Sexo (" << paciente.sexo << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        char nuevoSexo = toupper(input[0]);
        if (nuevoSexo == 'M' || nuevoSexo == 'F') {
            paciente.sexo = nuevoSexo;
        } else {
            cout << "?? Sexo inválido, manteniendo valor actual" << endl;
        }
    }
    
    cout << "Teléfono (" << paciente.telefono << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        strcpy(paciente.telefono, input);
    }
    
    cout << "Email (" << paciente.email << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        strcpy(paciente.email, input);
    }
    
    cout << "Dirección (" << paciente.direccion << "): ";
    cin.getline(input, 100);
    if (strlen(input) > 0) {
        strcpy(paciente.direccion, input);
    }
    
    // Actualizar timestamp
    paciente.fechaModificacion = time(nullptr);
    
    // Guardar en archivo
    bool exito = actualizarPacienteEnArchivo(paciente);
    
    if (exito) {
        cout << "? Paciente actualizado exitosamente" << endl;
        cout << "? Fecha de modificación: " << formatearFecha(paciente.fechaModificacion) << endl;
    } else {
        cout << "? Error: No se pudo actualizar el paciente" << endl;
    }
    
    return exito;
}



HistorialMedico buscarUltimaConsulta(int pacienteID) {
  Paciente paciente = buscarPacientePorID(pacienteID);
  if (paciente.id == -1 || paciente.primerConsultaID == -1) {
    return crearConsultaVacia();
  }

  HistorialMedico actual = buscarConsultaPorID(paciente.primerConsultaID);
  HistorialMedico ultima = actual;

  // Recorrer lista enlazada hasta encontrar la última
  while (actual.siguienteConsultaID != -1) {
    actual = buscarConsultaPorID(actual.siguienteConsultaID);
    if (actual.id == -1) break;  // Error en la lista
    ultima = actual;
  }

  return ultima;
}


bool agregarPaciente(Paciente nuevoPaciente) {
    // Validaciones iniciales
    if (strlen(nuevoPaciente.nombre) == 0 || strlen(nuevoPaciente.apellido) == 0) {
        cout << "? Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }

    if (strlen(nuevoPaciente.cedula) == 0) {
        cout << "Error: La Cedula es obligatoria" << endl;
        return false;
    }

    // Verificar que no exista paciente con misma cédula
    Paciente existente = buscarPacientePorCedula(nuevoPaciente.cedula);
    if (existente.id != -1) {
        cout << "Error: Ya existe un paciente con esa cedula " << nuevoPaciente.cedula << endl;
        return false;
    }

    // Leer header actual
    ArchivoHeader header = leerHeader("pacientes.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "? Error: No se puede leer header de pacientes.bin" << endl;
        return false;
    }

    // Asignar ID y preparar paciente
    nuevoPaciente.id = header.proximoID;
    inicializarPaciente(nuevoPaciente);

    // Abrir en modo append
    ofstream archivo("pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "? Error: No se puede abrir pacientes.bin en modo append" << endl;
        return false;
    }

    // Si el archivo está vacío, escribir header primero
    archivo.seekp(0, ios::end);
    if (archivo.tellp() == 0) {
        // Archivo vacío, crear header nuevo
        ArchivoHeader nuevoHeader;
        strcpy(nuevoHeader.tipoArchivo, "PACIENTES");
        nuevoHeader.version = 1;
        nuevoHeader.cantidadRegistros = 1;
        nuevoHeader.registrosActivos = 1;
        nuevoHeader.proximoID = nuevoPaciente.id + 1;
        nuevoHeader.fechaCreacion = time(nullptr);
        nuevoHeader.fechaUltimaModificacion = nuevoHeader.fechaCreacion;
        
        archivo.seekp(0);
        archivo.write(reinterpret_cast<const char*>(&nuevoHeader), sizeof(ArchivoHeader));
    }

    // Escribir registro
    archivo.write(reinterpret_cast<const char*>(&nuevoPaciente), sizeof(Paciente));
    bool exitoEscritura = archivo.good();
    archivo.close();

    if (!exitoEscritura) {
        cout << "? Error crítico: Fallo en escritura de paciente" << endl;
        return false;
    }

    // Actualizar header - CORREGIDO
    header = leerHeader("pacientes.bin"); // Leer header actualizado
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
        header.cantidadRegistros++;
        header.proximoID++;
        header.registrosActivos++;
        header.fechaUltimaModificacion = time(nullptr);

        // Actualizar header en archivo
        if (actualizarHeader("pacientes.bin", header)) {
            cout << "? PACIENTE AGREGADO EXITOSAMENTE" << endl;
            cout << "  ID: " << nuevoPaciente.id << endl;
            cout << "  Nombre: " << nuevoPaciente.nombre << " " << nuevoPaciente.apellido << endl;
            cout << "  Cédula: " << nuevoPaciente.cedula << endl;
            cout << "  Fecha: " << formatearFecha(nuevoPaciente.fechaCreacion) << endl;
            return true;
        }
    }

    cout << "? ERROR: Paciente escrito pero header corrupto" << endl;
    return false;
}


void listarPacientes() {
    // Leer header para saber cuántos pacientes hay
    ArchivoHeader header = leerHeader("pacientes.bin");
    
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0 || header.registrosActivos == 0) {
        cout << "No hay pacientes registrados." << endl;
        return;
    }

    cout << "\nLISTA DE PACIENTES (" << header.registrosActivos << "):" << endl;
    cout << "+-------------------------------------------------------------------+" << endl;
    cout << "|  ID  |       NOMBRE COMPLETO    |    CÉDULA    | EDAD | CONSULTAS  |" << endl;
    cout << "+------+--------------------------+--------------+------+------------+" << endl;

    // Abrir archivo y leer pacientes
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de pacientes" << endl;
        return;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Paciente p;
    int pacientesMostrados = 0;

    while (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && pacientesMostrados < header.registrosActivos) {
        // Solo mostrar pacientes no eliminados
        if (!p.eliminado) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", p.nombre, p.apellido);

            // Truncar nombre si es muy largo
            if (strlen(nombreCompleto) > 22) {
                nombreCompleto[19] = '.';
                nombreCompleto[20] = '.';
                nombreCompleto[21] = '.';
                nombreCompleto[22] = '\0';
            }

            printf("| %4d | %-24s | %-12s | %4d | %10d |\n", 
                   p.id, nombreCompleto, p.cedula, p.edad, p.cantidadConsultas);
            
            pacientesMostrados++;
        }
    }

    archivo.close();

    cout << "+-------------------------------------------------------------------+" << endl;
    
    // Mostrar estadísticas adicionales
    if (pacientesMostrados > 0) {
        cout << "Total mostrados: " << pacientesMostrados << " paciente(s)" << endl;
        cout << "Registros en archivo: " << header.cantidadRegistros << endl;
    }
}


HistorialMedico* obtenerHistorialCompleto(int pacienteID, int* cantidad) {
    *cantidad = 0; // Inicializar a 0
    
    if (pacienteID <= 0) {
        return nullptr;
    }

    // Primero verificar si el paciente existe y tiene consultas
    Paciente paciente = buscarPacientePorID(pacienteID);
    if (paciente.id == -1 || paciente.cantidadConsultas == 0) {
        return nullptr;
    }

    // Abrir archivo de historial médico
    ifstream archivo("historial_medico.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de historial médico" << endl;
        return nullptr;
    }

    // Contar cuántas consultas tiene este paciente
    HistorialMedico consulta;
    int totalConsultas = 0;
    
    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
        if (consulta.pacienteID == pacienteID && !consulta.eliminado) {
            totalConsultas++;
        }
    }

    if (totalConsultas == 0) {
        archivo.close();
        return nullptr;
    }

    // Reservar memoria para el array de historial
    HistorialMedico* historial = new HistorialMedico[totalConsultas];
    if (!historial) {
        cout << "Error: No se pudo asignar memoria para el historial" << endl;
        archivo.close();
        return nullptr;
    }

    // Volver a leer y guardar las consultas
    archivo.clear();
    archivo.seekg(0, ios::beg);
    
    int index = 0;
    while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico)) && index < totalConsultas) {
        if (consulta.pacienteID == pacienteID && !consulta.eliminado) {
            historial[index] = consulta;
            index++;
        }
    }

    archivo.close();
    *cantidad = totalConsultas;
    
    return historial;
}
int leerTodosDoctores(Doctor* doctores, int maxDoctores, bool soloDisponibles) {
    // Validar parámetros
    if (doctores == nullptr || maxDoctores <= 0) {
        return 0;
    }

    // Abrir archivo
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de doctores" << endl;
        return 0;
    }

    // Leer header usando la función corregida
    ArchivoHeader header = leerHeader("doctores.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        archivo.close();
        return 0;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    int contador = 0;
    Doctor temp;

    while (contador < maxDoctores && 
           archivo.read(reinterpret_cast<char*>(&temp), sizeof(Doctor))) {
        
        // Usar campo 'disponible' para filtrar (CORREGIDO)
        if (!soloDisponibles || temp.disponible) {
            doctores[contador] = temp;
            contador++;
        }
    }

    archivo.close();
    return contador;
}
void mostrarHistorialMedico(int pacienteID) {
    if (pacienteID <= 0) {
        cout << "Error: ID de paciente no válido." << endl;
        return;
    }

    // Buscar paciente en archivo
    Paciente paciente = buscarPacientePorID(pacienteID);
    if (paciente.id == -1) {
        cout << "Error: Paciente no encontrado." << endl;
        return;
    }

    if (paciente.eliminado) {
        cout << "Error: No se puede mostrar historial de paciente eliminado." << endl;
        return;
    }

    // Obtener historial usando la versión con array
    int cantidad;
    HistorialMedico* historial = obtenerHistorialCompleto(pacienteID, &cantidad);

    if (!historial || cantidad == 0) {
        cout << "El paciente no tiene consultas en su historial médico." << endl;
        if (historial) liberarHistorial(historial);
        return;
    }

    cout << "\n=== HISTORIAL MÉDICO ===" << endl;
    cout << "Paciente: " << paciente.nombre << " " << paciente.apellido << endl;
    cout << "Cédula: " << paciente.cedula << " | Edad: " << paciente.edad << endl;
    cout << "Total de consultas: " << cantidad << endl;
    cout << endl;

    cout << left << setw(12) << "FECHA" << setw(8) << "HORA" << setw(25)
         << "DIAGNÓSTICO" << setw(8) << "DOCTOR" << setw(10) << "COSTO" << endl;

    cout << string(65, '-') << endl;

    for (int i = 0; i < cantidad; i++) {
        char diagnosticoMostrar[26];
        strcpy(diagnosticoMostrar, historial[i].diagnostico);
        if (strlen(diagnosticoMostrar) > 23) {
            diagnosticoMostrar[20] = '.';
            diagnosticoMostrar[21] = '.';
            diagnosticoMostrar[22] = '.';
            diagnosticoMostrar[23] = '\0';
        }

        cout << left << setw(12) << historial[i].fecha << setw(8)
             << historial[i].hora << setw(25) << diagnosticoMostrar << setw(8)
             << historial[i].idDoctor << "$" << setw(9) << fixed << setprecision(2)
             << historial[i].costo << endl;
    }

    cout << string(65, '-') << endl;

    float costoTotal = 0;
    for (int i = 0; i < cantidad; i++) {
        costoTotal += historial[i].costo;
    }
    cout << "Costo total del historial: $" << fixed << setprecision(2)
         << costoTotal << endl;

    // IMPORTANTE: Liberar memoria
    liberarHistorial(historial);
}


// MODULO DE DOCTORES


void inicializarDoctor(Doctor& doctor) {
  doctor.disponible = true;
  doctor.eliminado = false;
  doctor.cantidadPacientes = 0;
  doctor.cantidadCitas = 0;

  // Inicializar arrays
  for (int i = 0; i < 50; i++) {
    doctor.pacientesIDs[i] = -1;
  }
  for (int i = 0; i < 30; i++) {
    doctor.citasIDs[i] = -1;
  }

  // Timestamps
  time_t ahora = time(nullptr);
  doctor.fechaCreacion = ahora;
  doctor.fechaModificacion = ahora;

  // Valores por defecto
  strcpy(doctor.horarioAtencion, "L-V 8:00-17:00");
  strcpy(doctor.telefono, "");
  strcpy(doctor.email, "");
}

Doctor buscarDoctorPorCedula(const char* cedula) {
    Doctor doctor;
    memset(&doctor, 0, sizeof(Doctor));
    doctor.id = -1;  // Marcador de no encontrado

    // Validaciones
    if (!cedula || strlen(cedula) == 0) {
        cout << "Error: Cédula inválida" << endl;
        return doctor;
    }

    // Abrir archivo de doctores
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo de doctores" << endl;
        return doctor;
    }

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    
    if (archivo.fail()) {
        cout << "Error: No se puede leer header de doctores" << endl;
        archivo.close();
        return doctor;
    }

    // Buscar doctor por cédula
    bool encontrado = false;
    int doctoresLeidos = 0;

    while (doctoresLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor))) {
        
        // Verificar si es el doctor buscado (misma cédula y no eliminado)
        if (strcmp(doctor.cedula, cedula) == 0 && !doctor.eliminado) {
            encontrado = true;
            break;
        }
        doctoresLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        memset(&doctor, 0, sizeof(Doctor));
        doctor.id = -1;
        cout << "Doctor con cédula " << cedula << " no encontrado" << endl;
    } else {
        cout << "Doctor encontrado: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
    }

    return doctor;
}

bool crearDoctor(const char* nombre, const char* apellido, const char* cedula,
                 const char* especialidad, int aniosExperiencia,
                 float costoConsulta) {
    // Validaciones basicas
    if (!nombre || strlen(nombre) == 0 || !apellido || strlen(apellido) == 0) {
        cout << "Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }

    if (!cedula || strlen(cedula) == 0) {
        cout << "Error: Cedula es obligatoria" << endl;
        return false;
    }

    if (!especialidad || strlen(especialidad) == 0) {
        cout << "Error: Especialidad es obligatoria" << endl;
        return false;
    }

    if (aniosExperiencia < 0) {
        cout << "Error: Anos de experiencia no pueden ser negativos" << endl;
        return false;
    }

    if (costoConsulta < 0) {
        cout << "Error: Costo de consulta no puede ser negativo" << endl;
        return false;
    }

    // Verificar que no exista doctor con misma cedula
    Doctor existente = buscarDoctorPorCedula(cedula);
    if (existente.id != -1) {
        cout << "Error: Ya existe un doctor con cedula " << cedula << endl;
        return false;
    }

    // Leer header actual - CORREGIDO
    ArchivoHeader header = leerHeader("doctores.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "Error: No se puede leer header de doctores.bin" << endl;
        return false;
    }

    // Crear estructura Doctor
    Doctor doctor;
    memset(&doctor, 0, sizeof(Doctor));

    // Asignar ID
    doctor.id = header.proximoID;

    // Copiar datos basicos
    strncpy(doctor.nombre, nombre, sizeof(doctor.nombre) - 1);
    strncpy(doctor.apellido, apellido, sizeof(doctor.apellido) - 1);
    strncpy(doctor.cedula, cedula, sizeof(doctor.cedula) - 1); // Cambié a cedula
    strncpy(doctor.especialidad, especialidad, sizeof(doctor.especialidad) - 1);

    // Asignar datos numericos
    doctor.aniosExperiencia = aniosExperiencia;
    doctor.costoConsulta = costoConsulta;

    // Inicializar campos del nuevo doctor
    inicializarDoctor(doctor);

    // Guardar en archivo
    ofstream archivo("doctores.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir doctores.bin en modo append" << endl;
        return false;
    }

    // Si el archivo está vacío, escribir header primero
    archivo.seekp(0, ios::end);
    if (archivo.tellp() == 0) {
        // Archivo vacío, crear header nuevo
        ArchivoHeader nuevoHeader;
        strcpy(nuevoHeader.tipoArchivo, "DOCTORES");
        nuevoHeader.version = 1;
        nuevoHeader.cantidadRegistros = 1;
        nuevoHeader.registrosActivos = 1;
        nuevoHeader.proximoID = doctor.id + 1;
        nuevoHeader.fechaCreacion = time(nullptr);
        nuevoHeader.fechaUltimaModificacion = nuevoHeader.fechaCreacion;
        
        archivo.seekp(0);
        archivo.write(reinterpret_cast<const char*>(&nuevoHeader), sizeof(ArchivoHeader));
    }

    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    bool exitoEscritura = archivo.good();
    archivo.close();

    if (!exitoEscritura) {
        cout << "Error crítico: Fallo en escritura de doctor" << endl;
        return false;
    }

    // Actualizar header - CORREGIDO
    header = leerHeader("doctores.bin"); // Leer header actualizado
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
        header.cantidadRegistros++;
        header.proximoID++;
        header.registrosActivos++;
        header.fechaUltimaModificacion = time(nullptr);

        // Actualizar header en archivo
        if (actualizarHeader("doctores.bin", header)) {
            cout << "DOCTOR CREADO EXITOSAMENTE" << endl;
            cout << "ID: " << doctor.id << endl;
            cout << "Nombre: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
            cout << "Especialidad: " << doctor.especialidad << endl;
            cout << "Costo consulta: $" << doctor.costoConsulta << endl;
            cout << "Experiencia: " << doctor.aniosExperiencia << " anos" << endl;
            cout << "Cedula: " << doctor.cedula << endl;
            return true;
        }
    }

    cout << "ERROR: Doctor escrito pero header corrupto" << endl;
    return false;
}
Doctor buscarDoctorPorID(int id) {
  Doctor doctor;

  // Inicializar doctor vacio en caso de no encontrarlo
  memset(&doctor, 0, sizeof(Doctor));
  doctor.id = -1;  // Marcar como no encontrado

  // Validar ID
  if (id <= 0) {
    cout << "Error: ID de doctor invalido" << endl;
    return doctor;
  }

  //  Abrir doctores.bin
  ifstream archivo("doctores.bin", ios::binary);
  if (!archivo.is_open()) {
    cout << "Error: No se puede abrir doctores.bin" << endl;
    return doctor;
  }

  //  Leer header para saber cantidad de registros
  ArchivoHeader header;
  archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

  if (archivo.fail()) {
    cout << "Error: No se puede leer header de doctores.bin" << endl;
    archivo.close();
    return doctor;
  }

  // Verificar si hay registros
  if (header.cantidadRegistros == 0) {
    cout << "No hay doctores registrados" << endl;
    archivo.close();
    return doctor;
  }

  // Verificar si el ID podria existir
  if (id >= header.proximoID) {
    cout << "Error: ID " << id
         << " fuera de rango (maximo: " << header.proximoID - 1 << ")" << endl;
    archivo.close();
    return doctor;
  }

  // Saltar header
  archivo.seekg(sizeof(ArchivoHeader), ios::beg);

  // Buscar doctor por ID
  bool encontrado = false;
  int doctoresLeidos = 0;

  while (doctoresLeidos < header.cantidadRegistros &&
         archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor))) {
    // Verificar si es el doctor buscado y no esta eliminado
    if (doctor.id == id && !doctor.eliminado) {
      encontrado = true;
      break;
    }

    doctoresLeidos++;
  }

  archivo.close();

  if (!encontrado) {
    memset(&doctor, 0, sizeof(Doctor));
    doctor.id = -1;
    cout << "Doctor con ID " << id << " no encontrado" << endl;
  } else {
    cout << "Doctor encontrado: Dr. " << doctor.nombre << " " << doctor.apellido
         << endl;
  }

  return doctor;
}

int buscarDoctoresPorEspecialidad(const char* especialidad, Doctor* resultados,
                                  int maxResultados) {
  if (!especialidad || strlen(especialidad) == 0 || !resultados ||
      maxResultados <= 0) {
    cout << "Error: Parametros de busqueda invalidos" << endl;
    return 0;
  }

  const int MAX_DOCTORES = 100;
  Doctor todosDoctores[MAX_DOCTORES];
  int cantidadTotal = leerTodosDoctores(todosDoctores, MAX_DOCTORES, true);

  if (cantidadTotal == 0) {
    cout << "No hay doctores registrados" << endl;
    return 0;
  }

  int encontrados = 0;

  for (int i = 0; i < cantidadTotal && encontrados < maxResultados; i++) {
    Doctor* d = &todosDoctores[i];

    if (strcmp(d->especialidad, especialidad) == 0) {
      resultados[encontrados] = *d;
      encontrados++;
    }
  }

  if (encontrados > 0) {
    cout << "Encontrados " << encontrados << " doctores de " << especialidad
         << endl;
  } else {
    cout << "No se encontraron doctores de " << especialidad << endl;
  }

  return encontrados;
}

int buscarIndiceDoctorPorID(int id) {
  if (id <= 0) return -1;

  ifstream archivo("doctores.bin", ios::binary);
  if (!archivo.is_open()) return -1;

  // Leer header
  ArchivoHeader header;
  archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

  // Busqueda secuencial por indice
  Doctor d;
  int indice = 0;

  while (indice < header.cantidadRegistros &&
         archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor))) {
    if (d.id == id) {
      archivo.close();
      return indice;
    }
    indice++;
  }

  archivo.close();
  return -1;
}

long calcularPosicionDoctor(int indice) {
  return sizeof(ArchivoHeader) + (indice * sizeof(Doctor));
}


bool actualizarDoctor(Doctor doctorModificado) {
  if (doctorModificado.id <= 0) {
    cout << "Error: ID de doctor invalido" << endl;
    return false;
  }

  if (strlen(doctorModificado.nombre) == 0 ||
      strlen(doctorModificado.apellido) == 0) {
    cout << "Error: Nombre y apellido son obligatorios" << endl;
    return false;
  }

  if (strlen(doctorModificado.cedula) == 0) {
    cout << "Error: Cedula es obligatoria" << endl;
    return false;
  }

  Doctor doctorActual = buscarDoctorPorID(doctorModificado.id);
  if (doctorActual.id == -1) {
    cout << "Error: Doctor ID " << doctorModificado.id << " no encontrado"
         << endl;
    return false;
  }

  if (doctorActual.eliminado) {
    cout << "Error: No se puede actualizar doctor eliminado" << endl;
    return false;
  }

  //  Verificar que la cedula no este duplicada (si cambio)
  if (strcmp(doctorActual.cedula,
             doctorModificado.cedula) != 0) {
    Doctor existente =
        buscarDoctorPorCedula(doctorModificado.cedula);
    if (existente.id != -1) {
      cout << "Error: Ya existe otro doctor con cedula "
           << doctorModificado.cedula << endl;
      return false;
    }
  }

  //  Abrir archivo en modo lectura/escritura
  fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
  if (!archivo.is_open()) {
    cout << "Error: No se puede abrir doctores.bin para actualizacion" << endl;
    return false;
  }

  //  Buscar posicion del doctor
  int indice = buscarIndiceDoctorPorID(doctorModificado.id);
  if (indice == -1) {
    cout << "Error: No se puede encontrar indice del doctor" << endl;
    archivo.close();
    return false;
  }

  //  Posicionarse en el registro
  long posicion = calcularPosicionDoctor(indice);
  archivo.seekg(posicion);

  //  Leer doctor actual para preservar algunos campos
  Doctor temp;
  archivo.read(reinterpret_cast<char*>(&temp), sizeof(Doctor));

  //  Preservar campos que no deben cambiar
  doctorModificado.fechaCreacion =
      temp.fechaCreacion;              // Mantener fecha original
  doctorModificado.eliminado = false;  // Asegurar que no este eliminado
  doctorModificado.fechaModificacion =
      time(nullptr);  // Nueva fecha de modificacion

  // Preservar relaciones importantes
  doctorModificado.cantidadPacientes = temp.cantidadPacientes;
  doctorModificado.cantidadCitas = temp.cantidadCitas;

  // Preservar arreglos de relaciones
  for (int i = 0; i < 50; i++) {
    doctorModificado.pacientesIDs[i] = temp.pacientesIDs[i];
  }
  for (int i = 0; i < 30; i++) {
    doctorModificado.citasIDs[i] = temp.citasIDs[i];
  }

  // Volver a posicionarse y escribir
  archivo.seekp(posicion);
  archivo.write(reinterpret_cast<const char*>(&doctorModificado),
                sizeof(Doctor));
  archivo.flush();

  bool exito = !archivo.fail();
  archivo.close();

  //  Mostrar resultado
  if (exito) {
    cout << "ACTUALIZACION EXITOSA - Doctor ID " << doctorModificado.id << endl;

    // Mostrar cambios si los nombres son diferentes
    if (strcmp(doctorActual.nombre, doctorModificado.nombre) != 0) {
      cout << "  Nombre: " << doctorActual.nombre << " -> "
           << doctorModificado.nombre << endl;
    }
    if (strcmp(doctorActual.apellido, doctorModificado.apellido) != 0) {
      cout << "  Apellido: " << doctorActual.apellido << " -> "
           << doctorModificado.apellido << endl;
    }
    if (strcmp(doctorActual.especialidad, doctorModificado.especialidad) != 0) {
      cout << "  Especialidad: " << doctorActual.especialidad << " -> "
           << doctorModificado.especialidad << endl;
    }
    if (doctorActual.costoConsulta != doctorModificado.costoConsulta) {
      cout << "  Costo: $" << doctorActual.costoConsulta << " -> $"
           << doctorModificado.costoConsulta << endl;
    }

    cout << "  Modificado: "
         << formatearFecha(doctorModificado.fechaModificacion) << endl;
  } else {
    cout << "ERROR: No se pudo actualizar doctor" << endl;
  }

  return exito;
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
  cout << left << setw(4) << "ID" << setw(20) << "NOMBRE COMPLETO" << setw(15)
       << "ESPECIALIDAD" << setw(5) << "aniosS" << setw(10) << "COSTO"
       << setw(10) << "ESTADO" << endl;
  cout << string(70, '-') << endl;

  for (int i = 0; i < cantidad; i++) {
    Doctor* d = doctores[i];
    char nombreCompleto[100];
    snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", d->nombre,
             d->apellido);

    cout << left << setw(4) << d->id << setw(20) << nombreCompleto << setw(15)
         << d->especialidad << setw(5) << d->aniosExperiencia << setw(10)
         << d->costoConsulta << setw(10)
         << (d->disponible ? "Activo" : "Inactivo") << endl;
  }
  cout << string(70, '-') << endl;
}
bool actualizarDoctorEnArchivo(const Doctor& doctor) {
    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de doctores" << endl;
        return false;
    }
    
    // Buscar la posición del doctor en el archivo
    int indice = buscarIndiceDoctorPorID(doctor.id);
    if (indice == -1) {
        cout << "Error: No se encontró el doctor en el archivo" << endl;
        archivo.close();
        return false;
    }
    
    // Calcular posición en el archivo
    long posicion = sizeof(ArchivoHeader) + (indice * sizeof(Doctor));
    
    // Escribir el doctor actualizado
    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    archivo.flush();
    
    bool exito = archivo.good();
    archivo.close();
    
    // Actualizar header si fue exitoso
    if (exito) {
        ArchivoHeader header = leerHeader("doctores.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.fechaUltimaModificacion = time(nullptr);
            actualizarHeader("doctores.bin", header);
        }
        cout << "? Doctor actualizado exitosamente en archivo" << endl;
    } else {
        cout << "? Error: No se pudo actualizar el doctor en archivo" << endl;
    }
    
    return exito;
}
bool asignarPacienteADoctor(int idDoctor, int idPaciente) {
    // Buscar doctor en archivo
    Doctor doctor = buscarDoctorPorID(idDoctor);
    if (doctor.id == -1) {
        cout << "Error: No se encontró doctor con ID " << idDoctor << endl;
        return false;
    }

    // Buscar paciente en archivo
    Paciente paciente = buscarPacientePorID(idPaciente);
    if (paciente.id == -1) {
        cout << "Error: No se encontró paciente con ID " << idPaciente << endl;
        return false;
    }

    // Verificar si el paciente ya está asignado al doctor
    for (int i = 0; i < doctor.cantidadPacientes; i++) {
        if (doctor.pacientesIDs[i] == idPaciente) {
            cout << "El paciente ya está asignado al doctor." << endl;
            return false;
        }
    }

    // Verificar capacidad máxima
    if (doctor.cantidadPacientes >= 50) {
        cout << "Error: El doctor ya tiene el máximo de pacientes asignados (50)" << endl;
        return false;
    }

    // Asignar paciente al doctor
    doctor.pacientesIDs[doctor.cantidadPacientes] = idPaciente;
    doctor.cantidadPacientes++;
    doctor.fechaModificacion = time(nullptr);

    // Guardar doctor actualizado en archivo
    bool exito = actualizarDoctorEnArchivo(doctor);
    
    if (exito) {
        cout << "Paciente ID " << idPaciente << " asignado al Doctor ID " << idDoctor
             << " correctamente." << endl;
    } else {
        cout << "Error: No se pudo guardar la asignación" << endl;
    }

    return exito;
}

void listarPacientesDeDoctor(int idDoctor) {
    // Buscar doctor en archivo
    Doctor doctor = buscarDoctorPorID(idDoctor);
    if (doctor.id == -1) {
        cout << "Error: No se encontró doctor con ID " << idDoctor << endl;
        return;
    }

    if (doctor.cantidadPacientes == 0) {
        cout << "El doctor " << doctor.nombre << " " << doctor.apellido
             << " no tiene pacientes asignados." << endl;
        return;
    }

    cout << "\n=== PACIENTES ASIGNADOS AL DOCTOR " << doctor.nombre << " "
         << doctor.apellido << " ===" << endl;
    cout << left << setw(4) << "ID" << setw(20) << "NOMBRE COMPLETO" << setw(12)
         << "CÉDULA" << setw(5) << "EDAD" << endl;
    cout << string(50, '-') << endl;

    int pacientesMostrados = 0;

    for (int i = 0; i < doctor.cantidadPacientes; i++) {
        int idPaciente = doctor.pacientesIDs[i]; // CORREGIDO: pacientesIDs en lugar de pacientesAsignados
        
        // Buscar paciente en archivo
        Paciente paciente = buscarPacientePorID(idPaciente);
        if (paciente.id != -1 && !paciente.eliminado) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s",
                     paciente.nombre, paciente.apellido);

            // Truncar nombre si es muy largo
            if (strlen(nombreCompleto) > 18) {
                nombreCompleto[15] = '.';
                nombreCompleto[16] = '.';
                nombreCompleto[17] = '.';
                nombreCompleto[18] = '\0';
            }

            cout << left << setw(4) << paciente.id << setw(20) << nombreCompleto
                 << setw(12) << paciente.cedula << setw(5) << paciente.edad << endl;
            
            pacientesMostrados++;
        }
    }

    cout << string(50, '-') << endl;
    
    // Mostrar estadísticas
    if (pacientesMostrados == 0) {
        cout << "No se encontraron pacientes activos asignados a este doctor." << endl;
    } else {
        cout << "Total de pacientes mostrados: " << pacientesMostrados << endl;
        cout << "Pacientes en lista del doctor: " << doctor.cantidadPacientes << endl;
        
        if (pacientesMostrados < doctor.cantidadPacientes) {
            cout << "Nota: " << (doctor.cantidadPacientes - pacientesMostrados) 
                 << " paciente(s) pueden estar eliminados o no encontrados." << endl;
        }
    }
}
void listarDoctores(bool mostrarEliminados = false) {
  ArchivoHeader header = leerHeader("doctores.bin");

  if (header.registrosActivos == 0 && !mostrarEliminados) {
    cout << "No hay doctores registrados." << endl;
    return;
  }

  const int MAX_DOCTORES = 100;
  Doctor doctores[MAX_DOCTORES];
  int cantidad = leerTodosDoctores(doctores, MAX_DOCTORES, !mostrarEliminados);

  if (cantidad == 0) {
    cout << "No se encontraron doctores" << endl;
    return;
  }

  cout << "\nLISTA DE DOCTORES ";
  if (mostrarEliminados) {
    cout << "(INCLUYENDO ELIMINADOS)";
  }
  cout << " (" << cantidad << "):" << endl;

  cout << "+-------------------------------------------------------------------"
          "------------------+"
       << endl;
  cout << "¦  ID  ¦       NOMBRE COMPLETO    ¦   ESPECIALIDAD   ¦ AÑOS ¦    "
          "COSTO   ¦   ESTADO   ¦"
       << endl;
  cout << "+------+--------------------------+------------------+------+-------"
          "-----+------------¦"
       << endl;

  for (int i = 0; i < cantidad; i++) {
    Doctor* d = &doctores[i];

    char nombreCompleto[100];
    snprintf(nombreCompleto, sizeof(nombreCompleto), "Dr. %s %s", d->nombre,
             d->apellido);

    // Recortar nombre si es muy largo
    if (strlen(nombreCompleto) > 22) {
      nombreCompleto[19] = '.';
      nombreCompleto[20] = '.';
      nombreCompleto[21] = '.';
      nombreCompleto[22] = '\0';
    }

    // Recortar especialidad si es muy larga
    char especialidadMostrar[18];
    strncpy(especialidadMostrar, d->especialidad,
            sizeof(especialidadMostrar) - 1);
    especialidadMostrar[sizeof(especialidadMostrar) - 1] = '\0';
    if (strlen(d->especialidad) > 16) {
      especialidadMostrar[13] = '.';
      especialidadMostrar[14] = '.';
      especialidadMostrar[15] = '.';
      especialidadMostrar[16] = '\0';
    }

    char estado[12];
    if (d->eliminado) {
      strcpy(estado, "ELIMINADO");
    } else if (d->disponible) {
      strcpy(estado, "DISPONIBLE");
    } else {
      strcpy(estado, "NO DISP.");
    }

    printf("¦ %4d ¦ %-24s ¦ %-16s ¦ %4d ¦ $%9.2f ¦ %-10s ¦\n", d->id,
           nombreCompleto, especialidadMostrar, d->aniosExperiencia,
           d->costoConsulta, estado);
  }

  cout << "+-------------------------------------------------------------------"
          "------------------+"
       << endl;

  // Mostrar resumen de eliminados si aplica
  if (mostrarEliminados && header.registrosActivos < header.cantidadRegistros) {
    int eliminados = header.cantidadRegistros - header.registrosActivos;
    cout << "Informacion: " << header.registrosActivos << " activos, "
         << eliminados << " eliminados" << endl;
  }
}

bool eliminarDoctor(int id) {
    //  Buscar indice del doctor
    int indice = buscarIndiceDoctorPorID(id);
    if (indice == -1) {
        cout << "Error: No se puede encontrar doctor con ID " << id << endl;
        return false;
    }
    
    //  Abrir archivo en modo lectura/escritura
    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir doctores.bin para eliminacion" << endl;
        return false;
    }
    
    // Posicionarse en el registro
    long posicion = calcularPosicionDoctor(indice);
    archivo.seekg(posicion);
    
    // Leer el doctor completo
    Doctor doctor;
    archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor));
    
    if (archivo.fail()) {
        cout << "Error: No se puede leer doctor ID " << id << endl;
        archivo.close();
        return false;
    }
    
    // Verificar que no este ya eliminado
    if (doctor.eliminado) {
        cout << "Advertencia: Doctor ID " << id << " ya esta eliminado" << endl;
        archivo.close();
        return false;
    }
    
    //  Marcar: doctor.eliminado = true
    doctor.eliminado = true;
    doctor.fechaModificacion = time(nullptr);
    
    //  Volver a posicionarse (seekp)
    archivo.seekp(posicion);
    
    //  Sobrescribir el registro modificado
    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    archivo.flush();
    
    bool escrituraExitosa = !archivo.fail();
    archivo.close();
    
    if (!escrituraExitosa) {
        cout << "Error: No se pudo marcar doctor como eliminado" << endl;
        return false;
    }
    
    //  Actualizar header.registrosActivos--
    ArchivoHeader header = leerHeader("doctores.bin");
    if (header.registrosActivos > 0) {
        header.registrosActivos--;
        header.fechaUltimaModificacion = time(nullptr);
        
        if (!actualizarHeader("doctores.bin", header)) {
            cout << "Advertencia: Doctor marcado como eliminado pero header no actualizado" << endl;
            return false;
        }
    }
    
    //  Mostrar resultado
    cout << "ELIMINACION LOGICA EXITOSA" << endl;
    cout << "Doctor ID " << id << " marcado como eliminado" << endl;
    cout << "Fecha de eliminacion: " << formatearFecha(doctor.fechaModificacion) << endl;
    cout << "Registros activos restantes: " << header.registrosActivos << endl;
    
    return true;
}


int contarDoctoresEliminados() {
    ArchivoHeader header = leerHeader("doctores.bin");
    return header.cantidadRegistros - header.registrosActivos;
}

void listarDoctoresEliminados() {
    ArchivoHeader header = leerHeader("doctores.bin");
    
    if (header.registrosActivos == header.cantidadRegistros) {
        cout << "No hay doctores eliminados" << endl;
        return;
    }
    
    cout << "DOCTORES ELIMINADOS (" << (header.cantidadRegistros - header.registrosActivos) << "):" << endl;
    cout << "=========================================" << endl;
    
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) return;
    
    archivo.seekg(sizeof(ArchivoHeader));
    
    Doctor d;
    int contador = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor));
        if (d.eliminado) {
            cout << "ID: " << d.id << " | Dr. " << d.nombre << " " << d.apellido;
            
            char fechaStr[20];
            strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d", localtime(&d.fechaModificacion));
            cout << " | Eliminado: " << fechaStr << endl;
            
            contador++;
        }
    }
    
    archivo.close();
    cout << "=========================================" << endl;
    cout << "Total: " << contador << " doctores eliminados" << endl;
}

bool restaurarDoctor(int id) {
    int indice = buscarIndiceDoctorPorID(id);
    if (indice == -1) {
        cout << "Error: No se puede encontrar doctor con ID " << id << endl;
        return false;
    }
    
    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    long posicion = calcularPosicionDoctor(indice);
    
    // Leer doctor
    Doctor doctor;
    archivo.seekg(posicion);
    archivo.read(reinterpret_cast<char*>(&doctor), sizeof(Doctor));
    
    if (!doctor.eliminado) {
        cout << "Advertencia: Doctor ID " << id << " no está eliminado" << endl;
        archivo.close();
        return false;
    }
    
    // Restaurar
    doctor.eliminado = false;
    doctor.fechaModificacion = time(nullptr);
    
    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&doctor), sizeof(Doctor));
    archivo.flush();
    
    bool exito = archivo.good();
    archivo.close();
    
    if (exito) {
        // Actualizar header - CORREGIDO
        ArchivoHeader header = leerHeader("doctores.bin");
        
        // Verificar si el header es válido
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.registrosActivos++;
            header.fechaUltimaModificacion = time(nullptr);
            actualizarHeader("doctores.bin", header);
            
            cout << "DOCTOR RESTAURADO" << endl;
            cout << "Doctor ID " << id << " reactivado exitosamente" << endl;
            cout << "Registros activos: " << header.registrosActivos << endl;
        } else {
            cout << "DOCTOR RESTAURADO" << endl;
            cout << "Doctor ID " << id << " reactivado exitosamente" << endl;
            cout << "Nota: No se pudo actualizar el header del archivo" << endl;
        }
    }
    
    return exito;
}


bool atenderCita(int idCita, const char* diagnostico, const char* tratamiento, const char* medicamentos) {
    // 1. Validar parámetros
    if (!diagnostico || !tratamiento || !medicamentos) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }

    if (strlen(diagnostico) == 0) {
        cout << "Error: El diagnóstico no puede estar vacío" << endl;
        return false;
    }

    // 2. Buscar la cita por ID
    int indiceCita = buscarIndiceCitaPorID(idCita);
    if (indiceCita == -1) {
        cout << "Error: No se encontró cita con ID " << idCita << endl;
        return false;
    }

    // 3. Leer la cita completa
    Cita cita = leerCitaPorIndice(indiceCita);
    if (cita.id == -1) {
        cout << "Error: No se puede acceder a la cita ID " << idCita << endl;
        return false;
    }

    // 4. Verificar que esté en estado "Agendada"
    if (strcmp(cita.estado, "AGENDADA") != 0) {
        cout << "Error: La cita no está en estado 'Agendada'. Estado actual: "
             << cita.estado << endl;
        return false;
    }

    if (cita.atendida) {
        cout << "Error: La cita ya fue atendida." << endl;
        return false;
    }

    // 5. Obtener información del paciente y doctor
    Paciente paciente = buscarPacientePorID(cita.idPaciente);
    Doctor doctor = buscarDoctorPorID(cita.idDoctor);

    if (paciente.id == -1) {
        cout << "Error: No se encontró el paciente asociado a la cita." << endl;
        return false;
    }

    if (doctor.id == -1) {
        cout << "Error: No se encontró el doctor asociado a la cita." << endl;
        return false;
    }

    // 6. Actualizar estado de la cita - CORREGIDO (eliminado fechaModificacion)
    strcpy(cita.estado, "ATENDIDA");
    cita.atendida = true;
    // cita.fechaModificacion = time(nullptr); // ELIMINADO - La estructura Cita no tiene este campo
    
    char observaciones[200];
    snprintf(observaciones, sizeof(observaciones), "Atendida - Diagnóstico: %s", diagnostico);
    strncpy(cita.observaciones, observaciones, sizeof(cita.observaciones) - 1);

    // 7. Guardar cambios en la cita
    fstream archivoCitas("citas.bin", ios::binary | ios::in | ios::out);
    if (!archivoCitas.is_open()) {
        cout << "Error: No se puede abrir citas.bin para actualización" << endl;
        return false;
    }

    long posicionCita = sizeof(ArchivoHeader) + (indiceCita * sizeof(Cita));
    archivoCitas.seekp(posicionCita);
    archivoCitas.write(reinterpret_cast<const char*>(&cita), sizeof(Cita));
    archivoCitas.flush();
    bool exitoCita = !archivoCitas.fail();
    archivoCitas.close();

    if (!exitoCita) {
        cout << "Error: No se pudo actualizar la cita" << endl;
        return false;
    }

    // 8. Crear entrada en el historial médico
    HistorialMedico nuevaConsulta;
    memset(&nuevaConsulta, 0, sizeof(HistorialMedico));

    // Obtener próximo ID del historial
    ArchivoHeader headerHistorial = leerHeader("historiales.bin");
    if (strcmp(headerHistorial.tipoArchivo, "INVALIDO") == 0) {
        if (!inicializarArchivo("historiales.bin")) {
            cout << "Error: No se pudo inicializar archivo de historiales" << endl;
            return false;
        }
        headerHistorial = leerHeader("historiales.bin");
    }

    nuevaConsulta.id = headerHistorial.proximoID;
    nuevaConsulta.pacienteID = cita.idPaciente; // CORREGIDO: pacienteID en lugar de idPaciente
    nuevaConsulta.idDoctor = cita.idDoctor;
    
    strncpy(nuevaConsulta.fecha, cita.fecha, sizeof(nuevaConsulta.fecha) - 1);
    strncpy(nuevaConsulta.hora, cita.hora, sizeof(nuevaConsulta.hora) - 1);
    strncpy(nuevaConsulta.diagnostico, diagnostico, sizeof(nuevaConsulta.diagnostico) - 1);
    strncpy(nuevaConsulta.tratamiento, tratamiento, sizeof(nuevaConsulta.tratamiento) - 1);
    strncpy(nuevaConsulta.medicamentos, medicamentos, sizeof(nuevaConsulta.medicamentos) - 1);
    
    nuevaConsulta.costo = doctor.costoConsulta;
    nuevaConsulta.eliminado = false;
    nuevaConsulta.fechaRegistro = time(nullptr);

    // 9. Guardar en historial médico
    ofstream archivoHistorial("historiales.bin", ios::binary | ios::app);
    if (!archivoHistorial.is_open()) {
        cout << "Error: No se puede abrir historiales.bin" << endl;
        return false;
    }

    archivoHistorial.write(reinterpret_cast<const char*>(&nuevaConsulta), sizeof(HistorialMedico));
    bool exitoHistorial = !archivoHistorial.fail();
    archivoHistorial.close();

    if (!exitoHistorial) {
        cout << "Error: No se pudo guardar el historial médico" << endl;
        return false;
    }

    // 10. Actualizar header del historial
    headerHistorial.cantidadRegistros++;
    headerHistorial.proximoID++;
    headerHistorial.registrosActivos++;
    headerHistorial.fechaUltimaModificacion = time(nullptr);
    actualizarHeader("historiales.bin", headerHistorial);

    // 11. Actualizar contador de consultas del paciente
    paciente.cantidadConsultas++;
    paciente.fechaModificacion = time(nullptr);
    actualizarPacienteEnArchivo(paciente); // CORREGIDO: usar actualizarPacienteEnArchivo

    // 12. Mostrar confirmación
    cout << "CITA ATENDIDA EXITOSAMENTE" << endl;
    cout << "ID Cita: " << cita.id << endl;
    cout << "Paciente: " << paciente.nombre << " " << paciente.apellido << endl;
    cout << "Doctor: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
    cout << "Fecha: " << cita.fecha << " " << cita.hora << endl;
    cout << "Costo: $" << doctor.costoConsulta << endl;
    cout << "Consulta agregada al historial médico. ID Consulta: " << nuevaConsulta.id << endl;

    return true;
}
bool cancelarCita(int idCita) {
    // Validar ID
    if (idCita <= 0) {
        cout << "Error: ID de cita inválido" << endl;
        return false;
    }

    // Buscar la cita por ID en el archivo
    int indice = buscarIndiceCitaPorID(idCita);
    if (indice == -1) {
        cout << "Error: No se encontró cita con ID " << idCita << endl;
        return false;
    }

    // Leer la cita completa
    Cita cita = leerCitaPorIndice(indice);
    if (cita.id == -1) {
        cout << "Error: No se puede acceder a la cita ID " << idCita << endl;
        return false;
    }

    // Verificar que la cita no esté ya cancelada
    if (strcmp(cita.estado, "CANCELADA") == 0) {
        cout << "La cita ya está cancelada." << endl;
        return false;
    }

    // Verificar que no esté atendida
    if (cita.atendida) {
        cout << "Error: No se puede cancelar una cita ya atendida." << endl;
        return false;
    }

    // Obtener información del paciente y doctor para el mensaje
    Paciente paciente = buscarPacientePorID(cita.idPaciente);
    Doctor doctor = buscarDoctorPorID(cita.idDoctor);

    // Actualizar estado de la cita - CORREGIDO (eliminado fechaModificacion)
    strcpy(cita.estado, "CANCELADA");
    cita.atendida = false;
    // cita.fechaModificacion = time(nullptr); // ELIMINADO - La estructura Cita no tiene este campo
    strcpy(cita.observaciones, "Cita cancelada por el usuario");

    // Guardar los cambios en el archivo
    fstream archivo("citas.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin para actualización" << endl;
        return false;
    }

    // Calcular posición y escribir
    long posicion = sizeof(ArchivoHeader) + (indice * sizeof(Cita));
    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&cita), sizeof(Cita));
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (!exito) {
        cout << "Error: No se pudo actualizar la cita en el archivo" << endl;
        return false;
    }

    // Actualizar header del archivo de citas
    ArchivoHeader header = leerHeader("citas.bin");
    if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
        header.fechaUltimaModificacion = time(nullptr);
        actualizarHeader("citas.bin", header);
    }

    // Mostrar confirmación
    cout << "CITA CANCELADA EXITOSAMENTE" << endl;
    cout << "ID Cita: " << cita.id << endl;
    
    if (paciente.id != -1) {
        cout << "Paciente: " << paciente.nombre << " " << paciente.apellido << endl;
    } else {
        cout << "Paciente: No encontrado" << endl;
    }
    
    if (doctor.id != -1) {
        cout << "Doctor: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
    } else {
        cout << "Doctor: No encontrado" << endl;
    }
    
    cout << "Fecha: " << cita.fecha << " " << cita.hora << endl;
    cout << "Motivo original: " << cita.motivo << endl;

    return true;
}


Cita** obtenerCitasDePaciente(int idPaciente, int* cantidad) {
    // 1. Validar parámetros (igual que tu código)
    if (!cantidad) {
        return nullptr;
    }
    *cantidad = 0;

    // 2. Verificar que el paciente exista
    Paciente paciente = buscarPacientePorID(idPaciente);
    if (paciente.id == -1) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        return nullptr;
    }

    // 3. Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "El paciente no tiene citas registradas." << endl;
        return nullptr;
    }

    // 4. Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        archivo.close();
        return nullptr;
    }

    // 5. Contar citas del paciente
    int totalCitasPaciente = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.eliminado && cita.idPaciente == idPaciente) {
                totalCitasPaciente++;
            }
        }
    }

    // 6. Si no tiene citas, retornar nullptr
    if (totalCitasPaciente == 0) {
        cout << "El paciente no tiene citas registradas." << endl;
        archivo.close();
        return nullptr;
    }

    // 7. CREAR ARRAY DE PUNTEROS Y ESTRUCTURAS (SOLUCIÓN SEGURA)
    Cita** resultados = new Cita*[totalCitasPaciente];
    
    // Volver al inicio y leer citas
    archivo.clear();
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    
    int indice = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.eliminado && cita.idPaciente == idPaciente) {
                // Crear NUEVA estructura en el heap para cada cita
                resultados[indice] = new Cita;
                *resultados[indice] = cita; // Copiar los datos
                indice++;
            }
        }
    }
    archivo.close();

    *cantidad = totalCitasPaciente;

    // 8. Mantener el mismo mensaje
    cout << "Encontradas " << *cantidad << " citas para el paciente ID "
         << idPaciente << endl;

    return resultados;
}

bool validarFecha(const char* fecha) {
  // Verificar formato YYYY-MM-DD (longitud 10)
  if (!fecha || strlen(fecha) != 10) {
    cout << "Error: La fecha debe tener formato YYYY-MM-DD (10 caracteres)"
         << endl;
    return false;
  }

  // Verificar que los separadores sean guiones
  if (fecha[4] != '-' || fecha[7] != '-') {
    cout << "Error: Formato incorrecto. Use YYYY-MM-DD con guiones" << endl;
    return false;
  }

  // Verificar que todos los caracteres excepto los guiones sean dÃ­gitos
  for (int i = 0; i < 10; i++) {
    if (i != 4 && i != 7 && !isdigit(fecha[i])) {
      cout << "Error: La fecha debe contener solo nÃºmeros y guiones" << endl;
      return false;
    }
  }

  int anios = atoi(fecha);
  int mes = atoi(fecha + 5);
  int dia = atoi(fecha + 8);

  // Validar anios (rango razonable para un sistema hospitalario)
  if (anios < 2020 || anios > 2030) {
    cout << "Error: anios " << anios << " fuera de rango. Use entre 2020-2030"
         << endl;
    return false;
  }

  // Validar mes (1-12)
  if (mes < 1 || mes > 12) {
    cout << "Error: Mes " << mes << " invÃ¡lido. Use 1-12" << endl;
    return false;
  }

  // Validar dÃ­a segÃºn el mes
  int diasEnMes;

  switch (mes) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      diasEnMes = 31;
      break;
    case 4:
    case 6:
    case 9:
    case 11:
      diasEnMes = 30;
      break;
    case 2:
      // Febrero - considerar anios bisiestos
      if ((anios % 4 == 0 && anios % 100 != 0) || (anios % 400 == 0)) {
        diasEnMes = 29;  // anios bisiesto
      } else {
        diasEnMes = 28;  // anios no bisiesto
      }
      break;
    default:
      diasEnMes = 0;  // Nunca deberÃ­a llegar aquÃ­
  }

  if (dia < 1 || dia > diasEnMes) {
    cout << "Error: DÃ­a " << dia << " invÃ¡lido para " << obtenerNombreMes(mes)
         << " " << anios << " (mÃ¡ximo " << diasEnMes << " dÃ­as)" << endl;
    return false;
  }

  return true;
}
bool agendarCita(int idPaciente, int idDoctor, const char* fecha, const char* hora, const char* motivo) {
    // Validaciones básicas de parámetros
    if (!fecha || !hora || !motivo) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }

    // Verificar que el paciente exista (usando función de archivos)
    Paciente paciente = buscarPacientePorID(idPaciente);
    if (paciente.id == -1) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        return false;
    }

    // Verificar que el doctor exista (usando función de archivos)
    Doctor doctor = buscarDoctorPorID(idDoctor);
    if (doctor.id == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return false;
    }

    // Verificar que el doctor esté disponible
    if (!doctor.disponible) {
        cout << "Error: El doctor no está disponible." << endl;
        return false;
    }

    // Validar formato de fecha (YYYY-MM-DD)
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha inválido. Use YYYY-MM-DD" << endl;
        return false;
    }

    // Validar que los componentes de la fecha sean números
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(fecha[i])) {
            cout << "Error: La fecha debe contener solo números y guiones." << endl;
            return false;
        }
    }

    // Validar formato de hora (HH:MM)
    if (strlen(hora) != 5 || hora[2] != ':') {
        cout << "Error: Formato de hora inválido. Use HH:MM" << endl;
        return false;
    }

    // Validar que los componentes de la hora sean números
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) {
            cout << "Error: La hora debe contener solo números y dos puntos." << endl;
            return false;
        }
    }

    // Validar rango de hora (00-23:00-59)
    int horas = atoi(hora);
    int minutos = atoi(hora + 3);
    if (horas < 0 || horas > 23 || minutos < 0 || minutos > 59) {
        cout << "Error: Hora fuera de rango. Use HH:MM entre 00:00 y 23:59" << endl;
        return false;
    }

    // Verificar disponibilidad del doctor en archivo
    ifstream archivoCitas("citas.bin", ios::binary);
    if (archivoCitas.is_open()) {
        ArchivoHeader header;
        archivoCitas.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
        
        for (int i = 0; i < header.cantidadRegistros; i++) {
            Cita citaExistente;
            if (archivoCitas.read(reinterpret_cast<char*>(&citaExistente), sizeof(Cita))) {
                if (!citaExistente.eliminado && 
                    citaExistente.idDoctor == idDoctor &&
                    strcmp(citaExistente.fecha, fecha) == 0 &&
                    strcmp(citaExistente.hora, hora) == 0 &&
                    strcmp(citaExistente.estado, "AGENDADA") == 0) {
                    
                    cout << "Error: El doctor ya tiene una cita agendada para " << fecha
                         << " a las " << hora << endl;
                    archivoCitas.close();
                    return false;
                }
            }
        }
        archivoCitas.close();
    }

    // Verificar que el paciente no tenga otra cita a la misma hora
    ifstream archivoCitas2("citas.bin", ios::binary);
    if (archivoCitas2.is_open()) {
        ArchivoHeader header;
        archivoCitas2.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
        
        for (int i = 0; i < header.cantidadRegistros; i++) {
            Cita citaExistente;
            if (archivoCitas2.read(reinterpret_cast<char*>(&citaExistente), sizeof(Cita))) {
                if (!citaExistente.eliminado && 
                    citaExistente.idPaciente == idPaciente &&
                    strcmp(citaExistente.fecha, fecha) == 0 &&
                    strcmp(citaExistente.hora, hora) == 0 &&
                    strcmp(citaExistente.estado, "AGENDADA") == 0) {
                    
                    cout << "Error: El paciente ya tiene una cita agendada para " << fecha
                         << " a las " << hora << endl;
                    archivoCitas2.close();
                    return false;
                }
            }
        }
        archivoCitas2.close();
    }

    // Crear nueva cita
    Cita nuevaCita;
    memset(&nuevaCita, 0, sizeof(Cita));
    
    // Obtener próximo ID del header
    ArchivoHeader headerCitas = leerHeader("citas.bin");
    nuevaCita.id = headerCitas.proximoID;
    nuevaCita.idPaciente = idPaciente;
    nuevaCita.idDoctor = idDoctor;
    strncpy(nuevaCita.fecha, fecha, sizeof(nuevaCita.fecha) - 1);
    strncpy(nuevaCita.hora, hora, sizeof(nuevaCita.hora) - 1);
    strncpy(nuevaCita.motivo, motivo, sizeof(nuevaCita.motivo) - 1);
    strcpy(nuevaCita.estado, "AGENDADA");
    strcpy(nuevaCita.observaciones, "");
    nuevaCita.atendida = false;
    nuevaCita.eliminado = false;

    // Guardar cita en archivo
    ofstream archivo("citas.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin para guardar" << endl;
        return false;
    }

    archivo.write(reinterpret_cast<const char*>(&nuevaCita), sizeof(Cita));
    bool exito = !archivo.fail();
    archivo.close();

    if (!exito) {
        cout << "Error: No se pudo guardar la cita" << endl;
        return false;
    }

    // Actualizar header
    headerCitas.cantidadRegistros++;
    headerCitas.proximoID++;
    headerCitas.registrosActivos++;
    headerCitas.fechaUltimaModificacion = time(nullptr);
    actualizarHeader("citas.bin", headerCitas);

    cout << "Cita agendada exitosamente. ID: " << nuevaCita.id << endl;
    cout << "Paciente: " << paciente.nombre << " " << paciente.apellido << endl;
    cout << "Doctor: " << doctor.nombre << " " << doctor.apellido << endl;
    cout << "Fecha: " << fecha << " " << hora << endl;
    cout << "Motivo: " << motivo << endl;

    return true;
}
Cita** obtenerCitasDeDoctor(int idDoctor, int* cantidad) {
    // 1. Validar parámetros (sin Hospital*)
    if (!cantidad) {
        return nullptr;
    }
    *cantidad = 0;

    // 2. Verificar que el doctor exista (en archivo)
    Doctor doctor = buscarDoctorPorID(idDoctor);
    if (doctor.id == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return nullptr;
    }

    // 3. Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "El doctor no tiene citas registradas." << endl;
        return nullptr;
    }

    // 4. Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        archivo.close();
        return nullptr;
    }

    // 5. Contar citas del doctor
    int totalCitasDoctor = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.eliminado && cita.idDoctor == idDoctor) {
                totalCitasDoctor++;
            }
        }
    }

    // 6. Si no tiene citas, retornar nullptr
    if (totalCitasDoctor == 0) {
        cout << "El doctor no tiene citas registradas." << endl;
        archivo.close();
        return nullptr;
    }

    // 7. Crear array de punteros y estructuras (solución segura)
    Cita** resultados = new Cita*[totalCitasDoctor];
    
    // Volver al inicio y leer citas
    archivo.clear();
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);
    
    int indice = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.eliminado && cita.idDoctor == idDoctor) {
                // Crear nueva estructura en el heap para cada cita
                resultados[indice] = new Cita;
                *resultados[indice] = cita; // Copiar los datos
                indice++;
            }
        }
    }
    archivo.close();

    *cantidad = totalCitasDoctor;

    // 8. Mantener el mismo mensaje
    cout << "Encontradas " << *cantidad << " citas para el doctor ID " << idDoctor
         << endl;

    return resultados;
}

Cita* obtenerCitasPorFecha(const char* fecha, int* cantidad) {
    // 1. Validar parámetros
    if (!fecha || !cantidad) {
        if (cantidad) *cantidad = 0;
        return nullptr;
    }
    *cantidad = 0;

    // 2. Validar formato de fecha
    if (!validarFecha(fecha)) {
        cout << "Error: Formato de fecha inválido. Use YYYY-MM-DD" << endl;
        return nullptr;
    }

    // 3. Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir citas.bin" << endl;
        return nullptr;
    }

    // 4. Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        cout << "Error: No se puede leer header de citas.bin" << endl;
        archivo.close();
        return nullptr;
    }

    // 5. Contar citas para la fecha
    int maxCitas = header.cantidadRegistros;
    Cita* todasCitas = new Cita[maxCitas];
    int citasEncontradas = 0;

    // Leer todas las citas
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.eliminado && strcmp(cita.fecha, fecha) == 0) {
                todasCitas[citasEncontradas] = cita;
                citasEncontradas++;
            }
        }
    }
    archivo.close();

    // 6. Si no hay citas, liberar memoria y retornar
    if (citasEncontradas == 0) {
        delete[] todasCitas;
        cout << "No hay citas registradas para la fecha " << fecha << endl;
        return nullptr;
    }

    // 7. Crear array del tamaño exacto
    Cita* resultados = new Cita[citasEncontradas];
    for (int i = 0; i < citasEncontradas; i++) {
        resultados[i] = todasCitas[i];
    }
    delete[] todasCitas;

    *cantidad = citasEncontradas;

    // 8. Mostrar resultados
    cout << "Encontradas " << citasEncontradas << " citas para la fecha " << fecha << endl;

    return resultados;
}

void listarCitasPendientes() {
    // 1. Abrir archivo de citas
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "No hay citas en el sistema." << endl;
        return;
    }

    // 2. Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail() || header.cantidadRegistros == 0) {
        cout << "No hay citas en el sistema." << endl;
        archivo.close();
        return;
    }

    // 3. Contar citas pendientes
    int cantidadPendientes = 0;
    Cita* citasPendientes = new Cita[header.cantidadRegistros];

    // Leer todas las citas y filtrar pendientes
    for (int i = 0; i < header.cantidadRegistros; i++) {
        Cita cita;
        if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
            if (!cita.eliminado && strcmp(cita.estado, "AGENDADA") == 0) {
                citasPendientes[cantidadPendientes] = cita;
                cantidadPendientes++;
            }
        }
    }
    archivo.close();

    // 4. Mostrar resultados
    if (cantidadPendientes == 0) {
        cout << "No hay citas pendientes." << endl;
        delete[] citasPendientes;
        return;
    }

    cout << "\n================================================================" << endl;
    cout << "               CITAS PENDIENTES - TOTAL: " << cantidadPendientes << endl;
    cout << "================================================================" << endl;

    for (int i = 0; i < cantidadPendientes; i++) {
        Cita c = citasPendientes[i];
        Paciente paciente = buscarPacientePorID(c.idPaciente);
        Doctor doctor = buscarDoctorPorID(c.idDoctor);

        cout << "CITA #" << c.id << " -----------------------------------------" << endl;
        cout << "  Fecha: " << c.fecha << " " << c.hora << endl;
        cout << "  Paciente: " << (paciente.id != -1 ? 
              string(paciente.nombre) + " " + paciente.apellido : "No encontrado") << endl;
        cout << "  Doctor: " << (doctor.id != -1 ? 
              "Dr. " + string(doctor.nombre) + " " + doctor.apellido : "No encontrado") << endl;
        cout << "  Especialidad: " << (doctor.id != -1 ? doctor.especialidad : "N/A") << endl;
        cout << "  Motivo: " << c.motivo << endl;
        cout << "  Costo: $" << (doctor.id != -1 ? doctor.costoConsulta : 0) << endl;
        cout << endl;
    }

    cout << "================================================================" << endl;
    delete[] citasPendientes;
}

bool validarHora(const char* hora) {
  if (strlen(hora) != 5) {
    return false;
  }

  // Verificar formato HH:MM
  // Posiciones: 0,1 = horas; 2 = ':' ; 3,4 = minutos
  if (hora[2] != ':') {
    return false;
  }

  // Verificar que todos los caracteres numÃ©ricos sean dÃ­gitos
  if (!isdigit(hora[0]) || !isdigit(hora[1]) || !isdigit(hora[3]) ||
      !isdigit(hora[4])) {
    return false;
  }

  // Convertir horas y minutos a nÃºmeros
  int horas = (hora[0] - '0') * 10 + (hora[1] - '0');
  int minutos = (hora[3] - '0') * 10 + (hora[4] - '0');

  // Validar rangos
  if (horas < 0 || horas > 23) {
    return false;
  }

  if (minutos < 0 || minutos > 59) {
    return false;
  }

  return true;
}
int compararFechas(const char* fecha1, const char* fecha2) {
  // Asumimos formato DD/MM/AAAA (10 caracteres)
  // Extraer dÃ­a, mes y anios de ambas fechas

  // Fecha 1
  int dia1 = (fecha1[0] - '0') * 10 + (fecha1[1] - '0');
  int mes1 = (fecha1[3] - '0') * 10 + (fecha1[4] - '0');
  int anios1 = (fecha1[6] - '0') * 1000 + (fecha1[7] - '0') * 100 +
               (fecha1[8] - '0') * 10 + (fecha1[9] - '0');

  // Fecha 2
  int dia2 = (fecha2[0] - '0') * 10 + (fecha2[1] - '0');
  int mes2 = (fecha2[3] - '0') * 10 + (fecha2[4] - '0');
  int anios2 = (fecha2[6] - '0') * 1000 + (fecha2[7] - '0') * 100 +
               (fecha2[8] - '0') * 10 + (fecha2[9] - '0');

  // Comparar por anios
  if (anios1 < anios2) return -1;
  if (anios1 > anios2) return 1;

  // Si anios iguales, comparar por mes
  if (mes1 < mes2) return -1;
  if (mes1 > mes2) return 1;

  // Si meses iguales, comparar por dÃ­a
  if (dia1 < dia2) return -1;
  if (dia1 > dia2) return 1;

  // Si todos son iguales
  return 0;
}

bool verificarDisponibilidad(int idDoctor, const char* fecha, const char* hora) {
    // 1. Validar parámetros
    if (!fecha || !hora) {
        cout << "Error: Parámetros inválidos." << endl;
        return false;
    }

    // 2. Verificar que el doctor exista
    Doctor doctor = buscarDoctorPorID(idDoctor);
    if (doctor.id == -1) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return false;
    }

    // 3. Verificar que el doctor esté disponible
    if (!doctor.disponible || doctor.eliminado) {
        cout << "El doctor no está disponible para consultas." << endl;
        return false;
    }

    // 4. Validar formato de fecha
    if (!validarFecha(fecha)) {
        cout << "Error: Formato de fecha inválido." << endl;
        return false;
    }

    // 5. Validar formato de hora
    if (!validarHora(hora)) {
        cout << "Error: Formato de hora inválido." << endl;
        return false;
    }

    // 6. Verificar si el doctor ya tiene una cita a esa fecha/hora
    ifstream archivo("citas.bin", ios::binary);
    if (archivo.is_open()) {
        ArchivoHeader header;
        archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
        
        for (int i = 0; i < header.cantidadRegistros; i++) {
            Cita cita;
            if (archivo.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
                if (!cita.eliminado && 
                    cita.idDoctor == idDoctor &&
                    strcmp(cita.fecha, fecha) == 0 &&
                    strcmp(cita.hora, hora) == 0 &&
                    strcmp(cita.estado, "AGENDADA") == 0) {
                    
                    // Encontrar información del paciente para el mensaje
                    Paciente paciente = buscarPacientePorID(cita.idPaciente);
                    cout << "El doctor ya tiene una cita agendada para " << fecha << " a las "
                         << hora << endl;
                    if (paciente.id != -1) {
                        cout << "Cita con: " << paciente.nombre << " " << paciente.apellido
                             << endl;
                    }
                    archivo.close();
                    return false;
                }
            }
        }
        archivo.close();
    }

    // 7. Si llegamos aquí, el doctor está disponible
    cout << "   DOCTOR DISPONIBLE" << endl;
    cout << "   Fecha: " << fecha << " a las " << hora << endl;
    cout << "   Doctor: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
    cout << "   Especialidad: " << doctor.especialidad << endl;
    cout << "   Costo consulta: $" << doctor.costoConsulta << endl;

    return true;
}


// modulo de validaciones



const char* obtenerNombreMes(int mes) {
  static const char* nombresMeses[] = {
      "Enero", "Febrero", "Marzo",      "Abril",   "Mayo",      "Junio",
      "Julio", "Agosto",  "Septiembre", "Octubre", "Noviembre", "Diciembre"};

  if (mes >= 1 && mes <= 12) {
    return nombresMeses[mes - 1];
  }
  return "Mes invÃ¡lido";
}


bool validarEmail(const char* email) {
  if (email == nullptr || strlen(email) < 5) {  // mÃ­nimo: a@b.c
    return false;
  }

  const char* arroba = strchr(email, '@');
  if (arroba == nullptr || arroba == email) {
    return false;  // No hay @ o estÃ¡ al inicio
  }

  // Buscar punto despuÃ©s del @
  const char* punto = strchr(arroba + 1, '.');
  if (punto == nullptr || punto == arroba + 1 || *(punto + 1) == '\0') {
    return false;  // No hay punto, o estÃ¡ justo despuÃ©s del @, o no hay nada
                   // despuÃ©s
  }

  return (strchr(email, ' ') == nullptr);
}

char* copiarString(const char* origen) {
  // Si origen es nullptr, retornar nullptr
  if (origen == nullptr) {
    return nullptr;
  }

  // Calcular longitud con strlen()
  int longitud = strlen(origen);

  // Crear nuevo arreglo con new char[longitud + 1]
  char* destino = new char[longitud + 1];

  strcpy(destino, origen);

  return destino;
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Función para centrar texto en la consola
void centrarTexto(const string& texto) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int anchoConsola;
    
    // Obtener el ancho de la consola
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    anchoConsola = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    
    // Calcular espacios para centrar
    int espacios = (anchoConsola - texto.length()) / 2;
    
    // Imprimir espacios y texto
    cout << string(espacios, ' ') << texto << endl;
}

void mostrarMenuPrincipal() {
    system("cls"); // Limpiar pantalla (Windows)
    
    // Color codes: 0=Negro, 4=Rojo, 7=Gris claro (default), 15=Blanco
    const int COLOR_ROJO = 4;
    const int COLOR_NORMAL = 7;
    const int COLOR_BLANCO = 15;
    
    cout << "\n\n\n"; // Espacios verticales para centrar verticalmente
    
    // Línea superior
    centrarTexto("===============================================");
    
    // Logo en rojo
    setColor(COLOR_ROJO);
    centrarTexto("       HOSPITAL EL CALLAO V2");
    setColor(COLOR_NORMAL);
    
    // Línea inferior del encabezado
    centrarTexto("===============================================");
    
    // Opciones del menú
    centrarTexto("1.  Gestion de Pacientes");
    centrarTexto("2.  Gestion de Doctores");
    centrarTexto("3.  Gestion de Citas");
    centrarTexto("4.  Historial Medico");
    centrarTexto("5.  Reportes y Estadisticas");
    centrarTexto("6.  Mantenimiento del Sistema");
    centrarTexto("0.  Salir");
    
    // Línea final
    centrarTexto("===============================================");
    
    cout << "\n\n"; // Espacios al final
}
// FUNCIÃ“N PARA LEER OPCIONES CON VALIDACIÃ“N
int leerOpcion(const char* mensaje, int min, int max) {
  int opcion;
  while (true) {
    cout << mensaje << " [" << min << "-" << max << "]: ";
    cin >> opcion;

    if (cin.fail()) {
      cin.clear();
      limpiarBuffer();
      cout << "Error: Por favor ingrese un numero valido." << endl;
      continue;
    }

    limpiarBuffer();

    if (opcion >= min && opcion <= max) {
      return opcion;
    } else {
      cout << "Error: OpciÃ³n fuera de rango. Use [" << min << "-" << max << "]"
           << endl;
    }
  }
}

void pausarPantalla() {
  cout << "\nPresione Enter para continuar...";
  cin.get();
}

void menuGestionPacientes() {
    int opcion;

    do {
        system("cls");
        cout << "\n===============================================" << endl;
        cout << "           GESTION DE PACIENTES" << endl;
        cout << "===============================================" << endl;
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

        opcion = leerOpcion("Seleccione una opcion", 0, 10);

        switch (opcion) {
            case 1: {
                cout << "\n--- REGISTRAR NUEVO PACIENTE ---" << endl;
                // Llamar a funcion para agregar paciente a archivo
                if (agregarPaciente()) {
                    cout << "Paciente registrado exitosamente." << endl;
                } else {
                    cout << "Error al registrar paciente." << endl;
                }
                break;
            }

            case 2: {
                cout << "\n--- BUSCAR PACIENTE POR ID ---" << endl;
                int id;
                cout << "Ingrese ID del paciente: ";
                cin >> id;
                limpiarBuffer();
                
                Paciente paciente = buscarPacientePorID(id);
                if (paciente.id != -1) {
                    cout << "\nPACIENTE ENCONTRADO:" << endl;
                    cout << "ID: " << paciente.id << endl;
                    cout << "Nombre: " << paciente.nombre << " " << paciente.apellido << endl;
                    cout << "Cedula: " << paciente.cedula << endl;
                    cout << "Edad: " << paciente.edad << endl;
                    cout << "Sexo: " << paciente.sexo << endl;
                    cout << "Telefono: " << paciente.telefono << endl;
                    cout << "Email: " << paciente.email << endl;
                    cout << "Direccion: " << paciente.direccion << endl;
                    cout << "Consultas realizadas: " << paciente.cantidadConsultas << endl;
                } else {
                    cout << "Paciente no encontrado." << endl;
                }
                break;
            }

            case 3: {
                cout << "\n--- BUSCAR PACIENTE POR CEDULA ---" << endl;
                char cedula[20];
                cout << "Ingrese cedula: ";
                cin.getline(cedula, sizeof(cedula));
                
                Paciente paciente = buscarPacientePorCedula(cedula);
                if (paciente.id != -1) {
                    cout << "\nPACIENTE ENCONTRADO:" << endl;
                    cout << "ID: " << paciente.id << endl;
                    cout << "Nombre: " << paciente.nombre << " " << paciente.apellido << endl;
                    cout << "Cedula: " << paciente.cedula << endl;
                } else {
                    cout << "Paciente no encontrado." << endl;
                }
                break;
            }

            case 4: {
                cout << "\n--- BUSCAR PACIENTES POR NOMBRE ---" << endl;
                char nombre[50];
                cout << "Ingrese nombre a buscar: ";
                cin.getline(nombre, sizeof(nombre));
                
                const int MAX_RESULTADOS = 10;
                Paciente resultados[MAX_RESULTADOS];
                int cantidad = buscarPacientesPorNombre(nombre, resultados, MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    cout << "\nENCONTRADOS " << cantidad << " PACIENTES:" << endl;
                    for (int i = 0; i < cantidad; i++) {
                        cout << i+1 << ". ID: " << resultados[i].id << " - " 
                             << resultados[i].nombre << " " << resultados[i].apellido 
                             << " (Cedula: " << resultados[i].cedula << ")" << endl;
                    }
                } else {
                    cout << "No se encontraron pacientes." << endl;
                }
                break;
            }

            case 5: {
                cout << "\n--- ACTUALIZAR DATOS DE PACIENTE ---" << endl;
                int id;
                cout << "Ingrese ID del paciente a actualizar: ";
                cin >> id;
                limpiarBuffer();
                
                if (actualizarPaciente(id)) {
                    cout << "Paciente actualizado exitosamente." << endl;
                } else {
                    cout << "Error al actualizar paciente." << endl;
                }
                break;
            }

            case 6: {
                cout << "\n--- LISTA DE PACIENTES ---" << endl;
                listarPacientes();
                break;
            }

            case 7: {
                cout << "\n--- ELIMINAR PACIENTE ---" << endl;
                int id;
                cout << "Ingrese ID del paciente a eliminar: ";
                cin >> id;
                limpiarBuffer();
                
                if (eliminarPaciente(id, true)) {
                    cout << "Paciente eliminado exitosamente." << endl;
                } else {
                    cout << "Error al eliminar paciente." << endl;
                }
                break;
            }

            case 8: {
                cout << "\n--- RESTAURAR PACIENTE ELIMINADO ---" << endl;
                int id;
                cout << "Ingrese ID del paciente a restaurar: ";
                cin >> id;
                limpiarBuffer();
                
                if (restaurarPaciente(id)) {
                    cout << "Paciente restaurado exitosamente." << endl;
                } else {
                    cout << "Error al restaurar paciente." << endl;
                }
                break;
            }

            case 9: {
                cout << "\n--- PACIENTES ELIMINADOS ---" << endl;
                listarPacientesEliminados();
                break;
            }

            case 10: {
                cout << "\n--- INFORMACION COMPLETA DE PACIENTE ---" << endl;
                int id;
                cout << "Ingrese ID del paciente: ";
                cin >> id;
                limpiarBuffer();
                
                Paciente paciente = buscarPacientePorID(id);
                if (paciente.id != -1) {
                    cout << "\nINFORMACION COMPLETA DEL PACIENTE:" << endl;
                    cout << "=========================================" << endl;
                    cout << "ID: " << paciente.id << endl;
                    cout << "Nombre: " << paciente.nombre << " " << paciente.apellido << endl;
                    cout << "Cedula: " << paciente.cedula << endl;
                    cout << "Edad: " << paciente.edad << " anos" << endl;
                    cout << "Sexo: " << (paciente.sexo == 'M' ? "Masculino" : "Femenino") << endl;
                    cout << "Tipo de sangre: " << paciente.tipoSangre << endl;
                    cout << "Telefono: " << paciente.telefono << endl;
                    cout << "Email: " << paciente.email << endl;
                    cout << "Direccion: " << paciente.direccion << endl;
                    cout << "Alergias: " << paciente.alergias << endl;
                    cout << "Observaciones: " << paciente.observaciones << endl;
                    cout << "Consultas realizadas: " << paciente.cantidadConsultas << endl;
                    cout << "Citas agendadas: " << paciente.cantidadCitas << endl;
                    
                    char fechaCreacion[20], fechaModificacion[20];
                    strftime(fechaCreacion, sizeof(fechaCreacion), "%Y-%m-%d %H:%M", 
                            localtime(&paciente.fechaCreacion));
                    strftime(fechaModificacion, sizeof(fechaModificacion), "%Y-%m-%d %H:%M", 
                            localtime(&paciente.fechaModificacion));
                    
                    cout << "Fecha de registro: " << fechaCreacion << endl;
                    cout << "Ultima modificacion: " << fechaModificacion << endl;
                    cout << "Estado: " << (paciente.eliminado ? "ELIMINADO" : "ACTIVO") << endl;
                }
                break;
            }

            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}


void menuGestionDoctores() {
    int opcion;

    do {
        system("cls");
        cout << "\n===============================================" << endl;
        cout << "           GESTION DE DOCTORES" << endl;
        cout << "===============================================" << endl;
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

        opcion = leerOpcion("Seleccione una opcion", 0, 11);

        switch (opcion) {
            case 1: {
                cout << "\n--- REGISTRAR NUEVO DOCTOR ---" << endl;
                char nombre[50], apellido[50], cedula[20], especialidad[50];
                int aniosExperiencia;
                float costoConsulta;

                cout << "Nombre: ";
                cin.getline(nombre, sizeof(nombre));
                cout << "Apellido: ";
                cin.getline(apellido, sizeof(apellido));
                cout << "Cedula: ";
                cin.getline(cedula, sizeof(cedula));
                cout << "Especialidad: ";
                cin.getline(especialidad, sizeof(especialidad));
                cout << "Anios de experiencia: ";
                cin >> aniosExperiencia;
                cout << "Costo de consulta: $";
                cin >> costoConsulta;
                limpiarBuffer();

                if (crearDoctor(nombre, apellido, cedula, especialidad, 
                              aniosExperiencia, costoConsulta)) {
                    cout << "Doctor registrado exitosamente." << endl;
                } else {
                    cout << "Error al registrar doctor." << endl;
                }
                break;
            }

            case 2: {
                cout << "\n--- BUSCAR DOCTOR POR ID ---" << endl;
                int id;
                cout << "Ingrese ID del doctor: ";
                cin >> id;
                limpiarBuffer();
                
                Doctor doctor = buscarDoctorPorID(id);
                if (doctor.id != -1) {
                    cout << "\nDOCTOR ENCONTRADO:" << endl;
                    cout << "ID: " << doctor.id << endl;
                    cout << "Nombre: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
                    cout << "Cedula: " << doctor.cedula << endl;
                    cout << "Especialidad: " << doctor.especialidad << endl;
                    cout << "Experiencia: " << doctor.aniosExperiencia << " anos" << endl;
                    cout << "Costo consulta: $" << doctor.costoConsulta << endl;
                    cout << "Telefono: " << doctor.telefono << endl;
                    cout << "Email: " << doctor.email << endl;
                    cout << "Horario: " << doctor.horarioAtencion << endl;
                    cout << "Pacientes asignados: " << doctor.cantidadPacientes << endl;
                    cout << "Citas pendientes: " << doctor.cantidadCitas << endl;
                } else {
                    cout << "Doctor no encontrado." << endl;
                }
                break;
            }

            case 3: {
                cout << "\n--- BUSCAR DOCTOR POR CEDULA ---" << endl;
                char cedula[20];
                cout << "Ingrese cedula profesional: ";
                cin.getline(cedula, sizeof(cedula));
                
                Doctor doctor = buscarDoctorPorCedula(cedula);
                if (doctor.id != -1) {
                    cout << "\nDOCTOR ENCONTRADO:" << endl;
                    cout << "ID: " << doctor.id << endl;
                    cout << "Nombre: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
                    cout << "Especialidad: " << doctor.especialidad << endl;
                } else {
                    cout << "Doctor no encontrado." << endl;
                }
                break;
            }

            case 4: {
                cout << "\n--- BUSCAR DOCTORES POR ESPECIALIDAD ---" << endl;
                char especialidad[50];
                cout << "Ingrese especialidad: ";
                cin.getline(especialidad, sizeof(especialidad));
                
                const int MAX_RESULTADOS = 10;
                Doctor resultados[MAX_RESULTADOS];
                int cantidad = buscarDoctoresPorEspecialidad(especialidad, resultados, MAX_RESULTADOS);
                
                if (cantidad > 0) {
                    cout << "\nENCONTRADOS " << cantidad << " DOCTORES EN " << especialidad << ":" << endl;
                    for (int i = 0; i < cantidad; i++) {
                        cout << i+1 << ". Dr. " << resultados[i].nombre << " " << resultados[i].apellido 
                             << " (ID: " << resultados[i].id 
                             << ", Experiencia: " << resultados[i].aniosExperiencia << " anos"
                             << ", Costo: $" << resultados[i].costoConsulta << ")" << endl;
                    }
                } else {
                    cout << "No se encontraron doctores con esa especialidad." << endl;
                }
                break;
            }

            case 5: {
                cout << "\n--- ACTUALIZAR DATOS DE DOCTOR ---" << endl;
                int id;
                cout << "Ingrese ID del doctor a actualizar: ";
                cin >> id;
                limpiarBuffer();
                
                Doctor doctor = buscarDoctorPorID(id);
                if (doctor.id != -1) {
                    cout << "Actualizando: Dr. " << doctor.nombre << " " << doctor.apellido << endl;
                    
                    char buffer[200];
                    float nuevoCosto;
                    int nuevaExp;
                    
                    cout << "Nuevo costo de consulta (actual: $" << doctor.costoConsulta << "): ";
                    cin.getline(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0) {
                        nuevoCosto = atof(buffer);
                        if (nuevoCosto > 0) doctor.costoConsulta = nuevoCosto;
                    }
                    
                    cout << "Nuevos anos de experiencia (actual: " << doctor.aniosExperiencia << "): ";
                    cin.getline(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0) {
                        nuevaExp = atoi(buffer);
                        if (nuevaExp >= 0) doctor.aniosExperiencia = nuevaExp;
                    }
                    
                    cout << "Nuevo telefono (actual: " << doctor.telefono << "): ";
                    cin.getline(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0) strcpy(doctor.telefono, buffer);
                    
                    cout << "Nuevo email (actual: " << doctor.email << "): ";
                    cin.getline(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0) strcpy(doctor.email, buffer);
                    
                    cout << "Nuevo horario (actual: " << doctor.horarioAtencion << "): ";
                    cin.getline(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0) strcpy(doctor.horarioAtencion, buffer);
                    
                    if (actualizarDoctor(doctor)) {
                        cout << "Doctor actualizado exitosamente." << endl;
                    } else {
                        cout << "Error al actualizar doctor." << endl;
                    }
                } else {
                    cout << "Doctor no encontrado." << endl;
                }
                break;
            }

            case 6: {
                cout << "\n--- LISTA DE DOCTORES ---" << endl;
                listarDoctores();
                break;
            }

            case 7: {
                cout << "\n--- ELIMINAR DOCTOR ---" << endl;
                int id;
                cout << "Ingrese ID del doctor a eliminar: ";
                cin >> id;
                limpiarBuffer();
                
                if (eliminarDoctor(id)) {
                    cout << "Doctor eliminado exitosamente." << endl;
                } else {
                    cout << "Error al eliminar doctor." << endl;
                }
                break;
            }

            case 8: {
                cout << "\n--- RESTAURAR DOCTOR ELIMINADO ---" << endl;
                int id;
                cout << "Ingrese ID del doctor a restaurar: ";
                cin >> id;
                limpiarBuffer();
                
                if (restaurarDoctor(id)) {
                    cout << "Doctor restaurado exitosamente." << endl;
                } else {
                    cout << "Error al restaurar doctor." << endl;
                }
                break;
            }

            case 9: {
                cout << "\n--- DOCTORES ELIMINADOS ---" << endl;
                listarDoctoresEliminados();
                break;
            }

            case 10: {
                cout << "\n--- ASIGNAR PACIENTE A DOCTOR ---" << endl;
                int idDoctor, idPaciente;
                cout << "ID del doctor: ";
                cin >> idDoctor;
                cout << "ID del paciente: ";
                cin >> idPaciente;
                limpiarBuffer();
                
                if (asignarPacienteADoctor(idDoctor, idPaciente)) {
                    cout << "Paciente asignado exitosamente al doctor." << endl;
                } else {
                    cout << "Error al asignar paciente." << endl;
                }
                break;
            }

            case 11: {
                cout << "\n--- PACIENTES DE UN DOCTOR ---" << endl;
                int idDoctor;
                cout << "ID del doctor: ";
                cin >> idDoctor;
                limpiarBuffer();
                
                listarPacientesDeDoctor(idDoctor);
                break;
            }

            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}

void menuGestionCitas() {
    int opcion;

    do {
        system("cls");
        cout << "\n===============================================" << endl;
        cout << "            GESTION DE CITAS" << endl;
        cout << "===============================================" << endl;
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

        opcion = leerOpcion("Seleccione una opcion", 0, 8);

        switch (opcion) {
            case 1: {
                cout << "\n--- AGENDAR NUEVA CITA ---" << endl;
                int idPaciente, idDoctor;
                char fecha[11], hora[6], motivo[150];

                cout << "ID del paciente: ";
                cin >> idPaciente;
                limpiarBuffer();
                cout << "ID del doctor: ";
                cin >> idDoctor;
                limpiarBuffer();
                cout << "Fecha (YYYY-MM-DD): ";
                cin.getline(fecha, sizeof(fecha));
                cout << "Hora (HH:MM): ";
                cin.getline(hora, sizeof(hora));
                cout << "Motivo: ";
                cin.getline(motivo, sizeof(motivo));

                if (agendarCita(idPaciente, idDoctor, fecha, hora, motivo)) {
                    cout << "Cita agendada exitosamente." << endl;
                } else {
                    cout << "Error al agendar cita." << endl;
                }
                break;
            }

            case 2: {
                cout << "\n--- CANCELAR CITA ---" << endl;
                int idCita;
                cout << "ID de la cita a cancelar: ";
                cin >> idCita;
                limpiarBuffer();

                if (cancelarCita(idCita)) {
                    cout << "Cita cancelada exitosamente." << endl;
                } else {
                    cout << "Error al cancelar cita." << endl;
                }
                break;
            }

            case 3: {
                cout << "\n--- ATENDER CITA ---" << endl;
                int idCita;
                char diagnostico[200], tratamiento[200], medicamentos[150];

                cout << "ID de la cita a atender: ";
                cin >> idCita;
                limpiarBuffer();
                cout << "Diagnostico: ";
                cin.getline(diagnostico, sizeof(diagnostico));
                cout << "Tratamiento: ";
                cin.getline(tratamiento, sizeof(tratamiento));
                cout << "Medicamentos: ";
                cin.getline(medicamentos, sizeof(medicamentos));

                if (atenderCita(idCita, diagnostico, tratamiento, medicamentos)) {
                    cout << "Cita atendida exitosamente." << endl;
                } else {
                    cout << "Error al atender cita." << endl;
                }
                break;
            }

            case 4: {
                cout << "\n--- CITAS DE UN PACIENTE ---" << endl;
                int idPaciente;
                cout << "ID del paciente: ";
                cin >> idPaciente;
                limpiarBuffer();

                int cantidad;
                Cita** citas = obtenerCitasDePaciente(idPaciente, &cantidad);
                if (citas && cantidad > 0) {
                    cout << "\nCITAS DEL PACIENTE (Total: " << cantidad << "):" << endl;
                    cout << "ID  | Fecha      | Hora  | Doctor | Estado    | Motivo" << endl;
                    cout << "----|------------|-------|--------|-----------|-------------------" << endl;
                    
                    for (int i = 0; i < cantidad; i++) {
                        Cita* c = citas[i];
                        Doctor doc = buscarDoctorPorID(c->idDoctor);
                        char nombreDoctor[100];
                        snprintf(nombreDoctor, sizeof(nombreDoctor), "Dr. %s", doc.nombre);
                        
                        cout << setw(3) << c->id << " | " 
                             << setw(10) << c->fecha << " | " 
                             << setw(5) << c->hora << " | "
                             << setw(6) << c->idDoctor << " | "
                             << setw(9) << c->estado << " | "
                             << (strlen(c->motivo) > 15 ? string(c->motivo).substr(0, 12) + "..." : c->motivo)
                             << endl;
                    }
                    
                    // Liberar memoria
                    for (int i = 0; i < cantidad; i++) {
                        delete citas[i];
                    }
                    delete[] citas;
                } else {
                    cout << "No se encontraron citas para este paciente." << endl;
                }
                break;
            }

            case 5: {
                cout << "\n--- CITAS DE UN DOCTOR ---" << endl;
                int idDoctor;
                cout << "ID del doctor: ";
                cin >> idDoctor;
                limpiarBuffer();

                int cantidad;
                Cita** citas = obtenerCitasDeDoctor(idDoctor, &cantidad);
                if (citas && cantidad > 0) {
                    cout << "\nCITAS DEL DOCTOR (Total: " << cantidad << "):" << endl;
                    cout << "ID  | Fecha      | Hora  | Paciente | Estado    | Motivo" << endl;
                    cout << "----|------------|-------|----------|-----------|-------------------" << endl;
                    
                    for (int i = 0; i < cantidad; i++) {
                        Cita* c = citas[i];
                        Paciente pac = buscarPacientePorID(c->idPaciente);
                        char nombrePaciente[100];
                        snprintf(nombrePaciente, sizeof(nombrePaciente), "%s %s", pac.nombre, pac.apellido);
                        
                        cout << setw(3) << c->id << " | " 
                             << setw(10) << c->fecha << " | " 
                             << setw(5) << c->hora << " | "
                             << setw(8) << c->idPaciente << " | "
                             << setw(9) << c->estado << " | "
                             << (strlen(c->motivo) > 15 ? string(c->motivo).substr(0, 12) + "..." : c->motivo)
                             << endl;
                    }
                    
                    // Liberar memoria
                    for (int i = 0; i < cantidad; i++) {
                        delete citas[i];
                    }
                    delete[] citas;
                } else {
                    cout << "No se encontraron citas para este doctor." << endl;
                }
                break;
            }

            case 6: {
                cout << "\n--- CITAS POR FECHA ---" << endl;
                char fecha[11];
                cout << "Fecha (YYYY-MM-DD): ";
                cin.getline(fecha, sizeof(fecha));

                int cantidad;
                Cita* citas = obtenerCitasPorFecha(fecha, &cantidad);
                if (citas && cantidad > 0) {
                    cout << "\nCITAS PARA " << fecha << " (Total: " << cantidad << "):" << endl;
                    cout << "ID  | Hora  | Paciente | Doctor | Estado    | Motivo" << endl;
                    cout << "----|-------|----------|--------|-----------|-------------------" << endl;
                    
                    for (int i = 0; i < cantidad; i++) {
                        cout << setw(3) << citas[i].id << " | " 
                             << setw(5) << citas[i].hora << " | "
                             << setw(8) << citas[i].idPaciente << " | "
                             << setw(6) << citas[i].idDoctor << " | "
                             << setw(9) << citas[i].estado << " | "
                             << (strlen(citas[i].motivo) > 15 ? string(citas[i].motivo).substr(0, 12) + "..." : citas[i].motivo)
                             << endl;
                    }
                    delete[] citas;
                } else {
                    cout << "No se encontraron citas para esta fecha." << endl;
                }
                break;
            }

            case 7: {
                cout << "\n--- CITAS PENDIENTES ---" << endl;
                listarCitasPendientes();
                break;
            }

            case 8: {
                cout << "\n--- VERIFICAR DISPONIBILIDAD ---" << endl;
                int idDoctor;
                char fecha[11], hora[6];
                
                cout << "ID del doctor: ";
                cin >> idDoctor;
                limpiarBuffer();
                cout << "Fecha (YYYY-MM-DD): ";
                cin.getline(fecha, sizeof(fecha));
                cout << "Hora (HH:MM): ";
                cin.getline(hora, sizeof(hora));

                verificarDisponibilidad(idDoctor, fecha, hora);
                break;
            }

            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}

void menuHistorialMedico() {
    int opcion;

    do {
        system("cls");
        cout << "\n===============================================" << endl;
        cout << "           HISTORIAL MEDICO" << endl;
        cout << "===============================================" << endl;
        cout << "1.  Ver historial completo de paciente" << endl;
        cout << "2.  Agregar consulta al historial" << endl;
        cout << "3.  Buscar consulta por ID" << endl;
        cout << "4.  Ver ultima consulta de paciente" << endl;
        cout << "5.  Generar reporte de historial" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;

        opcion = leerOpcion("Seleccione una opcion", 0, 5);

        switch (opcion) {
            case 1: {
                cout << "\n--- HISTORIAL COMPLETO ---" << endl;
                int idPaciente;
                cout << "ID del paciente: ";
                cin >> idPaciente;
                limpiarBuffer();
                
                mostrarHistorialMedico(idPaciente);
                break;
            }

            case 2: {
                cout << "\n--- AGREGAR CONSULTA ---" << endl;
                int idPaciente, idDoctor;
                char fecha[11], hora[6], diagnostico[200], tratamiento[200], medicamentos[150];
                float costo;

                cout << "ID del paciente: ";
                cin >> idPaciente;
                cout << "ID del doctor: ";
                cin >> idDoctor;
                limpiarBuffer();
                cout << "Fecha (YYYY-MM-DD): ";
                cin.getline(fecha, sizeof(fecha));
                cout << "Hora (HH:MM): ";
                cin.getline(hora, sizeof(hora));
                cout << "Diagnostico: ";
                cin.getline(diagnostico, sizeof(diagnostico));
                cout << "Tratamiento: ";
                cin.getline(tratamiento, sizeof(tratamiento));
                cout << "Medicamentos: ";
                cin.getline(medicamentos, sizeof(medicamentos));
                cout << "Costo: $";
                cin >> costo;
                limpiarBuffer();

                HistorialMedico nuevaConsulta;
                memset(&nuevaConsulta, 0, sizeof(HistorialMedico));
                strcpy(nuevaConsulta.fecha, fecha);
                strcpy(nuevaConsulta.hora, hora);
                strcpy(nuevaConsulta.diagnostico, diagnostico);
                strcpy(nuevaConsulta.tratamiento, tratamiento);
                strcpy(nuevaConsulta.medicamentos, medicamentos);
                nuevaConsulta.costo = costo;
                nuevaConsulta.idDoctor = idDoctor;

                if (agregarConsultaAlHistorial(idPaciente, nuevaConsulta)) {
                    cout << "Consulta agregada exitosamente al historial." << endl;
                } else {
                    cout << "Error al agregar consulta." << endl;
                }
                break;
            }

            case 3: {
                cout << "\n--- BUSCAR CONSULTA ---" << endl;
                int idConsulta;
                cout << "ID de la consulta: ";
                cin >> idConsulta;
                limpiarBuffer();
                
                HistorialMedico consulta = buscarConsultaPorID(idConsulta);
                if (consulta.id != -1) {
                    cout << "\nCONSULTA ENCONTRADA:" << endl;
                    cout << "ID: " << consulta.id << endl;
                    cout << "Paciente ID: " << consulta.pacienteID << endl;
                    cout << "Doctor ID: " << consulta.idDoctor << endl;
                    cout << "Fecha: " << consulta.fecha << " " << consulta.hora << endl;
                    cout << "Diagnostico: " << consulta.diagnostico << endl;
                    cout << "Tratamiento: " << consulta.tratamiento << endl;
                    cout << "Medicamentos: " << consulta.medicamentos << endl;
                    cout << "Costo: $" << consulta.costo << endl;
                } else {
                    cout << "Consulta no encontrada." << endl;
                }
                break;
            }

            case 4: {
                cout << "\n--- ULTIMA CONSULTA ---" << endl;
                int idPaciente;
                cout << "ID del paciente: ";
                cin >> idPaciente;
                limpiarBuffer();
                
                HistorialMedico ultima = buscarUltimaConsulta(idPaciente);
                if (ultima.id != -1) {
                    cout << "\nULTIMA CONSULTA:" << endl;
                    cout << "Fecha: " << ultima.fecha << " " << ultima.hora << endl;
                    cout << "Diagnostico: " << ultima.diagnostico << endl;
                    cout << "Tratamiento: " << ultima.tratamiento << endl;
                    cout << "Medicamentos: " << ultima.medicamentos << endl;
                    cout << "Costo: $" << ultima.costo << endl;
                } else {
                    cout << "No se encontraron consultas para este paciente." << endl;
                }
                break;
            }

            case 5: {
                cout << "\n--- REPORTE DE HISTORIAL ---" << endl;
                int idPaciente;
                cout << "ID del paciente: ";
                cin >> idPaciente;
                limpiarBuffer();
                
                // Esta funcion podria generar un reporte mas detallado
                mostrarHistorialMedico(idPaciente);
                break;
            }

            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}
void menuReportesEstadisticas() {
    int opcion;

    do {
        system("cls");
        cout << "\n===============================================" << endl;
        cout << "         REPORTES Y ESTADISTICAS" << endl;
        cout << "===============================================" << endl;
        cout << "1.  Estadisticas generales del sistema" << endl;
        cout << "2.  Reporte de pacientes por edad" << endl;
        cout << "3.  Reporte de doctores por especialidad" << endl;
        cout << "4.  Reporte de citas por mes" << endl;
        cout << "5.  Ingresos por periodo" << endl;
        cout << "6.  Top 5 doctores mas solicitados" << endl;
        cout << "7.  Pacientes con mas consultas" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;

        opcion = leerOpcion("Seleccione una opcion", 0, 7);

        switch (opcion) {
            case 1: {
                cout << "\n--- ESTADISTICAS GENERALES ---" << endl;
                
                // Leer headers para obtener estadisticas
                ArchivoHeader headerPacientes = leerHeader("pacientes.bin");
                ArchivoHeader headerDoctores = leerHeader("doctores.bin");
                ArchivoHeader headerCitas = leerHeader("citas.bin");
                ArchivoHeader headerHistorial = leerHeader("historiales.bin");

                cout << "RESUMEN DEL SISTEMA:" << endl;
                cout << "====================" << endl;
                cout << "Pacientes registrados: " << headerPacientes.registrosActivos << endl;
                cout << "Doctores registrados: " << headerDoctores.registrosActivos << endl;
                cout << "Citas en sistema: " << headerCitas.registrosActivos << endl;
                cout << "Consultas en historial: " << headerHistorial.registrosActivos << endl;
                
                // Estadisticas adicionales
                int pacientesEliminados = contarPacientesEliminados();
                int doctoresEliminados = contarDoctoresEliminados();
                
                cout << "Pacientes eliminados: " << pacientesEliminados << endl;
                cout << "Doctores eliminados: " << doctoresEliminados << endl;
                cout << "====================" << endl;
                
                break;
            }

            case 2: {
                cout << "\n--- PACIENTES POR EDAD ---" << endl;
                
                const int MAX_PACIENTES = 1000;
                Paciente pacientes[MAX_PACIENTES];
                int cantidad = leerTodosPacientes(pacientes, MAX_PACIENTES, true);
                
                if (cantidad == 0) {
                    cout << "No hay pacientes registrados." << endl;
                    break;
                }
                
                int ninos = 0, jovenes = 0, adultos = 0, mayores = 0;
                
                for (int i = 0; i < cantidad; i++) {
                    if (pacientes[i].edad < 18) ninos++;
                    else if (pacientes[i].edad < 30) jovenes++;
                    else if (pacientes[i].edad < 60) adultos++;
                    else mayores++;
                }
                
                cout << "DISTRIBUCION POR EDAD:" << endl;
                cout << "Niños (0-17): " << ninos << " pacientes" << endl;
                cout << "Jóvenes (18-29): " << jovenes << " pacientes" << endl;
                cout << "Adultos (30-59): " << adultos << " pacientes" << endl;
                cout << "Adultos mayores (60+): " << mayores << " pacientes" << endl;
                cout << "TOTAL: " << cantidad << " pacientes" << endl;
                
                break;
            }

            case 3: {
                cout << "\n--- DOCTORES POR ESPECIALIDAD ---" << endl;
                
                const int MAX_DOCTORES = 100;
                Doctor doctores[MAX_DOCTORES];
                int cantidad = leerTodosDoctores(doctores, MAX_DOCTORES, true);
                
                if (cantidad == 0) {
                    cout << "No hay doctores registrados." << endl;
                    break;
                }
                
                // Contar por especialidad
                const int MAX_ESPECIALIDADES = 20;
                char especialidades[MAX_ESPECIALIDADES][50];
                int conteo[MAX_ESPECIALIDADES] = {0};
                int totalEspecialidades = 0;
                
                for (int i = 0; i < cantidad; i++) {
                    bool encontrado = false;
                    for (int j = 0; j < totalEspecialidades; j++) {
                        if (strcmp(doctores[i].especialidad, especialidades[j]) == 0) {
                            conteo[j]++;
                            encontrado = true;
                            break;
                        }
                    }
                    if (!encontrado && totalEspecialidades < MAX_ESPECIALIDADES) {
                        strcpy(especialidades[totalEspecialidades], doctores[i].especialidad);
                        conteo[totalEspecialidades] = 1;
                        totalEspecialidades++;
                    }
                }
                
                cout << "DOCTORES POR ESPECIALIDAD:" << endl;
                cout << "==========================" << endl;
                for (int i = 0; i < totalEspecialidades; i++) {
                    cout << especialidades[i] << ": " << conteo[i] << " doctores" << endl;
                }
                cout << "TOTAL: " << cantidad << " doctores" << endl;
                
                break;
            }

            case 4: {
                cout << "\n--- CITAS POR MES ---" << endl;
                cout << "Funcion en desarrollo. Próximamente disponible." << endl;
                break;
            }

            case 5: {
                cout << "\n--- INGRESOS POR PERIODO ---" << endl;
                
                // Calcular ingresos totales del historial
                ifstream archivo("historiales.bin", ios::binary);
                if (!archivo.is_open()) {
                    cout << "No hay datos de ingresos disponibles." << endl;
                    break;
                }
                
                ArchivoHeader header = leerHeader("historiales.bin");
                if (header.registrosActivos == 0) {
                    cout << "No hay consultas registradas en el historial." << endl;
                    archivo.close();
                    break;
                }
                
                archivo.seekg(sizeof(ArchivoHeader));
                HistorialMedico consulta;
                float ingresosTotales = 0;
                int consultasCount = 0;
                
                while (archivo.read(reinterpret_cast<char*>(&consulta), sizeof(HistorialMedico))) {
                    if (!consulta.eliminado) {
                        ingresosTotales += consulta.costo;
                        consultasCount++;
                    }
                }
                archivo.close();
                
                cout << "INGRESOS TOTALES DEL SISTEMA:" << endl;
                cout << "=============================" << endl;
                cout << "Total de consultas: " << consultasCount << endl;
                cout << "Ingresos totales: $" << fixed << setprecision(2) << ingresosTotales << endl;
                cout << "Promedio por consulta: $" << (consultasCount > 0 ? ingresosTotales / consultasCount : 0) << endl;
                
                break;
            }

            case 6: {
                cout << "\n--- TOP 5 DOCTORES MAS SOLICITADOS ---" << endl;
                
                // Contar citas por doctor
                const int MAX_DOCTORES = 100;
                Doctor doctores[MAX_DOCTORES];
                int cantidadDoctores = leerTodosDoctores(doctores, MAX_DOCTORES, true);
                
                if (cantidadDoctores == 0) {
                    cout << "No hay doctores registrados." << endl;
                    break;
                }
                
                // Inicializar contadores
                int citasPorDoctor[MAX_DOCTORES] = {0};
                
                // Contar citas para cada doctor
                ifstream archivoCitas("citas.bin", ios::binary);
                if (archivoCitas.is_open()) {
                    ArchivoHeader headerCitas = leerHeader("citas.bin");
                    archivoCitas.seekg(sizeof(ArchivoHeader));
                    
                    Cita cita;
                    for (int i = 0; i < headerCitas.cantidadRegistros; i++) {
                        if (archivoCitas.read(reinterpret_cast<char*>(&cita), sizeof(Cita))) {
                            if (!cita.eliminado && strcmp(cita.estado, "AGENDADA") == 0) {
                                for (int j = 0; j < cantidadDoctores; j++) {
                                    if (doctores[j].id == cita.idDoctor) {
                                        citasPorDoctor[j]++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    archivoCitas.close();
                }
                
                // Ordenar doctores por número de citas (burbuja simple)
                for (int i = 0; i < cantidadDoctores - 1; i++) {
                    for (int j = 0; j < cantidadDoctores - i - 1; j++) {
                        if (citasPorDoctor[j] < citasPorDoctor[j + 1]) {
                            // Intercambiar conteos
                            int tempCont = citasPorDoctor[j];
                            citasPorDoctor[j] = citasPorDoctor[j + 1];
                            citasPorDoctor[j + 1] = tempCont;
                            
                            // Intercambiar doctores
                            Doctor tempDoc = doctores[j];
                            doctores[j] = doctores[j + 1];
                            doctores[j + 1] = tempDoc;
                        }
                    }
                }
                
                cout << "TOP 5 DOCTORES MAS SOLICITADOS:" << endl;
                cout << "===============================" << endl;
                int limite = (cantidadDoctores < 5) ? cantidadDoctores : 5;
                for (int i = 0; i < limite; i++) {
                    cout << i+1 << ". Dr. " << doctores[i].nombre << " " << doctores[i].apellido 
                         << " (" << doctores[i].especialidad << ")" 
                         << " - " << citasPorDoctor[i] << " citas" << endl;
                }
                
                break;
            }

            case 7: {
                cout << "\n--- PACIENTES CON MAS CONSULTAS ---" << endl;
                
                const int MAX_PACIENTES = 1000;
                Paciente pacientes[MAX_PACIENTES];
                int cantidad = leerTodosPacientes(pacientes, MAX_PACIENTES, true);
                
                if (cantidad == 0) {
                    cout << "No hay pacientes registrados." << endl;
                    break;
                }
                
                // Ordenar pacientes por número de consultas (burbuja simple)
                for (int i = 0; i < cantidad - 1; i++) {
                    for (int j = 0; j < cantidad - i - 1; j++) {
                        if (pacientes[j].cantidadConsultas < pacientes[j + 1].cantidadConsultas) {
                            Paciente temp = pacientes[j];
                            pacientes[j] = pacientes[j + 1];
                            pacientes[j + 1] = temp;
                        }
                    }
                }
                
                cout << "PACIENTES CON MAS CONSULTAS:" << endl;
                cout << "============================" << endl;
                int limite = (cantidad < 10) ? cantidad : 10;
                for (int i = 0; i < limite; i++) {
                    if (pacientes[i].cantidadConsultas > 0) {
                        cout << i+1 << ". " << pacientes[i].nombre << " " << pacientes[i].apellido 
                             << " - " << pacientes[i].cantidadConsultas << " consultas" << endl;
                    }
                }
                
                if (limite == 0) {
                    cout << "No hay pacientes con consultas registradas." << endl;
                }
                
                break;
            }

            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}
void menuMantenimientoSistema() {
    int opcion;

    do {
        system("cls");
        cout << "\n===============================================" << endl;
        cout << "         MANTENIMIENTO DEL SISTEMA" << endl;
        cout << "===============================================" << endl;
        cout << "1.  Verificar integridad de archivos" << endl;
        cout << "2.  Reconstruir archivos dañados" << endl;
        cout << "3.  Respaldar datos" << endl;
        cout << "4.  Restaurar respaldo" << endl;
        cout << "5.  Limpiar registros eliminados" << endl;
        cout << "6.  Mostrar informacion de archivos" << endl;
        cout << "7.  Reindexar archivos" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;

        opcion = leerOpcion("Seleccione una opcion", 0, 7);

        switch (opcion) {
            case 1: {
                cout << "\n--- VERIFICAR INTEGRIDAD ---" << endl;
                cout << "Verificando archivos del sistema..." << endl;
                
                const char* archivos[] = {
                    "pacientes.bin", "doctores.bin", "citas.bin", "historiales.bin"
                };
                
                bool todosValidos = true;
                for (int i = 0; i < 4; i++) {
                    cout << "Verificando " << archivos[i] << "... ";
                    if (verificarArchivo(archivos[i])) {
                        cout << "OK" << endl;
                    } else {
                        cout << "PROBLEMAS" << endl;
                        todosValidos = false;
                    }
                }
                
                if (todosValidos) {
                    cout << "Todos los archivos estan en buen estado." << endl;
                } else {
                    cout << "Algunos archivos tienen problemas." << endl;
                }
                break;
            }

            case 2: {
                cout << "\n--- RECONSTRUIR ARCHIVOS ---" << endl;
                cout << "Esta funcion reconstruira archivos danados." << endl;
                cout << "¿Esta seguro? (s/n): ";
                char confirmacion;
                cin >> confirmacion;
                limpiarBuffer();
                
                if (tolower(confirmacion) == 's') {
                    cout << "Reconstruyendo archivos..." << endl;
                    // Aqui iria la logica para reconstruir archivos
                    cout << "Funcion de reconstruccion en desarrollo." << endl;
                } else {
                    cout << "Operacion cancelada." << endl;
                }
                break;
            }

            case 3: {
                cout << "\n--- RESPALDAR DATOS ---" << endl;
                cout << "Generando respaldo de datos..." << endl;
                // Aqui iria la logica para respaldar
                cout << "Funcion de respaldo en desarrollo." << endl;
                break;
            }

            case 4: {
                cout << "\n--- RESTAURAR RESPALDO ---" << endl;
                cout << "Restaurando datos desde respaldo..." << endl;
                // Aqui iria la logica para restaurar
                cout << "Funcion de restauracion en desarrollo." << endl;
                break;
            }

            case 5: {
                cout << "\n--- LIMPIAR REGISTROS ELIMINADOS ---" << endl;
                cout << "Esta operacion eliminara permanentemente los registros marcados como eliminados." << endl;
                cout << "¿Esta seguro? (s/n): ";
                char confirmacion;
                cin >> confirmacion;
                limpiarBuffer();
                
                if (tolower(confirmacion) == 's') {
                    cout << "Limpiando registros eliminados..." << endl;
                    // Aqui iria la logica para limpiar registros eliminados
                    cout << "Funcion de limpieza en desarrollo." << endl;
                } else {
                    cout << "Operacion cancelada." << endl;
                }
                break;
            }

            case 6: {
                cout << "\n--- INFORMACION DE ARCHIVOS ---" << endl;
                cout << "Mostrando informacion de archivos..." << endl;
                
                const char* archivos[] = {
                    "pacientes.bin", "doctores.bin", "citas.bin", "historiales.bin"
                };
                
                for (int i = 0; i < 4; i++) {
                    cout << "\n--- " << archivos[i] << " ---" << endl;
                    leerHeader(archivos[i], true);
                }
                break;
            }

            case 7: {
                cout << "\n--- REINDEXAR ARCHIVOS ---" << endl;
                cout << "Reindexando archivos para optimizar el rendimiento..." << endl;
                // Aqui iria la logica para reindexar
                cout << "Funcion de reindexacion en desarrollo." << endl;
                break;
            }

            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}
void menuReportesEstadisticas();
int main() {
  
    cout << "==========================================" << endl;
    cout << "    SISTEMA HOSPITALARIO - VERSION ARCHIVOS" << endl;
    cout << "==========================================" << endl;
    
    if (!inicializarSistemaArchivos()) {
        cout << "Advertencia: Hubo problemas al inicializar los archivos." << endl;
        cout << "El sistema puede no funcionar correctamente." << endl;
        pausarPantalla();
    }

    int opcion;

    do {
        system("cls");
        mostrarMenuPrincipal();
        opcion = leerOpcion("Seleccione una opcion", 0, 6);

        switch (opcion) {
            case 1:
                menuGestionPacientes();
                break;
            case 2:
                menuGestionDoctores();
                break;
            case 3:
                menuGestionCitas();
                break;
            case 4:
                menuHistorialMedico();
                break;
            case 5:
                menuReportesEstadisticas();
                break;
            case 6:
                menuMantenimientoSistema();
                break;
            case 0:
                cout << "Saliendo del programa..." << endl;
                break;
            default:
                cout << "Opcion no valida." << endl;
                pausarPantalla();
        }

    } while (opcion != 0);

    cout << "Programa terminado correctamente." << endl;
    return 0;
}
