#include <iostream>

#include <locale>

#include <iomanip>

#include <cstring>

#include <ctime>

#include <cctype>

#include <limits>

#include <thread>

#include <chrono>

#include <cstdlib>

struct Paciente;
struct Doctor;
struct Cita;
struct HistorialMedico;

using namespace std;

const char * obtenerNombreMes(int mes);
void limpiarBuffer();
void pausarPantalla();

struct ArchivoHeader {
    int cantidadRegistros;     // Total de registros físicos
    int proximoID;            // Siguiente ID disponible  
    int registrosActivos;     // Registros no eliminados
    int version;              // Versión del formato
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
    char tipoArchivo[20];     // "PACIENTES", "DOCTORES", etc.
    
    // Para optimizar listas enlazadas
    int ultimoIDUtilizado;    // Último ID asignado (para evitar saltos)
};
//strucs de hospital, paciente, historial medico, doctor y cita

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

    int * pacientesAsignados;
    int cantidadPacientes;
    int capacidadPacientes;

    int * citasAgendadas;
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
    
    cout << "Archivo inicializado: " << nombreArchivo 
         << " (Tipo: " << tipo << ")" << endl;
    return true;
}

// Función auxiliar para determinar tipo de archivo
const char* determinarTipoArchivo(const char* nombreArchivo) {
    if (strstr(nombreArchivo, "pacientes") != nullptr) return "PACIENTES";
    if (strstr(nombreArchivo, "doctores") != nullptr) return "DOCTORES";
    if (strstr(nombreArchivo, "citas") != nullptr) return "CITAS";
    if (strstr(nombreArchivo, "historiales") != nullptr) return "HISTORIALES";
    if (strstr(nombreArchivo, "hospital") != nullptr) return "HOSPITAL";
    return "GENERAL";
}

bool verificarArchivo(const char* nombreArchivo) {
    //  Intentar abrir archivo para lectura
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
             << " está corrupto (tamaño insuficiente: " << fileSize << " bytes)" << endl;
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
        errores += "Versión inválida (" + to_string(header.version) + "), esperada 1. ";
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
    
    if (header.registrosActivos < 0 || header.registrosActivos > header.cantidadRegistros) {
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
    if (strlen(header.tipoArchivo) == 0 || strlen(header.tipoArchivo) >= sizeof(header.tipoArchivo)) {
        errores += "tipoArchivo inválido. ";
        valido = false;
    }
    
    // Mostrar resultado
    if (valido) {
        cout << "? Archivo válido: " << nombreArchivo << endl;
        cout << "   Tipo: " << header.tipoArchivo 
             << " | Registros: " << header.registrosActivos << "/" << header.cantidadRegistros
             << " | Próximo ID: " << header.proximoID << endl;
    } else {
        cout << "? Archivo inválido: " << nombreArchivo << " - " << errores << endl;
    }
    
    return valido;
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
    long tamaño = archivo.tellg();
    archivo.seekg(0, ios::beg);
    
    if (tamaño < static_cast<long>(sizeof(ArchivoHeader))) {
        if (mostrarInfo) {
            cout << "? Tamaño insuficiente: " << nombreArchivo 
                 << " (" << tamaño << "/" << sizeof(ArchivoHeader) << " bytes)" << endl;
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
        cout << "   +-- Registros: " << header.registrosActivos << "/" << header.cantidadRegistros << endl;
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

bool actualizarHeader(const char* nombreArchivo, ArchivoHeader header) {
    // 1. Verificar que el archivo existe y es accesible
    if (!archivoExiste(nombreArchivo)) {
        cout << "? Error: " << nombreArchivo << " no existe" << endl;
        return false;
    }
    
    // 2. Abrir en modo lectura/escritura
    fstream archivo(nombreArchivo, ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "? Error: No se puede abrir " << nombreArchivo << " para escritura" << endl;
        return false;
    }
    
    // 3. Validar header antes de escribir
    if (header.proximoID < 1) {
        cout << "??  Advertencia: próximoID inválido (" << header.proximoID << "), ajustando a 1" << endl;
        header.proximoID = 1;
    }
    
    if (header.registrosActivos < 0) {
        cout << "??  Advertencia: registrosActivos negativo, ajustando a 0" << endl;
        header.registrosActivos = 0;
    }
    
    if (header.registrosActivos > header.cantidadRegistros) {
        cout << "??  Advertencia: registrosActivos > cantidadRegistros, ajustando" << endl;
        header.registrosActivos = header.cantidadRegistros;
    }
    
    // 4. Actualizar metadata
    header.fechaUltimaModificacion = time(nullptr);
    
    // 5. Posicionarse al inicio y escribir
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    
    // 6. Forzar escritura a disco
    archivo.flush();
    
    bool exito = !archivo.fail();
    archivo.close();
    
    if (exito) {
        cout << "? Header actualizado: " << nombreArchivo << endl;
        cout << "   ?? Nuevos valores - Registros: " << header.registrosActivos 
             << "/" << header.cantidadRegistros << " | Próximo ID: " << header.proximoID << endl;
    } else {
        cout << "? Error crítico: No se pudo escribir header en " << nombreArchivo << endl;
    }
    
    return exito;
}










//funcion para liberar memoria del hospital

void liberarHospital(Hospital * hospital) {

    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        delete[] hospital -> pacientes[i].historial;
        delete[] hospital -> pacientes[i].citasAgendadas;
    }

    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        delete[] hospital -> doctores[i].pacientesAsignados;
        delete[] hospital -> doctores[i].citasAgendadas;
    }

    delete[] hospital -> pacientes;
    delete[] hospital -> doctores;
    delete[] hospital -> citas;

    delete hospital;

    cout << "Memoria del hospital liberada correctamente" << endl;
}

Paciente * buscarPacientePorCedula(Hospital * hospital,
    const char * cedula) {
    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        const char * cedulaPaciente = hospital -> pacientes[i].cedula;

        if (strcmp(cedulaPaciente, cedula) == 0) {
            return & hospital -> pacientes[i];
        }
    }

    return nullptr;
}

Paciente buscarPacientePorID(int id) {
    Paciente paciente;
    // Inicializar paciente vacío en caso de no encontrarlo
    memset(&paciente, 0, sizeof(Paciente));
    paciente.id = -1; // Marcar como no encontrado
    
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
    
    //  Cerrar archivo
    archivo.close();
    
    //  Retornar paciente encontrado (o paciente vacío si no existe)
    if (!encontrado) {
        memset(&paciente, 0, sizeof(Paciente));
        paciente.id = -1;
        cout << "? Paciente con ID " << id << " no encontrado" << endl;
    } else {
        cout << "? Paciente encontrado: " << paciente.nombre << " " << paciente.apellido << endl;
    }
    
    return paciente;
}

bool agregarPaciente(Paciente nuevoPaciente) {
    // Validaciones iniciales
    if (strlen(nuevoPaciente.nombre) == 0 || strlen(nuevoPaciente.apellido) == 0) {
        cout << "? Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }
    
    if (strlen(nuevoPaciente.cedula) == 0) {
        cout << "? Error: Cédula es obligatoria" << endl;
        return false;
    }
    
    // Verificar que no exista paciente con misma cédula
    Paciente existente = buscarPacientePorCedula(nuevoPaciente.cedula);
    if (existente.id != -1) {
        cout << "? Error: Ya existe un paciente con cédula " << nuevoPaciente.cedula << endl;
        return false;
    }
    
    //  Leer header actual
    ArchivoHeader header;
    if (!leerHeader("pacientes.bin", header)) {
        cout << "? Error: No se puede leer header de pacientes.bin" << endl;
        return false;
    }
    
    //  Asignar ID y preparar paciente
    nuevoPaciente.id = header.proximoID;
    inicializarPaciente(nuevoPaciente);
    
    // Abrir en modo append
    ofstream archivo("pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "? Error: No se puede abrir pacientes.bin en modo append" << endl;
        return false;
    }
    
    //  Escribir registro
    archivo.write(reinterpret_cast<const char*>(&nuevoPaciente), sizeof(Paciente));
    bool exitoEscritura = archivo.good();
    archivo.close();
    
    if (!exitoEscritura) {
        cout << "? Error crítico: Fallo en escritura de paciente" << endl;
        return false;
    }
    
    //  Actualizar header
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    header.fechaUltimaModificacion = time(nullptr);
    
    // Actualizar header en archivo
    if (actualizarHeader("pacientes.bin", header)) {
        cout << "? PACIENTE AGREGADO EXITOSAMENTE" << endl;
        mostrarResumenPaciente(nuevoPaciente);
        return true;
    } else {
        cout << "? ERROR: Paciente escrito pero header corrupto" << endl;
        return false;
    }
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
        cout << "??  Edad inválida (" << paciente.edad << "), ajustando a 0" << endl;
        paciente.edad = 0;
    }
    
    // Asegurar que sexo sea válido
    if (paciente.sexo != 'M' && paciente.sexo != 'F') {
        cout << " Sexo inválido (" << paciente.sexo << "), ajustando a 'M'" << endl;
        paciente.sexo = 'M';
    }
}

void mostrarResumenPaciente(const Paciente& paciente) {
    cout << "    RESUMEN DEL PACIENTE:" << endl;
    cout << "   +-- ID: " << paciente.id << endl;
    cout << "   +-- Nombre: " << paciente.nombre << " " << paciente.apellido << endl;
    cout << "   +-- Cédula: " << paciente.cedula << endl;
    cout << "   +-- Edad: " << paciente.edad << " años" << endl;
    cout << "   +-- Sexo: " << (paciente.sexo == 'M' ? "Masculino" : "Femenino") << endl;
    cout << "   +-- Teléfono: " << paciente.telefono << endl;
    
    // Mostrar fecha de creación formateada
    char fechaStr[20];
    strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d %H:%M", 
             localtime(&paciente.fechaCreacion));
    cout << "    Registrado: " << fechaStr << endl;
}

Paciente leerPacientePorIndice(int indice) {
    Paciente p;
    memset(&p, 0, sizeof(Paciente));
    p.id = -1; // Marcador de error
    
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
        cout << " Error: Índice " << indice << " fuera de rango (máximo: " 
             << header.cantidadRegistros - 1 << ")" << endl;
        archivo.close();
        return p;
    }
    
    //  Calcular posición exacta
    long posicion = calcularPosicion(indice);
    
    //. Ir directamente a esa posición
    archivo.seekg(posicion);
    if (archivo.fail()) {
        cout << "? Error: No se puede posicionar en índice " << indice << endl;
        archivo.close();
        return p;
    }
    
    //  Leer estructura completa
    archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente));
    
    if (archivo.fail()) {
        cout << "? Error: Fallo al leer paciente en índice " << indice << endl;
        memset(&p, 0, sizeof(Paciente));
        p.id = -1;
    } else if (p.eliminado) {
        cout << " Paciente en índice " << indice << " está eliminado" << endl;
        p.id = -2; // Código especial para eliminado
    } else {
        cout << " Paciente leído por índice " << indice << ": " 
             << p.nombre << " " << p.apellido << endl;
    }
    
    archivo.close();
    return p;
}

// Función auxiliar para calcular posición
long calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
}

Paciente leerPacientePorID(int id) {
    
    return buscarPacientePorID(id);
}












Paciente ** buscarPacientesPorNombre(Hospital * hospital,
    const char * nombre, int * cantidad) {
    if (!hospital || !nombre || !cantidad) {
        if (cantidad) * cantidad = 0;
        return nullptr;
    }

    Paciente ** resultados = new Paciente * [hospital -> cantidadPacientes];
    * cantidad = 0;

    char * nombreLower = new char[strlen(nombre) + 1];
    for (int i = 0; nombre[i]; i++) {
        nombreLower[i] = tolower(nombre[i]);
    }
    nombreLower[strlen(nombre)] = '\0';

    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        char * nombrePacienteLower = new char[strlen(hospital -> pacientes[i].nombre) + 1];
        for (int j = 0; hospital -> pacientes[i].nombre[j]; j++) {
            nombrePacienteLower[j] = tolower(hospital -> pacientes[i].nombre[j]);
        }
        nombrePacienteLower[strlen(hospital -> pacientes[i].nombre)] = '\0';
        if (strstr(nombrePacienteLower, nombreLower) != nullptr) {
            resultados[ * cantidad] = & hospital -> pacientes[i];
            ( * cantidad) ++;
        }

        delete[] nombrePacienteLower;
    }

    delete[] nombreLower;

    if ( * cantidad == 0) {
        delete[] resultados;
        return nullptr;
    }

    return resultados;
}

void liberarResultadosBusqueda(Paciente ** resultados) {
    if (resultados) {
        delete[] resultados;
    }
}

Paciente * crearPaciente(Hospital * hospital,
    const char * nombre,
        const char * apellido,
            const char * cedula,
                int edad, char sexo) {

    int indice = hospital -> cantidadPacientes;
    hospital -> pacientes[indice].id = hospital -> siguienteIdPaciente++;
    strcpy(hospital -> pacientes[indice].nombre, nombre);
    strcpy(hospital -> pacientes[indice].apellido, apellido);
    strcpy(hospital -> pacientes[indice].cedula, cedula);
    hospital -> pacientes[indice].edad = edad;
    hospital -> pacientes[indice].sexo = sexo;
    hospital -> pacientes[indice].capacidadHistorial = 5;
    hospital -> pacientes[indice].historial = new HistorialMedico[5];
    hospital -> pacientes[indice].cantidadConsultas = 0;

    if (!hospital -> pacientes[indice].historial) {
        cout << "Error crÃ­tico: No se pudo asignar memoria para historial mÃ©dico" << endl;
        return nullptr;
    }

    hospital -> pacientes[indice].capacidadCitas = 5;
    hospital -> pacientes[indice].citasAgendadas = new int[5];
    hospital -> pacientes[indice].cantidadCitas = 0;

    if (!hospital -> pacientes[indice].citasAgendadas) {
        cout << "Error crÃ­tico: No se pudo asignar memoria para citas agendadas" << endl;
        delete[] hospital -> pacientes[indice].historial; // Liberar memoria ya asignada
        return nullptr;
    }

    // 
    for (int i = 0; i < 5; i++) {
        hospital -> pacientes[indice].historial[i].idConsulta = 0;
        strcpy(hospital -> pacientes[indice].historial[i].fecha, "");
        strcpy(hospital -> pacientes[indice].historial[i].hora, "");
        strcpy(hospital -> pacientes[indice].historial[i].diagnostico, "");
        strcpy(hospital -> pacientes[indice].historial[i].tratamiento, "");
        strcpy(hospital -> pacientes[indice].historial[i].medicamentos, "");
        hospital -> pacientes[indice].historial[i].idDoctor = 0;
        hospital -> pacientes[indice].historial[i].costo = 0.0 f;
    }

    for (int i = 0; i < 5; i++) {
        hospital -> pacientes[indice].citasAgendadas[i] = 0;
    }

    strcpy(hospital -> pacientes[indice].tipoSangre, "");
    strcpy(hospital -> pacientes[indice].telefono, "");
    strcpy(hospital -> pacientes[indice].direccion, "");
    strcpy(hospital -> pacientes[indice].email, "");
    strcpy(hospital -> pacientes[indice].alergias, "");
    strcpy(hospital -> pacientes[indice].observaciones, "");

    hospital -> pacientes[indice].activo = true;
    hospital -> cantidadPacientes++;

    cout << "   Paciente creado exitosamente:" << endl;
    cout << "   ID: " << hospital -> pacientes[indice].id << endl;
    cout << "   Nombre: " << hospital -> pacientes[indice].nombre << " " << hospital -> pacientes[indice].apellido << endl;
    cout << "   CÃ©dula: " << hospital -> pacientes[indice].cedula << endl;
    cout << "   Edad: " << hospital -> pacientes[indice].edad << endl;
    cout << "   Sexo: " << hospital -> pacientes[indice].sexo << endl;

    return & hospital -> pacientes[indice];
}
bool actualizarPaciente(Hospital * hospital, int id) {

    Paciente * paciente = buscarPacientePorId(hospital, id);

    if (!paciente) {
        cout << "Error: No se encontrÃ³ paciente con ID " << id << endl;
        return false;
    }

    cout << "\n=== ACTUALIZAR DATOS DEL PACIENTE ===" << endl;
    cout << "Paciente: " << paciente -> nombre << " " << paciente -> apellido << endl;
    cout << "ID: " << paciente -> id << endl;
    cout << "Deje en blanco para mantener el valor actual\n" << endl;

    char buffer[200];

    cout << "Nombre actual: " << paciente -> nombre << endl;
    cout << "Nuevo nombre: ";
    cin.ignore();
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
            strcpy(paciente -> nombre, buffer);
        }
    }

    cout << "Apellido actual: " << paciente -> apellido << endl;
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
            strcpy(paciente -> apellido, buffer);
        }
    }

    cout << "Edad actual: " << paciente -> edad << endl;
    cout << "Nueva edad: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        int nuevaEdad = atoi(buffer);
        if (nuevaEdad > 0 && nuevaEdad <= 150) {
            paciente -> edad = nuevaEdad;
        } else {
            cout << "Edad no vÃ¡lida. Manteniendo valor actual." << endl;
        }
    }

    cout << "Sexo actual: " << paciente -> sexo << endl;
    cout << "Nuevo sexo (M/F): ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        char nuevoSexo = toupper(buffer[0]);
        if (nuevoSexo == 'M' || nuevoSexo == 'F') {
            paciente -> sexo = nuevoSexo;
        } else {
            cout << "Sexo no vÃ¡lido. Use M o F. Manteniendo valor actual." << endl;
        }
    }

    cout << "TelÃ©fono actual: " << paciente -> telefono << endl;
    cout << "Nuevo telÃ©fono: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente -> telefono, buffer);
    }

    cout << "DirecciÃ³n actual: " << paciente -> direccion << endl;
    cout << "Nueva direcciÃ³n: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente -> direccion, buffer);
    }

    cout << "Email actual: " << paciente -> email << endl;
    cout << "Nuevo email: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente -> email, buffer);
    }

    cout << "Tipo de sangre actual: " << paciente -> tipoSangre << endl;
    cout << "Nuevo tipo de sangre: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente -> tipoSangre, buffer);
    }

    cout << "Alergias actuales: " << paciente -> alergias << endl;
    cout << "Nuevas alergias: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente -> alergias, buffer);
    }

    cout << "Observaciones actuales: " << paciente -> observaciones << endl;
    cout << "Nuevas observaciones: ";
    cin.getline(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        strcpy(paciente -> observaciones, buffer);
    }

    cout << "\nDatos del paciente actualizados correctamente." << endl;
    return true;
}

bool eliminarPaciente(Hospital * hospital, int id) {

    int indice = -1;
    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        if (hospital -> pacientes[i].id == id) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        cout << "Error: No se encontrÃ³ paciente con ID " << id << endl;
        return false;
    }

    Paciente * paciente = & hospital -> pacientes[indice];

    // Liberar memoria de los arregloss dinÃ¡micos del paciente
    delete[] paciente -> historial;
    delete[] paciente -> citasAgendadas;

    // Eliminar o cancelar todas las citas asociadas al paciente
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idPaciente == id) {
            strcpy(hospital -> citas[i].estado, "CANCELADA");
            hospital -> citas[i].atendida = false;

            // TambiÃ©n remover esta cita de los doctores
            for (int j = 0; j < hospital -> cantidadDoctores; j++) {
                if (hospital -> doctores[j].id == hospital -> citas[i].idDoctor) {
                    // Buscar y eliminar la cita del array del doctor
                    for (int k = 0; k < hospital -> doctores[j].cantidadCitas; k++) {
                        if (hospital -> doctores[j].citasAgendadas[k] == hospital -> citas[i].id) {
                            // Mover citas restantes hacia adelante
                            for (int l = k; l < hospital -> doctores[j].cantidadCitas - 1; l++) {
                                hospital -> doctores[j].citasAgendadas[l] = hospital -> doctores[j].citasAgendadas[l + 1];
                            }
                            hospital -> doctores[j].cantidadCitas--;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        for (int j = 0; j < hospital -> doctores[i].cantidadPacientes; j++) {
            if (hospital -> doctores[i].pacientesAsignados[j] == id) {

                for (int k = j; k < hospital -> doctores[i].cantidadPacientes - 1; k++) {
                    hospital -> doctores[i].pacientesAsignados[k] = hospital -> doctores[i].pacientesAsignados[k + 1];
                }
                hospital -> doctores[i].cantidadPacientes--;
                break;
            }
        }
    }

    for (int i = indice; i < hospital -> cantidadPacientes - 1; i++) {
        hospital -> pacientes[i] = hospital -> pacientes[i + 1];
    }
    hospital -> cantidadPacientes--;

    cout << "Paciente con ID " << id << " eliminado correctamente." << endl;
    cout << "Citas asociadas canceladas y referencias eliminadas." << endl;

    return true;
}

void listarPacientes(Hospital * hospital) {
    if (hospital -> cantidadPacientes == 0) {
        cout << "No hay pacientes registrados." << endl;
        return;
    }

    cout << "\nLISTA DE PACIENTES (" << hospital -> cantidadPacientes << "):" << endl;
    cout << "+--------------------------------------------------------------------+" << endl;
    cout << "Â¦  ID  Â¦       NOMBRE COMPLETO    Â¦    CÃ‰DULA    Â¦ EDAD Â¦ CONSULTAS  Â¦" << endl;
    cout << "+------+--------------------------+--------------+------+------------Â¦" << endl;

    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        Paciente * p = & hospital -> pacientes[i];

        char nombreCompleto[100];
        snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", p -> nombre, p -> apellido);

        if (strlen(nombreCompleto) > 22) {
            nombreCompleto[19] = '.';
            nombreCompleto[20] = '.';
            nombreCompleto[21] = '.';
            nombreCompleto[22] = '\0';
        }

        printf("Â¦ %4d Â¦ %-24s Â¦ %-12s Â¦ %4d Â¦ %10d Â¦\n",
            p -> id, nombreCompleto, p -> cedula, p -> edad, p -> cantidadConsultas);
    }

    cout << "+--------------------------------------------------------------------+" << endl;

}

void agregarConsultaAlHistorial(Paciente * paciente, HistorialMedico consulta) {

    if (paciente -> cantidadConsultas >= paciente -> capacidadHistorial) {

        int nuevaCapacidad = paciente -> capacidadHistorial * 2;

        HistorialMedico * nuevoHistorial = new HistorialMedico[nuevaCapacidad];

        for (int i = 0; i < paciente -> cantidadConsultas; i++) {
            nuevoHistorial[i] = paciente -> historial[i];
        }

        delete[] paciente -> historial;

        paciente -> historial = nuevoHistorial;
        paciente -> capacidadHistorial = nuevaCapacidad;

        cout << "Historial mÃ©dico redimensionado. Nueva capacidad: " << nuevaCapacidad << endl;
    }

    paciente -> historial[paciente -> cantidadConsultas] = consulta;

    paciente -> cantidadConsultas++;

    cout << "Consulta agregada al historial. Total de consultas: " << paciente -> cantidadConsultas << endl;
}

HistorialMedico * obtenerHistorialCompleto(Paciente * paciente, int * cantidad) {
    if (!paciente || !cantidad) {
        if (cantidad) * cantidad = 0;
        return nullptr;
    }
    * cantidad = paciente -> cantidadConsultas;
    return paciente -> historial;
}

void mostrarHistorialMedico(Paciente * paciente) {
    if (!paciente) {
        cout << "Error: Paciente no vÃ¡lido." << endl;
        return;
    }

    if (paciente -> cantidadConsultas == 0) {
        cout << "El paciente no tiene consultas en su historial mÃ©dico." << endl;
        return;
    }

    cout << "\n=== HISTORIAL MÃ‰DICO ===" << endl;
    cout << "Paciente: " << paciente -> nombre << " " << paciente -> apellido << endl;
    cout << "CÃ©dula: " << paciente -> cedula << " | Edad: " << paciente -> edad << endl;
    cout << "Total de consultas: " << paciente -> cantidadConsultas << endl;
    cout << endl;

    cout << left << setw(12) << "FECHA" <<
        setw(8) << "HORA" <<
        setw(25) << "DIAGNÃ“STICO" <<
        setw(8) << "DOCTOR" <<
        setw(10) << "COSTO" <<
        endl;

    cout << string(65, '-') << endl;

    int cantidad;
    HistorialMedico * historial = obtenerHistorialCompleto(paciente, & cantidad);

    for (int i = 0; i < cantidad; i++) {

        char diagnosticoMostrar[26];
        strcpy(diagnosticoMostrar, historial[i].diagnostico);
        if (strlen(diagnosticoMostrar) > 23) {
            diagnosticoMostrar[20] = '.';
            diagnosticoMostrar[21] = '.';
            diagnosticoMostrar[22] = '.';
            diagnosticoMostrar[23] = '\0';
        }

        cout << left << setw(12) << historial[i].fecha <<
            setw(8) << historial[i].hora <<
            setw(25) << diagnosticoMostrar <<
            setw(8) << historial[i].idDoctor <<
            "$" << setw(9) << fixed << setprecision(2) << historial[i].costo <<
            endl;
    }

    cout << string(65, '-') << endl;

    float costoTotal = 0;
    for (int i = 0; i < cantidad; i++) {
        costoTotal += historial[i].costo;
    }
    cout << "Costo total del historial: $" << fixed << setprecision(2) << costoTotal << endl;
}

HistorialMedico * obtenerUltimaConsulta(Paciente * paciente) {
    if (!paciente || paciente -> cantidadConsultas == 0) {
        return nullptr;
    }
    return & paciente -> historial[paciente -> cantidadConsultas - 1];
}

//MODULO DE DOCTORES

Doctor * crearDoctor(Hospital * hospital,
    const char * nombre,
        const char * apellido,
            const char * cedula,
                const char * especialidad, int aniosExperiencia,
                    float costoConsulta) {

    if (!hospital || !nombre || !apellido || !cedula || !especialidad) {
        cout << "Error: ParÃ¡metros invÃ¡lidos." << endl;
        return nullptr;
    }

    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        if (strcmp(hospital -> doctores[i].cedula, cedula) == 0) {
            cout << "Error: Ya existe un doctor con la misma cÃ©dula " << cedula << endl;
            return nullptr;
        }
    }

    if (aniosExperiencia < 0) {
        cout << "Error: Los anios de experiencia no pueden ser negativos." << endl;
        return nullptr;
    }

    if (costoConsulta < 0) {
        cout << "Error: El costo de consulta no puede ser negativo." << endl;
        return nullptr;
    }

    if (hospital -> cantidadDoctores >= hospital -> capacidadDoctores) {
        int nuevaCapacidad = hospital -> capacidadDoctores * 2;
        Doctor * nuevosDoctores = new Doctor[nuevaCapacidad];

        for (int i = 0; i < hospital -> cantidadDoctores; i++) {
            nuevosDoctores[i] = hospital -> doctores[i];
        }

        delete[] hospital -> doctores;
        hospital -> doctores = nuevosDoctores;
        hospital -> capacidadDoctores = nuevaCapacidad;

        cout << "Capacidad de doctores aumentada a " << nuevaCapacidad << endl;
    }

    int indice = hospital -> cantidadDoctores;

    hospital -> doctores[indice].id = hospital -> siguienteIdDoctor++;
    strcpy(hospital -> doctores[indice].nombre, nombre);
    strcpy(hospital -> doctores[indice].apellido, apellido);
    strcpy(hospital -> doctores[indice].cedula, cedula);
    strcpy(hospital -> doctores[indice].especialidad, especialidad);
    hospital -> doctores[indice].aniosExperiencia = aniosExperiencia;
    hospital -> doctores[indice].costoConsulta = costoConsulta;

    hospital -> doctores[indice].capacidadPacientes = 10;
    hospital -> doctores[indice].pacientesAsignados = new int[10];
    hospital -> doctores[indice].cantidadPacientes = 0;

    hospital -> doctores[indice].capacidadCitas = 20;
    hospital -> doctores[indice].citasAgendadas = new int[20];
    hospital -> doctores[indice].cantidadCitas = 0;

    strcpy(hospital -> doctores[indice].horarioAtencion, "L-V 8:00-17:00");
    strcpy(hospital -> doctores[indice].telefono, "");
    strcpy(hospital -> doctores[indice].email, "");

    hospital -> doctores[indice].disponible = true;

    hospital -> cantidadDoctores++;

    cout << "Doctor creado exitosamente. ID: " << hospital -> doctores[indice].id << endl;
    cout << "Especialidad: " << hospital -> doctores[indice].especialidad << endl;
    cout << "anios experiencia: " << hospital -> doctores[indice].aniosExperiencia << endl;
    cout << "Costo consulta: $" << hospital -> doctores[indice].costoConsulta << endl;

    return & hospital -> doctores[indice];
}
Doctor * buscarDoctorPorId(Hospital * hospital, int id) {
    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        if (hospital -> doctores[i].id == id) {
            return & hospital -> doctores[i];
        }
    }

    return nullptr;
}

Doctor ** buscardoctorporespecialidad(Hospital * hospital,
    const char * especialidad, int * cantidad) {
    Doctor ** resultados = new Doctor * [hospital -> cantidadDoctores];
    * cantidad = 0;

    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        if (strstr(hospital -> doctores[i].especialidad, especialidad) != nullptr) {
            resultados[ * cantidad] = & hospital -> doctores[i];
            ( * cantidad) ++;
        }
    }

    return resultados;
}
Doctor ** buscarDoctoresPorEspecialidad(Hospital * hospital,
    const char * especialidad,
        int * cantidad) {

    if (!hospital || !especialidad || !cantidad) {
        if (cantidad) * cantidad = 0;
        return nullptr;
    }

    * cantidad = 0;
    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        if (strcmp(hospital -> doctores[i].especialidad, especialidad) == 0) {
            ( * cantidad) ++;
        }
    }

    if ( * cantidad == 0) {
        cout << "No se encontraron doctores con especialidad: " << especialidad << endl;
        return nullptr;
    }

    Doctor ** resultados = new Doctor * [ * cantidad];

    int indice = 0;
    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        if (strcmp(hospital -> doctores[i].especialidad, especialidad) == 0) {
            resultados[indice] = & hospital -> doctores[i];
            indice++;
        }
    }

    cout << "Encontrados " << * cantidad << " doctores en " << especialidad << endl;
    return resultados;
}
void liberarResultadosDoctores(Doctor ** resultados) {
    if (resultados) {
        delete[] resultados;
    }
}
void mostrarDoctoresEspecialidad(Doctor ** doctores, int cantidad) {
    if (!doctores || cantidad == 0) {
        cout << "No hay doctores para mostrar." << endl;
        return;
    }

    cout << "\n=== DOCTORES ENCONTRADOS ===" << endl;
    cout << left << setw(4) << "ID" <<
        setw(20) << "NOMBRE COMPLETO" <<
        setw(15) << "ESPECIALIDAD" <<
        setw(5) << "aniosS" <<
        setw(10) << "COSTO" <<
        setw(10) << "ESTADO" <<
        endl;
    cout << string(70, '-') << endl;

    for (int i = 0; i < cantidad; i++) {
        Doctor * d = doctores[i];
        char nombreCompleto[100];
        snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", d -> nombre, d -> apellido);

        cout << left << setw(4) << d -> id <<
            setw(20) << nombreCompleto <<
            setw(15) << d -> especialidad <<
            setw(5) << d -> aniosExperiencia <<
            setw(10) << d -> costoConsulta <<
            setw(10) << (d -> disponible ? "Activo" : "Inactivo") <<
            endl;
    }
    cout << string(70, '-') << endl;
}
bool asignarpacienteadoctor(Hospital * hospital, int idDoctor, int idPaciente) {
    Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No se encontrÃ³ doctor con ID " << idDoctor << endl;
        return false;
    }

    Paciente * paciente = buscarPacientePorId(hospital, idPaciente);
    if (!paciente) {
        cout << "Error: No se encontrÃ³ paciente con ID " << idPaciente << endl;
        return false;
    }

    for (int i = 0; i < doctor -> cantidadPacientes; i++) {
        if (doctor -> pacientesAsignados[i] == idPaciente) {
            cout << "El paciente ya estÃ¡ asignado al doctor." << endl;
            return false;
        }
    }

    if (doctor -> cantidadPacientes >= doctor -> capacidadPacientes) {
        int nuevaCapacidad = doctor -> capacidadPacientes * 2;
        int * nuevosPacientes = new int[nuevaCapacidad];

        for (int i = 0; i < doctor -> cantidadPacientes; i++) {
            nuevosPacientes[i] = doctor -> pacientesAsignados[i];
        }

        delete[] doctor -> pacientesAsignados;
        doctor -> pacientesAsignados = nuevosPacientes;
        doctor -> capacidadPacientes = nuevaCapacidad;

        cout << "Capacidad de pacientes del doctor aumentada a " << nuevaCapacidad << endl;
    }

    doctor -> pacientesAsignados[doctor -> cantidadPacientes] = idPaciente;
    doctor -> cantidadPacientes++;

    cout << "Paciente ID " << idPaciente << " asignado al Doctor ID " << idDoctor << " correctamente." << endl;
    return true;
}
bool removerPacienteDeDoctor(Doctor * doctor, int idPaciente) {
    if (!doctor) {
        cout << "Error: Doctor no vÃ¡lido." << endl;
        return false;
    }

    if (doctor -> cantidadPacientes == 0) {
        cout << "El doctor no tiene pacientes asignados." << endl;
        return false;
    }

    int indice = -1;
    for (int i = 0; i < doctor -> cantidadPacientes; i++) {
        if (doctor -> pacientesAsignados[i] == idPaciente) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        cout << "El paciente con ID " << idPaciente << " no estÃ¡ asignado a este doctor." << endl;
        return false;
    }

    // Compactar array (mover elementos hacia adelante)
    for (int i = indice; i < doctor -> cantidadPacientes - 1; i++) {
        doctor -> pacientesAsignados[i] = doctor -> pacientesAsignados[i + 1];
    }

    doctor -> cantidadPacientes--;

    cout << "Paciente con ID " << idPaciente << " removido del doctor " <<
        doctor -> nombre << " " << doctor -> apellido << endl;
    cout << "Pacientes restantes asignados: " << doctor -> cantidadPacientes << endl;

    return true;
}
void listarpacientesdedoctor(Hospital * hospital, int idDoctor) {
    Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No se encontrÃ³ doctor con ID " << idDoctor << endl;
        return;
    }

    if (doctor -> cantidadPacientes == 0) {
        cout << "El doctor " << doctor -> nombre << " " << doctor -> apellido << " no tiene pacientes asignados." << endl;
        return;
    }

    cout << "\n=== PACIENTES ASIGNADOS AL DOCTOR " << doctor -> nombre << " " << doctor -> apellido << " ===" << endl;
    cout << left << setw(4) << "ID" <<
        setw(20) << "NOMBRE COMPLETO" <<
        setw(12) << "CÃ‰DULA" <<
        setw(5) << "EDAD" <<
        endl;
    cout << string(50, '-') << endl;

    for (int i = 0; i < doctor -> cantidadPacientes; i++) {
        int idPaciente = doctor -> pacientesAsignados[i];
        Paciente * paciente = buscarPacientePorId(hospital, idPaciente);
        if (paciente) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", paciente -> nombre, paciente -> apellido);

            cout << left << setw(4) << paciente -> id <<
                setw(20) << nombreCompleto <<
                setw(12) << paciente -> cedula <<
                setw(5) << paciente -> edad <<
                endl;
        }
    }
    cout << string(50, '-') << endl;
}
void listardoctores(Hospital * hospital) {
    if (hospital -> cantidadDoctores == 0) {
        cout << "No hay doctores registrados." << endl;
        return;
    }

    cout << "\nLISTA DE DOCTORES (" << hospital -> cantidadDoctores << "):" << endl;
    cout << "+-----------------------------------------------------------------------------+" << endl;
    cout << "Â¦  ID  Â¦       NOMBRE COMPLETO    Â¦  ESPECIALIDAD Â¦ aniosS EXPERIENCIA Â¦  COSTO    Â¦" << endl;
    cout << "+------+--------------------------+--------------+---------------+------------Â¦" << endl;

    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        Doctor * d = & hospital -> doctores[i];

        char nombreCompleto[100];
        snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", d -> nombre, d -> apellido);

        if (strlen(nombreCompleto) > 22) {
            nombreCompleto[19] = '.';
            nombreCompleto[20] = '.';
            nombreCompleto[21] = '.';
            nombreCompleto[22] = '\0';
        }

        printf("Â¦ %4d Â¦ %-24s Â¦ %-12s Â¦     %4d     Â¦ $%8.2f Â¦\n",
            d -> id, nombreCompleto, d -> especialidad, d -> aniosExperiencia, d -> costoConsulta);
    }

    cout << "+-----------------------------------------------------------------------------+" << endl;

}
bool eliminarDoctor(Hospital * hospital, int id) {
    // Buscar el Ã­ndice del doctor
    int indice = -1;
    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        if (hospital -> doctores[i].id == id) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        cout << "Error: No se encontrÃ³ doctor con ID " << id << endl;
        return false;
    }

    Doctor * doctor = & hospital -> doctores[indice];
    // Liberar memoria de los arreglos dinÃ¡micos del doctor
    delete[] doctor -> pacientesAsignados;
    delete[] doctor -> citasAgendadas;

    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idDoctor == id) {
            strcpy(hospital -> citas[i].estado, "CANCELADA");
            hospital -> citas[i].atendida = false;

            for (int j = 0; j < hospital -> cantidadPacientes; j++) {
                for (int k = 0; k < hospital -> pacientes[j].cantidadCitas; k++) {
                    if (hospital -> pacientes[j].citasAgendadas[k] == hospital -> citas[i].id) {
                        // Mover citas restantes hacia adelante
                        for (int l = k; l < hospital -> pacientes[j].cantidadCitas - 1; l++) {
                            hospital -> pacientes[j].citasAgendadas[l] = hospital -> pacientes[j].citasAgendadas[l + 1];
                        }
                        hospital -> pacientes[j].cantidadCitas--;
                        break;
                    }
                }
            }
        }
    }

    //  Compactar el arreglo de doctores 
    for (int i = indice; i < hospital -> cantidadDoctores - 1; i++) {
        hospital -> doctores[i] = hospital -> doctores[i + 1];
    }

    hospital -> cantidadDoctores--;

    cout << "Doctor con ID " << id << " eliminado correctamente." << endl;
    cout << "Citas asociadas canceladas y memoria liberada." << endl;

    return true;
}
Cita * agendarCita(Hospital * hospital, int idPaciente, int idDoctor,
    const char * fecha,
        const char * hora,
            const char * motivo) {

    // Validaciones bÃ¡sicas de parÃ¡metros
    if (!hospital || !fecha || !hora || !motivo) {
        cout << "Error: ParÃ¡metros invÃ¡lidos." << endl;
        return nullptr;
    }

    // Verificar que el paciente exista
    Paciente * paciente = buscarPacientePorId(hospital, idPaciente);
    if (!paciente) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        return nullptr;
    }

    //  Verificar que el doctor exista
    Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return nullptr;
    }

    // Verificar que el doctor estÃ© disponible
    if (!doctor -> disponible) {
        cout << "Error: El doctor no estÃ¡ disponible." << endl;
        return nullptr;
    }

    // Validar formato de fecha (YYYY-MM-DD)
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha invÃ¡lido. Use YYYY-MM-DD" << endl;
        return nullptr;
    }

    // Validar que los componentes de la fecha sean nÃºmeros
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(fecha[i])) {
            cout << "Error: La fecha debe contener solo nÃºmeros y guiones." << endl;
            return nullptr;
        }
    }

    //  Validar formato de hora (HH:MM)
    if (strlen(hora) != 5 || hora[2] != ':') {
        cout << "Error: Formato de hora invÃ¡lido. Use HH:MM" << endl;
        return nullptr;
    }

    // Validar que los componentes de la hora sean nÃºmeros
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) {
            cout << "Error: La hora debe contener solo nÃºmeros y dos puntos." << endl;
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
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idDoctor == idDoctor &&
            strcmp(hospital -> citas[i].fecha, fecha) == 0 &&
            strcmp(hospital -> citas[i].hora, hora) == 0 &&
            strcmp(hospital -> citas[i].estado, "AGENDADA") == 0) {
            cout << "Error: El doctor ya tiene una cita agendada para " << fecha << " a las " << hora << endl;
            return nullptr;
        }
    }

    // Verificar que el paciente no tenga otra cita a la misma hora
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idPaciente == idPaciente &&
            strcmp(hospital -> citas[i].fecha, fecha) == 0 &&
            strcmp(hospital -> citas[i].hora, hora) == 0 &&
            strcmp(hospital -> citas[i].estado, "AGENDADA") == 0) {
            cout << "Error: El paciente ya tiene una cita agendada para " << fecha << " a las " << hora << endl;
            return nullptr;
        }
    }

    // Redimensionar arreglo de citas si estÃ¡ lleno
    if (hospital -> cantidadCitas >= hospital -> capacidadCitas) {
        int nuevaCapacidad = hospital -> capacidadCitas * 2;
        Cita * nuevasCitas = new Cita[nuevaCapacidad];

        for (int i = 0; i < hospital -> cantidadCitas; i++) {
            nuevasCitas[i] = hospital -> citas[i];
        }
        delete[] hospital -> citas;
        hospital -> citas = nuevasCitas;
        hospital -> capacidadCitas = nuevaCapacidad;

        cout << "Capacidad de citas aumentada a " << nuevaCapacidad << endl;
    }

    // Obtener Ã­ndice de la nueva cita
    int indice = hospital -> cantidadCitas;

    // Crear estructura Cita
    hospital -> citas[indice].id = hospital -> siguienteIdCita++;
    hospital -> citas[indice].idPaciente = idPaciente;
    hospital -> citas[indice].idDoctor = idDoctor;
    strcpy(hospital -> citas[indice].fecha, fecha);
    strcpy(hospital -> citas[indice].hora, hora);
    strcpy(hospital -> citas[indice].motivo, motivo);
    strcpy(hospital -> citas[indice].estado, "AGENDADA");
    strcpy(hospital -> citas[indice].observaciones, "");
    hospital -> citas[indice].atendida = false;

    // Agregar ID de cita al array del paciente
    if (paciente -> cantidadCitas >= paciente -> capacidadCitas) {
        int nuevaCapacidad = paciente -> capacidadCitas * 2;
        int * nuevasCitasPaciente = new int[nuevaCapacidad];

        for (int i = 0; i < paciente -> cantidadCitas; i++) {
            nuevasCitasPaciente[i] = paciente -> citasAgendadas[i];
        }

        delete[] paciente -> citasAgendadas;
        paciente -> citasAgendadas = nuevasCitasPaciente;
        paciente -> capacidadCitas = nuevaCapacidad;
    }
    paciente -> citasAgendadas[paciente -> cantidadCitas++] = hospital -> citas[indice].id;

    // Agregar ID de cita al arreglo del doctor
    if (doctor -> cantidadCitas >= doctor -> capacidadCitas) {
        int nuevaCapacidad = doctor -> capacidadCitas * 2;
        int * nuevasCitasDoctor = new int[nuevaCapacidad];

        for (int i = 0; i < doctor -> cantidadCitas; i++) {
            nuevasCitasDoctor[i] = doctor -> citasAgendadas[i];
        }

        delete[] doctor -> citasAgendadas;
        doctor -> citasAgendadas = nuevasCitasDoctor;
        doctor -> capacidadCitas = nuevaCapacidad;
    }
    doctor -> citasAgendadas[doctor -> cantidadCitas++] = hospital -> citas[indice].id;

    hospital -> cantidadCitas++;

    cout << "Cita agendada exitosamente. ID: " << hospital -> citas[indice].id << endl;
    cout << "Paciente: " << paciente -> nombre << " " << paciente -> apellido << endl;
    cout << "Doctor: " << doctor -> nombre << " " << doctor -> apellido << endl;
    cout << "Fecha: " << fecha << " " << hora << endl;
    cout << "Motivo: " << motivo << endl;

    return & hospital -> citas[indice];
}
bool cancelarCita(Hospital * hospital, int idCita) {
    // Buscar la cita por ID
    Cita * cita = nullptr;
    int indiceCita = -1;

    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].id == idCita) {
            cita = & hospital -> citas[i];
            indiceCita = i;
            break;
        }
    }

    if (!cita) {
        cout << "Error: No se encontrÃ³ cita con ID " << idCita << endl;
        return false;
    }

    // Verificar que la cita no estÃ© ya cancelada o atendida
    if (strcmp(cita -> estado, "CANCELADA") == 0) {
        cout << "La cita ya estÃ¡ cancelada." << endl;
        return false;
    }

    if (cita -> atendida) {
        cout << "Error: No se puede cancelar una cita ya atendida." << endl;
        return false;
    }

    // Cambiar estado a "Cancelada"
    strcpy(cita -> estado, "CANCELADA");
    cita -> atendida = false;

    // Remover del array de citas del paciente
    Paciente * paciente = buscarPacientePorId(hospital, cita -> idPaciente);
    if (paciente) {
        for (int i = 0; i < paciente -> cantidadCitas; i++) {
            if (paciente -> citasAgendadas[i] == idCita) {
                // Compactar array moviendo elementos hacia adelante
                for (int j = i; j < paciente -> cantidadCitas - 1; j++) {
                    paciente -> citasAgendadas[j] = paciente -> citasAgendadas[j + 1];
                }
                paciente -> cantidadCitas--;
                break;
            }
        }
    }

    // Remover del array de citas del doctor
    Doctor * doctor = buscarDoctorPorId(hospital, cita -> idDoctor);
    if (doctor) {
        for (int i = 0; i < doctor -> cantidadCitas; i++) {
            if (doctor -> citasAgendadas[i] == idCita) {
                // Compactar array moviendo elementos hacia adelante
                for (int j = i; j < doctor -> cantidadCitas - 1; j++) {
                    doctor -> citasAgendadas[j] = doctor -> citasAgendadas[j + 1];
                }
                doctor -> cantidadCitas--;
                break;
            }
        }
    }

    cout << "Cita cancelada exitosamente." << endl;
    cout << "ID Cita: " << cita -> id << endl;
    cout << "Paciente: " << (paciente ? paciente -> nombre : "No encontrado") << endl;
    cout << "Doctor: " << (doctor ? doctor -> nombre : "No encontrado") << endl;
    cout << "Fecha: " << cita -> fecha << " " << cita -> hora << endl;

    return true;
}
bool atenderCita(Hospital * hospital, int idCita,
    const char * diagnostico,
        const char * tratamiento,
            const char * medicamentos) {

    if (!hospital || !diagnostico || !tratamiento || !medicamentos) {
        cout << "Error: ParÃ¡metros invÃ¡lidos." << endl;
        return false;
    }

    // Buscar la cita por ID
    Cita * cita = nullptr;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].id == idCita) {
            cita = & hospital -> citas[i];
            break;
        }
    }

    if (!cita) {
        cout << "Error: No se encontrÃ³ cita con ID " << idCita << endl;
        return false;
    }

    //Verificar que estÃ© en estado "Agendada"
    if (strcmp(cita -> estado, "AGENDADA") != 0) {
        cout << "Error: La cita no estÃ¡ en estado 'Agendada'. Estado actual: " << cita -> estado << endl;
        return false;
    }

    if (cita -> atendida) {
        cout << "Error: La cita ya fue atendida." << endl;
        return false;
    }

    // Obtener paciente y doctor asociados
    Paciente * paciente = buscarPacientePorId(hospital, cita -> idPaciente);
    Doctor * doctor = buscarDoctorPorId(hospital, cita -> idDoctor);

    if (!paciente) {
        cout << "Error: No se encontrÃ³ el paciente asociado a la cita." << endl;
        return false;
    }

    if (!doctor) {
        cout << "Error: No se encontrÃ³ el doctor asociado a la cita." << endl;
        return false;
    }

    // Cambiar estado a "Atendida" y atendida = true
    strcpy(cita -> estado, "ATENDIDA");
    cita -> atendida = true;

    HistorialMedico nuevaConsulta;

    nuevaConsulta.idConsulta = hospital -> siguienteIdConsulta++;

    strcpy(nuevaConsulta.fecha, cita -> fecha);
    strcpy(nuevaConsulta.hora, cita -> hora);

    // DiagnÃ³stico, tratamiento, medicamentos recibidos
    strcpy(nuevaConsulta.diagnostico, diagnostico);
    strcpy(nuevaConsulta.tratamiento, tratamiento);
    strcpy(nuevaConsulta.medicamentos, medicamentos);

    nuevaConsulta.idDoctor = cita -> idDoctor;

    nuevaConsulta.costo = doctor -> costoConsulta;

    agregarConsultaAlHistorial(paciente, nuevaConsulta);

    // El contador de consultas se incrementa automÃ¡ticamente en agregarConsultaAlHistorial

    // Actualizar observaciones de la cita
    char observaciones[200];
    snprintf(observaciones, sizeof(observaciones),
        "Atendida - DiagnÃ³stico: %s", diagnostico);
    strcpy(cita -> observaciones, observaciones);

    cout << "Cita atendida exitosamente." << endl;
    cout << "ID Cita: " << cita -> id << endl;
    cout << "Paciente: " << paciente -> nombre << " " << paciente -> apellido << endl;
    cout << "Doctor: " << doctor -> nombre << " " << doctor -> apellido << endl;
    cout << "DiagnÃ³stico: " << diagnostico << endl;
    cout << "Costo: $" << doctor -> costoConsulta << endl;
    cout << "Consulta agregada al historial mÃ©dico. ID Consulta: " << nuevaConsulta.idConsulta << endl;

    return true;
}

Cita ** obtenerCitasDePaciente(Hospital * hospital, int idPaciente, int * cantidad) {
    // Validar parÃ¡metros
    if (!hospital || !cantidad) {
        if (cantidad) * cantidad = 0;
        return nullptr;
    }

    // Verificar que el paciente exista
    Paciente * paciente = buscarPacientePorId(hospital, idPaciente);
    if (!paciente) {
        cout << "Error: No existe paciente con ID " << idPaciente << endl;
        * cantidad = 0;
        return nullptr;
    }

    // Contar cuÃ¡ntas citas tiene el paciente
    * cantidad = 0;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idPaciente == idPaciente) {
            ( * cantidad) ++;
        }
    }

    // Si no tiene citas, retornar nullptr
    if ( * cantidad == 0) {
        cout << "El paciente no tiene citas registradas." << endl;
        return nullptr;
    }

    // Crear arreglo dinÃ¡mico de punteros a Cita
    Cita ** resultados = new Cita * [ * cantidad];

    // Llenar el arreglo con punteros a las citas del paciente
    int indice = 0;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idPaciente == idPaciente) {
            resultados[indice] = & hospital -> citas[i];
            indice++;
        }
    }

    cout << "Encontradas " << * cantidad << " citas para el paciente ID " << idPaciente << endl;
    return resultados;
}
void liberarResultadosCitas(Cita ** resultados) {
    if (resultados) {
        delete[] resultados;
    }
}
Cita ** obtenerCitasDeDoctor(Hospital * hospital, int idDoctor, int * cantidad) {
    // Validar parÃ¡metros
    if (!hospital || !cantidad) {
        if (cantidad) * cantidad = 0;
        return nullptr;
    }

    // Verificar que el doctor exista
    Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        * cantidad = 0;
        return nullptr;
    }

    // Contar cuÃ¡ntas citas tiene el doctor
    * cantidad = 0;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idDoctor == idDoctor) {
            ( * cantidad) ++;
        }
    }

    // Si no tiene citas, retornar nullptr
    if ( * cantidad == 0) {
        cout << "El doctor no tiene citas registradas." << endl;
        return nullptr;
    }

    // Crear arreglo dinÃ¡mico de punteros a Cita
    Cita ** resultados = new Cita * [ * cantidad];

    // Llenar el arreglo con punteros a las citas del doctor
    int indice = 0;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idDoctor == idDoctor) {
            resultados[indice] = & hospital -> citas[i];
            indice++;
        }
    }

    cout << "Encontradas " << * cantidad << " citas para el doctor ID " << idDoctor << endl;
    return resultados;
}
Cita ** obtenerCitasPorFecha(Hospital * hospital,
    const char * fecha, int * cantidad) {
    // Validar parÃ¡metros
    if (!hospital || !fecha || !cantidad) {
        if (cantidad) * cantidad = 0;
        return nullptr;
    }

    // Validar formato de fecha
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha invÃ¡lido. Use YYYY-MM-DD" << endl;
        * cantidad = 0;
        return nullptr;
    }

    // Contar cuÃ¡ntas citas hay para esa fecha
    * cantidad = 0;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (strcmp(hospital -> citas[i].fecha, fecha) == 0) {
            ( * cantidad) ++;
        }
    }

    // Si no hay citas para esa fecha, retornar nullptr
    if ( * cantidad == 0) {
        cout << "No hay citas registradas para la fecha " << fecha << endl;
        return nullptr;
    }

    // Crear array dinÃ¡mico de punteros a Cita
    Cita ** resultados = new Cita * [ * cantidad];

    // Llenar el array con punteros a las citas de la fecha
    int indice = 0;
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (strcmp(hospital -> citas[i].fecha, fecha) == 0) {
            resultados[indice] = & hospital -> citas[i];
            indice++;
        }
    }

    cout << "Encontradas " << * cantidad << " citas para la fecha " << fecha << endl;
    return resultados;
}

void listarCitasPendientes(Hospital * hospital) {
    if (!hospital || hospital -> cantidadCitas == 0) {
        cout << "No hay citas en el sistema." << endl;
        return;
    }

    int cantidadPendientes = 0;

    // Primero contar
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (strcmp(hospital -> citas[i].estado, "AGENDADA") == 0) {
            cantidadPendientes++;
        }
    }

    if (cantidadPendientes == 0) {
        cout << "No hay citas pendientes." << endl;
        return;
    }

    cout << "\n------------------------------------------------------------------------------" << endl;
    cout << "                         CITAS PENDIENTES - TOTAL: " << cantidadPendientes << endl;
    cout << "------------------------------------------------------------------------------" << endl;

    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (strcmp(hospital -> citas[i].estado, "AGENDADA") == 0) {
            Cita * c = & hospital -> citas[i];
            Paciente * p = buscarPacientePorId(hospital, c -> idPaciente);
            Doctor * d = buscarDoctorPorId(hospital, c -> idDoctor);

            cout << "+-- CITA #" << c -> id << " -----------------------------------------------------------+" << endl;
            cout << "Â¦ Fecha: " << setw(12) << left << c -> fecha << " Hora: " << setw(8) << c -> hora;
            cout << " Especialidad: " << setw(15) << (d ? d -> especialidad : "N/A") << " Â¦" << endl;
            cout << "Â¦ Paciente: " << setw(25) << (p ? string(p -> nombre) + " " + p -> apellido : "No encontrado");
            cout << " CÃ©dula: " << setw(12) << (p ? p -> cedula : "N/A") << " Â¦" << endl;
            cout << "Â¦ Doctor: " << setw(27) << (d ? "Dr. " + string(d -> nombre) + " " + d -> apellido : "No encontrado");
            cout << " Costo: $" << setw(8) << fixed << setprecision(2) << (d ? d -> costoConsulta : 0) << " Â¦" << endl;
            cout << "Â¦ Motivo: " << setw(58) << c -> motivo << " Â¦" << endl;
            cout << "+------------------------------------------------------------------------------------+" << endl;
        }
    }

    cout << "------------------------------------------------------------------------------" << endl;
}

bool verificarDisponibilidad(Hospital * hospital, int idDoctor,
    const char * fecha,
        const char * hora) {
    // Validar parÃ¡metros
    if (!hospital || !fecha || !hora) {
        cout << "Error: ParÃ¡metros invÃ¡lidos." << endl;
        return false;
    }

    // Verificar que el doctor exista
    Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
    if (!doctor) {
        cout << "Error: No existe doctor con ID " << idDoctor << endl;
        return false;
    }

    // Verificar que el doctor estÃ© disponible
    if (!doctor -> disponible) {
        cout << "El doctor no estÃ¡ disponible para consultas." << endl;
        return false;
    }

    // Validar formato de fecha
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        cout << "Error: Formato de fecha invÃ¡lido." << endl;
        return false;
    }

    // Validar formato de hora
    if (strlen(hora) != 5 || hora[2] != ':') {
        cout << "Error: Formato de hora invÃ¡lido." << endl;
        return false;
    }

    // Verificar si el doctor ya tiene una cita a esa fecha/hora
    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        if (hospital -> citas[i].idDoctor == idDoctor &&
            strcmp(hospital -> citas[i].fecha, fecha) == 0 &&
            strcmp(hospital -> citas[i].hora, hora) == 0 &&
            strcmp(hospital -> citas[i].estado, "AGENDADA") == 0) {

            // Encontrar informaciÃ³n del paciente para el mensaje
            Paciente * paciente = buscarPacientePorId(hospital, hospital -> citas[i].idPaciente);
            cout << "El doctor ya tiene una cita agendada para " << fecha << " a las " << hora << endl;
            if (paciente) {
                cout << "Cita con: " << paciente -> nombre << " " << paciente -> apellido << endl;
            }
            return false;
        }
    }

    // Si llegamos aquÃ­, el doctor estÃ¡ disponible
    cout << "Doctor disponible el " << fecha << " a las " << hora << endl;
    cout << "Doctor: " << doctor -> nombre << " " << doctor -> apellido << endl;
    cout << "Especialidad: " << doctor -> especialidad << endl;

    return true;
}
Hospital * inicializarHospital(const char * nombre = "Hospital Central",
    int capacidadPacientes = 10,
    int capacidadDoctores = 5,
    int capacidadCitas = 20) {

    if (!nombre || capacidadPacientes <= 0 || capacidadDoctores <= 0 || capacidadCitas <= 0) {
        cout << "Error: ParÃ¡metros invÃ¡lidos para inicializar hospital." << endl;
        return nullptr;
    }

    Hospital * hospital = new Hospital;

    strcpy(hospital -> nombre, nombre);
    strcpy(hospital -> direccion, "Por definir");
    strcpy(hospital -> telefono, "Por definir");

    hospital -> capacidadPacientes = capacidadPacientes;
    hospital -> pacientes = new Paciente[capacidadPacientes];
    hospital -> cantidadPacientes = 0;

    hospital -> capacidadDoctores = capacidadDoctores;
    hospital -> doctores = new Doctor[capacidadDoctores];
    hospital -> cantidadDoctores = 0;

    hospital -> capacidadCitas = capacidadCitas;
    hospital -> citas = new Cita[capacidadCitas];
    hospital -> cantidadCitas = 0;

    hospital -> siguienteIdPaciente = 1;
    hospital -> siguienteIdDoctor = 1;
    hospital -> siguienteIdCita = 1;
    hospital -> siguienteIdConsulta = 1;

    cout << "    Hospital '" << nombre << "' inicializado correctamente" << endl;
    cout << "   Capacidades configuradas:" << endl;
    cout << "    Pacientes: " << capacidadPacientes << endl;
    cout << "    Doctores: " << capacidadDoctores << endl;
    cout << "    Citas: " << capacidadCitas << endl;
    cout << endl;

    return hospital;
}

void destruirHospital(Hospital * hospital) {
    if (!hospital) return;

    cout << "Destruyendo hospital: " << hospital -> nombre << endl;

    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        delete[] hospital -> pacientes[i].historial;
        delete[] hospital -> pacientes[i].citasAgendadas;
    }
    delete[] hospital -> pacientes;

    for (int i = 0; i < hospital -> cantidadDoctores; i++) {
        delete[] hospital -> doctores[i].pacientesAsignados;
        delete[] hospital -> doctores[i].citasAgendadas;
    }
    delete[] hospital -> doctores;

    delete[] hospital -> citas;

    delete hospital;

    cout << "Memoria liberada completamente." << endl;
}

int redimensionarArrayPacientes(Hospital * hospital) {
    if (!hospital) return 0;

    int capacidadAnterior = hospital -> capacidadPacientes;
    int nuevaCapacidad = capacidadAnterior * 2;

    Paciente * nuevosPacientes = new Paciente[nuevaCapacidad];

    for (int i = 0; i < hospital -> cantidadPacientes; i++) {
        nuevosPacientes[i] = hospital -> pacientes[i];
    }

    delete[] hospital -> pacientes;
    hospital -> pacientes = nuevosPacientes;
    hospital -> capacidadPacientes = nuevaCapacidad;

    cout << "Arreglo pacientes redimensionado: " << capacidadAnterior << " ? " << nuevaCapacidad << endl;

    return nuevaCapacidad;
}

int redimensionarArrayCitas(Hospital * hospital) {
    if (!hospital) return 0;

    int capacidadAnterior = hospital -> capacidadCitas;
    int nuevaCapacidad = capacidadAnterior * 2;

    Cita * nuevasCitas = new Cita[nuevaCapacidad];

    for (int i = 0; i < hospital -> cantidadCitas; i++) {
        nuevasCitas[i] = hospital -> citas[i];
    }

    delete[] hospital -> citas;
    hospital -> citas = nuevasCitas;
    hospital -> capacidadCitas = nuevaCapacidad;

    cout << "Array citas redimensionado: " << capacidadAnterior << " ? " << nuevaCapacidad << endl;

    return nuevaCapacidad;
}

//modulo de validaciones

bool validarFecha(const char * fecha) {
    // Verificar formato YYYY-MM-DD (longitud 10)
    if (!fecha || strlen(fecha) != 10) {
        cout << "Error: La fecha debe tener formato YYYY-MM-DD (10 caracteres)" << endl;
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
        cout << "Error: anios " << anios << " fuera de rango. Use entre 2020-2030" << endl;
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
            diasEnMes = 29; // anios bisiesto
        } else {
            diasEnMes = 28; // anios no bisiesto
        }
        break;
    default:
        diasEnMes = 0; // Nunca deberÃ­a llegar aquÃ­
    }

    if (dia < 1 || dia > diasEnMes) {
        cout << "Error: DÃ­a " << dia << " invÃ¡lido para " <<
            obtenerNombreMes(mes) << " " << anios <<
            " (mÃ¡ximo " << diasEnMes << " dÃ­as)" << endl;
        return false;
    }

    return true;
}

const char * obtenerNombreMes(int mes) {
    static
    const char * nombresMeses[] = {
        "Enero",
        "Febrero",
        "Marzo",
        "Abril",
        "Mayo",
        "Junio",
        "Julio",
        "Agosto",
        "Septiembre",
        "Octubre",
        "Noviembre",
        "Diciembre"
    };

    if (mes >= 1 && mes <= 12) {
        return nombresMeses[mes - 1];
    }
    return "Mes invÃ¡lido";
}
bool validarHora(const char * hora) {

    if (strlen(hora) != 5) {
        return false;
    }

    // Verificar formato HH:MM
    // Posiciones: 0,1 = horas; 2 = ':' ; 3,4 = minutos
    if (hora[2] != ':') {
        return false;
    }

    // Verificar que todos los caracteres numÃ©ricos sean dÃ­gitos
    if (!isdigit(hora[0]) || !isdigit(hora[1]) || !isdigit(hora[3]) || !isdigit(hora[4])) {
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
int compararFechas(const char * fecha1,
    const char * fecha2) {
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

bool validarEmail(const char * email) {
    if (email == nullptr || strlen(email) < 5) { // mÃ­nimo: a@b.c
        return false;
    }

    const char * arroba = strchr(email, '@');
    if (arroba == nullptr || arroba == email) {
        return false; // No hay @ o estÃ¡ al inicio
    }

    // Buscar punto despuÃ©s del @
    const char * punto = strchr(arroba + 1, '.');
    if (punto == nullptr || punto == arroba + 1 || * (punto + 1) == '\0') {
        return false; // No hay punto, o estÃ¡ justo despuÃ©s del @, o no hay nada despuÃ©s
    }

    return (strchr(email, ' ') == nullptr);
}

char * copiarString(const char * origen) {
    // Si origen es nullptr, retornar nullptr
    if (origen == nullptr) {
        return nullptr;
    }

    // Calcular longitud con strlen()
    int longitud = strlen(origen);

    // Crear nuevo arreglo con new char[longitud + 1]
    char * destino = new char[longitud + 1];

    strcpy(destino, origen);

    return destino;
}

Paciente * copiarPaciente(Paciente * original) {
    if (original == nullptr) {
        return nullptr;
    }

    // Crear nuevo paciente en el heap
    Paciente * copia = new Paciente;

    // Copiar datos primitivos y arreglos estÃ¡ticos
    copia -> id = original -> id;
    copia -> edad = original -> edad;
    copia -> sexo = original -> sexo;
    copia -> cantidadConsultas = original -> cantidadConsultas;
    copia -> capacidadHistorial = original -> capacidadHistorial;
    copia -> cantidadCitas = original -> cantidadCitas;
    copia -> capacidadCitas = original -> capacidadCitas;
    copia -> activo = original -> activo;

    // Copiar arreglos estÃ¡ticos
    strcpy(copia -> nombre, original -> nombre);
    strcpy(copia -> apellido, original -> apellido);
    strcpy(copia -> cedula, original -> cedula);
    strcpy(copia -> tipoSangre, original -> tipoSangre);
    strcpy(copia -> telefono, original -> telefono);
    strcpy(copia -> direccion, original -> direccion);
    strcpy(copia -> email, original -> email);
    strcpy(copia -> alergias, original -> alergias);
    strcpy(copia -> observaciones, original -> observaciones);

    // DEEP COPY: Copiar arreglo dinÃ¡mico de historial mÃ©dico
    if (original -> cantidadConsultas > 0 && original -> historial != nullptr) {
        copia -> historial = new HistorialMedico[original -> capacidadHistorial];

        for (int i = 0; i < original -> cantidadConsultas; i++) {

            copia -> historial[i] = original -> historial[i];
            // esta parte hay que mejorarla 
        }
    } else {
        copia -> historial = nullptr;
    }

    // DEEP COPY: Copiar arreglo dinÃ¡mico de IDs de citas
    if (original -> cantidadCitas > 0 && original -> citasAgendadas != nullptr) {
        copia -> citasAgendadas = new int[original -> capacidadCitas];

        for (int i = 0; i < original -> cantidadCitas; i++) {
            copia -> citasAgendadas[i] = original -> citasAgendadas[i];
        }
    } else {
        copia -> citasAgendadas = nullptr;
    }

    return copia;
}

// FUNCIÃ“N PRINCIPAL
void mostrarMenuPrincipal() {
    ;
    cout << "\n-----------------------------------------------" << endl;
    cout << "            HOSPITAL EL CALLAO" << endl;
    cout << "              MENÃš PRINCIPAL" << endl;
    cout << "-----------------------------------------------" << endl;
    cout << "1.  GestiÃ³n de Pacientes" << endl;
    cout << "2.  GestiÃ³n de Doctores" << endl;
    cout << "3.  GestiÃ³n de Citas" << endl;
    cout << "0.  Salir" << endl;
    cout << "-----------------------------------------------" << endl;
}

// FUNCIÃ“N PARA LEER OPCIONES CON VALIDACIÃ“N
int leerOpcion(const char * mensaje, int min, int max) {
    int opcion;
    while (true) {
        cout << mensaje << " [" << min << "-" << max << "]: ";
        cin >> opcion;

        if (cin.fail()) {
            cin.clear();
            limpiarBuffer();
            cout << "Error: Por favor ingrese un nÃºmero vÃ¡lido." << endl;
            continue;
        }

        limpiarBuffer();

        if (opcion >= min && opcion <= max) {
            return opcion;
        } else {
            cout << "Error: OpciÃ³n fuera de rango. Use [" << min << "-" << max << "]" << endl;
        }
    }
}

void limpiarBuffer() {
    cin.ignore(numeric_limits < streamsize > ::max(), '\n');
}

void pausarPantalla() {
    cout << "\nPresione Enter para continuar...";
    cin.get();
}

void menuGestionPacientes(Hospital * hospital) {
    int opcion;

    do {
        system("cls");
        system("cls");
        cout << "\n-----------------------------------------------" << endl;
        cout << "           GESTIÃ“N DE PACIENTES" << endl;
        cout << "-----------------------------------------------" << endl;
        cout << "1.  Registrar nuevo paciente" << endl;
        cout << "2.  Buscar paciente por cÃ©dula" << endl;
        cout << "3.  Buscar paciente por nombre" << endl;
        cout << "4.  Ver historial mÃ©dico completo" << endl;
        cout << "5.  Actualizar datos del paciente" << endl;
        cout << "6.  Listar todos los pacientes" << endl;
        cout << "7.  Eliminar paciente" << endl;
        cout << "0.  Volver al menÃº principal" << endl;
        cout << "-----------------------------------------------" << endl;

        opcion = leerOpcion("Seleccione una opciÃ³n", 0, 7);

        switch (opcion) {
        case 1: {
            cout << "\n--- REGISTRAR NUEVO PACIENTE ---" << endl;
            char nombre[50], apellido[50], cedula[20];
            int edad;
            char sexo;

            cout << "Nombre: ";
            cin.getline(nombre, sizeof(nombre));
            cout << "Apellido: ";
            cin.getline(apellido, sizeof(apellido));
            cout << "CÃ©dula: ";
            cin.getline(cedula, sizeof(cedula));
            cout << "Edad: ";
            cin >> edad;
            limpiarBuffer();
            cout << "Sexo (M/F): ";
            cin >> sexo;
            limpiarBuffer();

            Paciente * paciente = crearPaciente(hospital, nombre, apellido, cedula, edad, sexo);
            if (paciente) {
                cout << " Paciente registrado exitosamente." << endl;
            }
            break;
        }

        case 2: {
            cout << "\n--- BUSCAR PACIENTE POR CÃ‰DULA ---" << endl;
            char cedula[20];
            cout << "Ingrese cÃ©dula: ";
            cin.getline(cedula, sizeof(cedula));

            Paciente * paciente = buscarPacientePorCedula(hospital, cedula);
            if (paciente) {
                cout << "Paciente encontrado:" << endl;
                cout << "ID: " << paciente -> id << endl;
                cout << "Nombre: " << paciente -> nombre << " " << paciente -> apellido << endl;
                cout << "Edad: " << paciente -> edad << endl;
                cout << "CÃ©dula: " << paciente -> cedula << endl;
            } else {
                cout << " No se encontrÃ³ paciente con esa cÃ©dula." << endl;
            }
            break;
        }

        case 3: {
            cout << "\n--- BUSCAR PACIENTE POR NOMBRE ---" << endl;
            char nombre[50];
            cout << "Ingrese nombre a buscar: ";
            cin.getline(nombre, sizeof(nombre));

            int cantidad;
            Paciente ** resultados = buscarPacientesPorNombre(hospital, nombre, & cantidad);
            if (resultados) {
                cout << "?? Encontrados " << cantidad << " pacientes:" << endl;
                for (int i = 0; i < cantidad; i++) {
                    cout << (i + 1) << ". " << resultados[i] -> nombre << " " <<
                        resultados[i] -> apellido << " (CÃ©dula: " <<
                        resultados[i] -> cedula << ")" << endl;
                }
                liberarResultadosBusqueda(resultados);
            } else {
                cout << " No se encontraron pacientes con ese nombre." << endl;
            }
            break;
        }

        case 4: {
            cout << "\n--- VER HISTORIAL MÃ‰DICO ---" << endl;
            int idPaciente;
            cout << "Ingrese ID del paciente: ";
            cin >> idPaciente;
            limpiarBuffer();

            Paciente * paciente = buscarPacientePorId(hospital, idPaciente);
            if (paciente) {
                mostrarHistorialMedico(paciente);
            } else {
                cout << " No se encontrÃ³ paciente con ese ID." << endl;
            }
            break;
        }

        case 5: {
            cout << "\n--- ACTUALIZAR DATOS DE PACIENTE ---" << endl;
            int idPaciente;
            cout << "Ingrese ID del paciente a actualizar: ";
            cin >> idPaciente;
            limpiarBuffer();

            bool resultado = actualizarPaciente(hospital, idPaciente);
            if (resultado) {
                cout << " Paciente actualizado exitosamente." << endl;
            } else {
                cout << "  No se pudo actualizar el paciente." << endl;
            }
            break;
        }

        case 6: {
            cout << "\n--- LISTA DE TODOS LOS PACIENTES ---" << endl;
            listarPacientes(hospital);
            break;
        }

        case 7: {
            cout << "\n--- ELIMINAR PACIENTE ---" << endl;
            int idPaciente;
            cout << "Ingrese ID del paciente a eliminar: ";
            cin >> idPaciente;
            limpiarBuffer();

            // Buscar y mostrar informaciÃ³n del paciente
            Paciente * paciente = buscarPacientePorId(hospital, idPaciente);
            if (!paciente) {
                cout << " No se encontrÃ³ paciente con ID " << idPaciente << endl;
                break;
            }

            cout << "\nPaciente a eliminar: " << paciente -> nombre << " " << paciente -> apellido << endl;

            char confirmacion;
            cout << "Â¿EstÃ¡ seguro de eliminar este paciente? (s/n): ";
            cin >> confirmacion;
            limpiarBuffer();

            if (tolower(confirmacion) == 's') {

                cout << "LOS DATOS SE PERDERÃN PERMANENTEMENTE. Â¿Continuar? (s/n): ";
                cin >> confirmacion;
                limpiarBuffer();

                if (tolower(confirmacion) == 's') {
                    bool resultado = eliminarPaciente(hospital, idPaciente);
                    if (resultado) {
                        cout << " Paciente eliminado exitosamente." << endl;
                    } else {
                        cout << " No se pudo eliminar el paciente." << endl;
                    }
                } else {
                    cout << " EliminaciÃ³n cancelada." << endl;
                }
            } else {
                cout << " EliminaciÃ³n cancelada." << endl;
            }
            break;
        }

        case 0:
            cout << "Volviendo al menÃº principal..." << endl;
            break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}

// MENÃš DE GESTIÃ“N DE DOCTORES
void menuGestionDoctores(Hospital * hospital) {
    int opcion;

    do {
        cout << "\n-----------------------------------------------" << endl;
        cout << "           GESTIÃ“N DE DOCTORES" << endl;
        cout << "-----------------------------------------------" << endl;
        cout << "1.  Registrar nuevo doctor" << endl;
        cout << "2.  Buscar doctor por ID" << endl;
        cout << "3.  Buscar doctores por especialidad" << endl;
        cout << "4.  Asignar paciente a doctor" << endl;
        cout << "5.  Ver pacientes asignados a doctor" << endl;
        cout << "6.  Listar todos los doctores" << endl;
        cout << "7.  Eliminar doctor" << endl;
        cout << "0.  Volver al menÃº principal" << endl;
        cout << "-----------------------------------------------" << endl;

        opcion = leerOpcion("Seleccione una opciÃ³n", 0, 7);

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
            cout << "CÃ©dula: ";
            cin.getline(cedula, sizeof(cedula));
            cout << "Especialidad: ";
            cin.getline(especialidad, sizeof(especialidad));
            cout << "anios de experiencia: ";
            cin >> aniosExperiencia;
            cout << "Costo de consulta: ";
            cin >> costoConsulta;
            limpiarBuffer();

            Doctor * doctor = crearDoctor(hospital, nombre, apellido, cedula,
                especialidad, aniosExperiencia, costoConsulta);
            if (doctor) {
                cout << " Doctor registrado exitosamente." << endl;
            }
            break;
        }

        case 2: {
            cout << "\n--- BUSCAR DOCTOR POR ID ---" << endl;
            int idDoctor;
            cout << "Ingrese ID del doctor: ";
            cin >> idDoctor;
            limpiarBuffer();

            Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
            if (doctor) {
                cout << " Doctor encontrado:" << endl;
                cout << "ID: " << doctor -> id << endl;
                cout << "Nombre: " << doctor -> nombre << " " << doctor -> apellido << endl;
                cout << "Especialidad: " << doctor -> especialidad << endl;
                cout << "anios experiencia: " << doctor -> aniosExperiencia << endl;
                cout << "Costo consulta: $" << doctor -> costoConsulta << endl;
                cout << "Estado: " << (doctor -> disponible ? " Disponible" : " No disponible") << endl;
            } else {
                cout << " No se encontrÃ³ doctor con ese ID." << endl;
            }
            break;
        }

        case 3: {
            cout << "\n--- BUSCAR DOCTORES POR ESPECIALIDAD ---" << endl;
            char especialidad[50];
            cout << "Ingrese especialidad: ";
            cin.getline(especialidad, sizeof(especialidad));

            int cantidad;
            Doctor ** resultados = buscarDoctoresPorEspecialidad(hospital, especialidad, & cantidad);
            if (resultados) {
                cout << "?? Encontrados " << cantidad << " doctores en " << especialidad << ":" << endl;
                for (int i = 0; i < cantidad; i++) {
                    cout << (i + 1) << ". " << resultados[i] -> nombre << " " <<
                        resultados[i] -> apellido << " (ID: " <<
                        resultados[i] -> id << ", Experiencia: " <<
                        resultados[i] -> aniosExperiencia << " anios)" << endl;
                }
                liberarResultadosDoctores(resultados);
            } else {
                cout << " No se encontraron doctores con esa especialidad." << endl;
            }
            break;
        }

        case 4: {
            cout << "\n--- ASIGNAR PACIENTE A DOCTOR ---" << endl;
            int idDoctor, idPaciente;

            cout << "ID del doctor: ";
            cin >> idDoctor;
            limpiarBuffer();
            cout << "ID del paciente: ";
            cin >> idPaciente;
            limpiarBuffer();

            // Verificar que existan ambos
            Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
            Paciente * paciente = buscarPacientePorId(hospital, idPaciente);

            if (!doctor) {
                cout << " No se encontrÃ³ doctor con ID " << idDoctor << endl;
                break;
            }
            if (!paciente) {
                cout << " No se encontrÃ³ paciente con ID " << idPaciente << endl;
                break;
            }

            bool resultado = asignarpacienteadoctor(hospital, idDoctor, idPaciente);
            if (resultado) {
                cout << " Paciente asignado exitosamente al doctor." << endl;
                cout << " Paciente: " << paciente -> nombre << " " << paciente -> apellido << endl;
                cout << " Doctor: " << doctor -> nombre << " " << doctor -> apellido << endl;
            } else {
                cout << " No se pudo asignar el paciente al doctor." << endl;
            }
            break;
        }

        case 5: {
            cout << "\n--- VER PACIENTES ASIGNADOS A DOCTOR ---" << endl;
            int idDoctor;
            cout << "Ingrese ID del doctor: ";
            cin >> idDoctor;
            limpiarBuffer();

            Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
            if (!doctor) {
                cout << " No se encontrÃ³ doctor con ID " << idDoctor << endl;
                break;
            }

            cout << "\n Pacientes asignados al Dr. " << doctor -> nombre << " " << doctor -> apellido << ":" << endl;
            cout << "Total: " << doctor -> cantidadPacientes << " pacientes" << endl;

            if (doctor -> cantidadPacientes == 0) {
                cout << "No hay pacientes asignados." << endl;
                break;
            }

            cout << "\n" << left << setw(8) << "ID" <<
                setw(25) << "NOMBRE COMPLETO" <<
                setw(15) << "CÃ‰DULA" <<
                setw(6) << "EDAD" << endl;
            cout << string(60, '-') << endl;

            for (int i = 0; i < doctor -> cantidadPacientes; i++) {
                Paciente * paciente = buscarPacientePorId(hospital, doctor -> pacientesAsignados[i]);
                if (paciente) {
                    cout << left << setw(8) << paciente -> id <<
                        setw(25) << (string(paciente -> nombre) + " " + paciente -> apellido) <<
                        setw(15) << paciente -> cedula <<
                        setw(6) << paciente -> edad << endl;
                }
            }
            break;
        }

        case 6: {
            cout << "\n--- LISTA DE TODOS LOS DOCTORES ---" << endl;
            if (hospital -> cantidadDoctores == 0) {
                cout << "No hay doctores registrados en el sistema." << endl;
                break;
            }

            cout << left << setw(6) << "ID" <<
                setw(20) << "NOMBRE COMPLETO" <<
                setw(20) << "ESPECIALIDAD" <<
                setw(8) << "EXP" <<
                setw(10) << "COSTO" <<
                setw(12) << "ESTADO" << endl;
            cout << string(80, '-') << endl;

            for (int i = 0; i < hospital -> cantidadDoctores; i++) {
                Doctor * d = & hospital -> doctores[i];
                cout << left << setw(6) << d -> id <<
                    setw(20) << (string(d -> nombre) + " " + d -> apellido) <<
                    setw(20) << d -> especialidad <<
                    setw(8) << d -> aniosExperiencia <<
                    setw(10) << d -> costoConsulta <<
                    setw(12) << (d -> disponible ? " Activo" : " Inactivo") << endl;
            }
            cout << string(80, '-') << endl;
            cout << "Total: " << hospital -> cantidadDoctores << " doctores" << endl;
            break;
        }

        case 7: {
            cout << "\n--- ELIMINAR DOCTOR ---" << endl;
            int idDoctor;
            cout << "Ingrese ID del doctor a eliminar: ";
            cin >> idDoctor;
            limpiarBuffer();

            // Buscar doctor primero para mostrar informaciÃ³n
            Doctor * doctor = buscarDoctorPorId(hospital, idDoctor);
            if (!doctor) {
                cout << " No se encontrÃ³ doctor con ID " << idDoctor << endl;
                break;
            }

            // Mostrar informaciÃ³n del doctor
            cout << "\n  INFORMACIÃ“N DEL DOCTOR A ELIMINAR:" << endl;
            cout << "ID: " << doctor -> id << endl;
            cout << "Nombre: " << doctor -> nombre << " " << doctor -> apellido << endl;
            cout << "Especialidad: " << doctor -> especialidad << endl;
            cout << "Pacientes asignados: " << doctor -> cantidadPacientes << endl;
            cout << "Citas agendadas: " << doctor -> cantidadCitas << endl;

            // Doble confirmaciÃ³n
            char confirmacion;
            cout << "Â¿EstÃ¡ seguro de eliminar este doctor? (s/n): ";
            cin >> confirmacion;
            limpiarBuffer();

            if (tolower(confirmacion) == 's') {
                cout << " LOS DATOS SE PERDERÃN PERMANENTEMENTE. Â¿Continuar? (s/n): ";
                cin >> confirmacion;
                limpiarBuffer();

                if (tolower(confirmacion) == 's') {
                    bool resultado = eliminarDoctor(hospital, idDoctor);
                    if (resultado) {
                        cout << " Doctor eliminado exitosamente." << endl;
                    } else {
                        cout << " No se pudo eliminar el doctor." << endl;
                    }
                } else {
                    cout << " EliminaciÃ³n cancelada." << endl;
                }
            } else {
                cout << " EliminaciÃ³n cancelada." << endl;
            }
            break;
        }

        case 0:
            cout << "Volviendo al menÃº principal..." << endl;
            break;
        }

        if (opcion != 0) {
            pausarPantalla();
        }

    } while (opcion != 0);
}

// MENÃš DE GESTIÃ“N DE CITAS
void menuGestionCitas(Hospital * hospital) {
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
        cout << "6.  Ver citas de una fecha" << endl;
        cout << "7.  Ver citas pendientes" << endl;
        cout << "0.  Volver al menu principal" << endl;
        cout << "===============================================" << endl;

        opcion = leerOpcion("Seleccione una opcion", 0, 7);

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

            Cita * cita = agendarCita(hospital, idPaciente, idDoctor, fecha, hora, motivo);
            if (cita) {
                cout << " Cita agendada exitosamente. ID: " << cita -> id << endl;
            }
            break;
        }

        case 2: {
            cout << "\n--- CANCELAR CITA ---" << endl;
            int idCita;
            cout << "Ingrese ID de la cita a cancelar: ";
            cin >> idCita;
            limpiarBuffer();

            bool resultado = cancelarCita(hospital, idCita);
            if (resultado) {
                cout << " Cita cancelada exitosamente." << endl;
            } else {
                cout << " No se pudo cancelar la cita." << endl;
            }
            break;
        }

        case 3: {
            cout << "\n--- ATENDER CITA ---" << endl;
            int idCita;
            char diagnostico[200], tratamiento[200], medicamentos[150];

            cout << "Ingrese ID de la cita a atender: ";
            cin >> idCita;
            limpiarBuffer();

            cout << "Diagnostico: ";
            cin.getline(diagnostico, sizeof(diagnostico));
            cout << "Tratamiento: ";
            cin.getline(tratamiento, sizeof(tratamiento));
            cout << "Medicamentos: ";
            cin.getline(medicamentos, sizeof(medicamentos));

            bool resultado = atenderCita(hospital, idCita, diagnostico, tratamiento, medicamentos);
            if (resultado) {
                cout << " Cita atendida exitosamente." << endl;
            } else {
                cout << " No se pudo atender la cita." << endl;
            }
            break;
        }

        case 4: {
            cout << "\n--- VER CITAS DE UN PACIENTE ---" << endl;
            int idPaciente;
            cout << "Ingrese ID del paciente: ";
            cin >> idPaciente;
            limpiarBuffer();

            int cantidad;
            Cita ** citas = obtenerCitasDePaciente(hospital, idPaciente, & cantidad);
            if (citas) {
                cout << "\nCitas del paciente ID " << idPaciente << ":" << endl;
                cout << "Total: " << cantidad << " citas" << endl;

                cout << left << setw(8) << "ID" <<
                    setw(12) << "FECHA" <<
                    setw(8) << "HORA" <<
                    setw(8) << "DOCTOR" <<
                    setw(20) << "MOTIVO" <<
                    setw(12) << "ESTADO" << endl;
                cout << string(70, '-') << endl;

                for (int i = 0; i < cantidad; i++) {
                    Cita * c = citas[i];
                    cout << left << setw(8) << c -> id <<
                        setw(12) << c -> fecha <<
                        setw(8) << c -> hora <<
                        setw(8) << c -> idDoctor <<
                        setw(20) << (strlen(c -> motivo) > 18 ? string(c -> motivo).substr(0, 15) + "..." : c -> motivo) <<
                        setw(12) << c -> estado << endl;
                }
                liberarResultadosCitas(citas);
            } else {
                cout << " No se encontraron citas para este paciente." << endl;
            }
            break;
        }

        case 5: {
            cout << "\n--- VER CITAS DE UN DOCTOR ---" << endl;
            int idDoctor;
            cout << "Ingrese ID del doctor: ";
            cin >> idDoctor;
            limpiarBuffer();

            int cantidad;
            Cita ** citas = obtenerCitasDeDoctor(hospital, idDoctor, & cantidad);
            if (citas) {
                cout << "\nCitas del doctor ID " << idDoctor << ":" << endl;
                cout << "Total: " << cantidad << " citas" << endl;

                cout << left << setw(8) << "ID" <<
                    setw(12) << "FECHA" <<
                    setw(8) << "HORA" <<
                    setw(8) << "PACIENTE" <<
                    setw(20) << "MOTIVO" <<
                    setw(12) << "ESTADO" << endl;
                cout << string(70, '-') << endl;

                for (int i = 0; i < cantidad; i++) {
                    Cita * c = citas[i];
                    cout << left << setw(8) << c -> id <<
                        setw(12) << c -> fecha <<
                        setw(8) << c -> hora <<
                        setw(8) << c -> idPaciente <<
                        setw(20) << (strlen(c -> motivo) > 18 ? string(c -> motivo).substr(0, 15) + "..." : c -> motivo) <<
                        setw(12) << c -> estado << endl;
                }
                liberarResultadosCitas(citas);
            } else {
                cout << " No se encontraron citas para este doctor." << endl;
            }
            break;
        }

        case 6: {
            cout << "\n--- VER CITAS DE UNA FECHA ---" << endl;
            char fecha[11];
            cout << "Ingrese fecha (YYYY-MM-DD): ";
            cin.getline(fecha, sizeof(fecha));

            int cantidad;
            Cita ** citas = obtenerCitasPorFecha(hospital, fecha, & cantidad);
            if (citas) {
                cout << "\nCitas para la fecha " << fecha << ":" << endl;
                cout << "Total: " << cantidad << " citas" << endl;

                cout << left << setw(8) << "ID" <<
                    setw(8) << "HORA" <<
                    setw(8) << "PACIENTE" <<
                    setw(8) << "DOCTOR" <<
                    setw(20) << "MOTIVO" <<
                    setw(12) << "ESTADO" << endl;
                cout << string(65, '-') << endl;

                for (int i = 0; i < cantidad; i++) {
                    Cita * c = citas[i];
                    cout << left << setw(8) << c -> id <<
                        setw(8) << c -> hora <<
                        setw(8) << c -> idPaciente <<
                        setw(8) << c -> idDoctor <<
                        setw(20) << (strlen(c -> motivo) > 18 ? string(c -> motivo).substr(0, 15) + "..." : c -> motivo) <<
                        setw(12) << c -> estado << endl;
                }
                liberarResultadosCitas(citas);
            } else {
                cout << " No se encontraron citas para esta fecha." << endl;
            }
            break;
        }

        case 7: {
            cout << "\n--- CITAS PENDIENTES ---" << endl;
            listarCitasPendientes(hospital);
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

using namespace std;

int main() {
    setlocale(LC_ALL, "es_ES.UTF-8");

    cout << "SISTEMA DE GESTIÃ“N HOSPITALARIA" << endl;
    cout << "===================================" << endl;

    Hospital * hospital = inicializarHospital("Hospital el callao", 10, 5, 20);
    if (!hospital) {
        cout << "Error: No se pudo inicializar el hospital." << endl;
        return 1;
    }

    int opcion;

    do {
        system("cls");
        mostrarMenuPrincipal();
        opcion = leerOpcion("Seleccione una opciÃ³n", 0, 4);

        switch (opcion) {
        case 1:
            menuGestionPacientes(hospital);
            break;
        case 2:
            menuGestionDoctores(hospital);
            break;
        case 3:
            menuGestionCitas(hospital);
            break;
        case 0:
            cout << "Saliendo del programa..." << endl;
            break;
        default:
            cout << "OpciÃ³n no vÃ¡lida, ingrese una opciÃ³n vÃ¡lida." << endl;
        }

    } while (opcion != 0);

    if (hospital) {
        destruirHospital(hospital);

        cout << "Programa terminado correctamente." << endl;
        hospital = nullptr;
    }

    return 0;
}
