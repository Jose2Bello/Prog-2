#include "../Pacientes/operacionesPacientes.hpp"
#include "../persistencia/GestorArchivos.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>

using namespace std;

bool OperacionesPacientes::agregarPaciente(Paciente& paciente) {
    cout << "\n=== REGISTRANDO PACIENTE ===" << endl;
    
    // Validaciones básicas
    if (strlen(paciente.getNombre()) == 0 || strlen(paciente.getApellido()) == 0) {
        cout << "Error: Nombre y apellido son obligatorios" << endl;
        return false;
    }

    if (strlen(paciente.getCedula()) == 0) {
        cout << "Error: La cedula es obligatoria" << endl;
        return false;
    }

    // Verificar cédula única
    if (existePacienteConCedula(paciente.getCedula())) {
        cout << "Error: Ya existe un paciente con la cedula " << paciente.getCedula() << endl;
        return false;
    }

    // Crear directorio si no existe
    system("mkdir datos 2>nul");
    
    // 1. Verificar si el archivo existe
    bool archivoNuevo = false;
    ArchivoHeader header;
    
    ifstream test("datos/pacientes.bin", ios::binary);
    if (!test.is_open()) {
        cout << "Creando archivo nuevo..." << endl;
        archivoNuevo = true;
        
        // Crear header inicial
        strcpy(header.tipoArchivo, "PACIENTES");
        header.version = 1;
        header.cantidadRegistros = 0;
        header.registrosActivos = 0;
        header.proximoID = 1;
        header.fechaCreacion = time(nullptr);
        header.fechaUltimaModificacion = header.fechaCreacion;
        
        // Crear archivo con header
        ofstream crear("datos/pacientes.bin", ios::binary);
        if (!crear.is_open()) {
            cout << "Error: No se puede crear archivo" << endl;
            return false;
        }
        crear.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
        crear.close();
    } else {
        // Leer header existente
        test.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
        test.close();
        cout << "Header leido - Proximo ID: " << header.proximoID << endl;
    }
    
    // 2. Asignar ID y preparar paciente
    paciente.setId(header.proximoID);
    paciente.setActivo(true);
    paciente.setEliminado(false);
    
    // Asegurarnos que tenga timestamps actualizados
    time_t ahora = time(nullptr);
    paciente.setFechaModificacion(ahora);
    
    // 3. Abrir archivo para agregar
    ofstream archivo("datos/pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir archivo para escribir" << endl;
        return false;
    }
    
    // 4. Escribir paciente al final
    archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
    archivo.close();
    
    // 5. Actualizar header (importante: reabrir en modo lectura/escritura)
    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;
    header.fechaUltimaModificacion = ahora;
    
    // Abrir para actualizar header
    fstream archivoHeader("datos/pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivoHeader.is_open()) {
        cout << "Error: No se puede actualizar header" << endl;
        return false;
    }
    
    archivoHeader.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    archivoHeader.close();
    
    // 6. Verificación final
    cout << "\n=== VERIFICACION ===" << endl;
    cout << "ID asignado: " << paciente.getId() << endl;
    cout << "Proximo ID para siguiente: " << header.proximoID << endl;
    
    // Verificar tamaño del archivo
    ifstream verificar("datos/pacientes.bin", ios::binary | ios::ate);
    if (verificar.is_open()) {
        long tamanio = verificar.tellg();
        verificar.close();
        
        long tamanioEsperado = sizeof(ArchivoHeader) + (header.cantidadRegistros * sizeof(Paciente));
        cout << "Tamaño archivo: " << tamanio << " bytes" << endl;
        cout << "Tamaño esperado: " << tamanioEsperado << " bytes" << endl;
        
        if (tamanio != tamanioEsperado) {
            cout << "ADVERTENCIA: Tamaño incorrecto!" << endl;
        }
    }
    
    cout << "\nPaciente registrado exitosamente!" << endl;
    return true;
}


bool OperacionesPacientes::actualizarPaciente(const Paciente& paciente) {
    if (paciente.getId() <= 0) {
        cout << "Error: ID de paciente inválido" << endl;
        return false;
    }

    // Buscar posición del paciente
    int indice = buscarIndicePorID(paciente.getId());
    if (indice == -1) {
        cout << "Error: No se puede encontrar paciente con ID " << paciente.getId() << endl;
        return false;
    }

    // Abrir archivo en modo lectura/escritura
    fstream archivo("datos/pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir pacientes.bin para actualización" << endl;
        return false;
    }

    // Posicionarse en la ubicación exacta
    long posicion = calcularPosicion(indice);
    archivo.seekp(posicion);

    if (archivo.fail()) {
        cout << "Error: No se puede posicionar en archivo" << endl;
        archivo.close();
        return false;
    }

    // Escribir estructura completa
    archivo.write(reinterpret_cast<const char*>(&paciente), sizeof(Paciente));
    archivo.flush();

    bool exito = !archivo.fail();
    archivo.close();

    if (exito) {
        cout << "Paciente ID " << paciente.getId() << " actualizado exitosamente" << endl;
    } else {
        cout << "Error al actualizar paciente ID " << paciente.getId() << endl;
    }

    return exito;
}

Paciente OperacionesPacientes::buscarPorID(int id) {
    Paciente paciente;
    
    if (id <= 0) {
        cout << "Error: ID de paciente inválido" << endl;
        return paciente;
    }

    // Abrir pacientes.bin
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir pacientes.bin" << endl;
        return paciente;
    }

    // Leer header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    if (archivo.fail()) {
        cout << "Error: No se puede leer header de pacientes.bin" << endl;
        archivo.close();
        return paciente;
    }

    // Verificar si hay registros
    if (header.cantidadRegistros == 0) {
        cout << "No hay pacientes registrados" << endl;
        archivo.close();
        return paciente;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader), ios::beg);

    // Búsqueda secuencial
    bool encontrado = false;
    int pacientesLeidos = 0;

    while (pacientesLeidos < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        if (paciente.getId() == id && !paciente.isEliminado()) {
            encontrado = true;
            break;
        }
        pacientesLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        Paciente vacio;
        cout << "Paciente con ID " << id << " no encontrado" << endl;
        return vacio;
    } else {
        cout << "Paciente encontrado: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    }

    return paciente;
}

Paciente OperacionesPacientes::buscarPorCedula(const char* cedula) {
    Paciente paciente;
    
    if (!cedula || strlen(cedula) == 0) {
        cout << "Error: Cédula inválida" << endl;
        return paciente;
    }

    ifstream archivo("datos/pacientes.bin", ios::binary);
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
        if (strcmp(paciente.getCedula(), cedula) == 0 && !paciente.isEliminado()) {
            encontrado = true;
            break;
        }
        pacientesLeidos++;
    }

    archivo.close();

    if (!encontrado) {
        Paciente vacio;
        cout << "Paciente con cédula " << cedula << " no encontrado" << endl;
        return vacio;
    } else {
        cout << "Paciente encontrado: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
    }

    return paciente;
}

void OperacionesPacientes::listarPacientes(bool mostrarEliminados) {
    // Leer header para saber cuántos pacientes hay
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");

    if (strcmp(header.tipoArchivo, "INVALIDO") == 0 ||
        header.registrosActivos == 0) {
        cout << "No hay pacientes registrados." << endl;
        return;
    }

    cout << "\nLISTA DE PACIENTES (" << header.registrosActivos << "):" << endl;
    cout << "+-------------------------------------------------------------------+" << endl;
    cout << "|  ID  |       NOMBRE COMPLETO    |    CÉDULA    | EDAD | CONSULTAS |" << endl;
    cout << "+------+--------------------------+--------------+------+-----------+" << endl;

    // Abrir archivo y leer pacientes
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo de pacientes" << endl;
        return;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Paciente p;
    int pacientesMostrados = 0;

    while (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) &&
           pacientesMostrados < header.registrosActivos) {
        // Solo mostrar pacientes no eliminados (o todos si se solicita)
        if (mostrarEliminados || !p.isEliminado()) {
            char nombreCompleto[100];
            snprintf(nombreCompleto, sizeof(nombreCompleto), "%s %s", p.getNombre(), p.getApellido());

            // Truncar nombre si es muy largo
            if (strlen(nombreCompleto) > 22) {
                nombreCompleto[19] = '.';
                nombreCompleto[20] = '.';
                nombreCompleto[21] = '.';
                nombreCompleto[22] = '\0';
            }

            printf("| %4d | %-24s | %-12s | %4d | %10d |\n", 
                   p.getId(), nombreCompleto, p.getCedula(), p.getEdad(), p.getCantidadConsultas());

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

bool OperacionesPacientes::existePacienteConCedula(const char* cedula) {
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));

    Paciente paciente;
    while (archivo.read(reinterpret_cast<char*>(&paciente), sizeof(Paciente))) {
        if (!paciente.isEliminado() && strcmp(paciente.getCedula(), cedula) == 0) {
            archivo.close();
            return true;
        }
    }

    archivo.close();
    return false;
}

// Funciones privadas helper
int OperacionesPacientes::buscarIndicePorID(int id) {
    if (id <= 0) return -1;

    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) return -1;

    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    // Búsqueda secuencial por índice
    Paciente p;
    int indice = 0;

    while (indice < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente))) {
        if (p.getId() == id) {
            archivo.close();
            return indice;
        }
        indice++;
    }

    archivo.close();
    return -1;
}

long OperacionesPacientes::calcularPosicion(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
}


bool OperacionesPacientes::eliminarPaciente(int id, bool confirmar) {
    // Validación básica
    if (id <= 0) {
        cout << "Error: ID inválido" << endl;
        return false;
    }
    
    // Buscar paciente
    Paciente paciente = buscarPorID(id);
    if (paciente.getId() == -1) {
        cout << "Error: Paciente no encontrado" << endl;
        return false;
    }
    
    // Verificar si ya está eliminado
    if (paciente.isEliminado()) {
        cout << "El paciente ya está eliminado" << endl;
        return false;
    }
    
    // Confirmación
    if (confirmar) {
        cout << "\n=== CONFIRMAR ELIMINACIÓN ===" << endl;
        cout << "Paciente: " << paciente.getNombre() << " " << paciente.getApellido() << endl;
        cout << "Cédula: " << paciente.getCedula() << endl;
        cout << "ID: " << paciente.getId() << endl;
        
        cout << "\n¿Está seguro de eliminar este paciente? (s/n): ";
        char respuesta;
        cin >> respuesta;
        cin.ignore();
        
        if (tolower(respuesta) != 's') {
            cout << "Eliminación cancelada" << endl;
            return false;
        }
    }
    
    // Buscar índice del paciente
    int indice = buscarIndicePorID(id);
    if (indice == -1) {
        cout << "Error: No se encontró el índice del paciente" << endl;
        return false;
    }
    
    // Abrir archivo para actualizar
    fstream archivo("datos/pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo" << endl;
        return false;
    }
    
    // Calcular posición
    long posicion = sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
    
    // Leer paciente actual
    archivo.seekg(posicion);
    Paciente temp;
    archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente));
    
    // Marcar como eliminado
    temp.setEliminado(true);
    temp.setFechaModificacion(time(nullptr));
    
    // Escribir de vuelta
    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&temp), sizeof(Paciente));
    archivo.flush();
    
    bool exito = archivo.good();
    archivo.close();
    
    if (exito) {
        // Actualizar header
        ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.registrosActivos--;
            header.fechaUltimaModificacion = time(nullptr);
            GestorArchivos::actualizarHeader("datos/pacientes.bin", header);
        }
        
        cout << "Paciente eliminado exitosamente" << endl;
        return true;
    }
    
    cout << "Error al eliminar paciente" << endl;
    return false;
}

bool OperacionesPacientes::restaurarPaciente(int id) {
    // Validación
    if (id <= 0) {
        cout << "Error: ID inválido" << endl;
        return false;
    }
    
    // Buscar índice
    int indice = buscarIndicePorID(id);
    if (indice == -1) {
        cout << "Error: Paciente no encontrado" << endl;
        return false;
    }
    
    // Abrir archivo
    fstream archivo("datos/pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo" << endl;
        return false;
    }
    
    // Calcular posición
    long posicion = sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
    
    // Leer paciente
    archivo.seekg(posicion);
    Paciente temp;
    archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente));
    
    // Verificar si está eliminado
    if (!temp.isEliminado()) {
        cout << "El paciente no está eliminado" << endl;
        archivo.close();
        return false;
    }
    
    // Restaurar
    temp.setEliminado(false);
    temp.setFechaModificacion(time(nullptr));
    
    // Escribir de vuelta
    archivo.seekp(posicion);
    archivo.write(reinterpret_cast<const char*>(&temp), sizeof(Paciente));
    archivo.flush();
    
    bool exito = archivo.good();
    archivo.close();
    
    if (exito) {
        // Actualizar header
        ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
        if (strcmp(header.tipoArchivo, "INVALIDO") != 0) {
            header.registrosActivos++;
            header.fechaUltimaModificacion = time(nullptr);
            GestorArchivos::actualizarHeader("datos/pacientes.bin", header);
        }
        
        cout << "Paciente restaurado exitosamente" << endl;
        return true;
    }
    
    cout << "Error al restaurar paciente" << endl;
    return false;
}

int OperacionesPacientes::buscarPorNombre(const char* nombre, Paciente* resultados, int maxResultados) {
    // Validaciones
    if (!nombre || strlen(nombre) == 0 || !resultados || maxResultados <= 0) {
        cout << "Error: Parámetros inválidos" << endl;
        return 0;
    }
    
    // Abrir archivo
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo" << endl;
        return 0;
    }
    
    // Leer header
    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (archivo.fail()) {
        cout << "Error: No se puede leer header" << endl;
        archivo.close();
        return 0;
    }
    
    // Preparar nombre de búsqueda en minúsculas
    char* nombreLower = new char[strlen(nombre) + 1];
    for (int i = 0; nombre[i]; i++) {
        nombreLower[i] = tolower(nombre[i]);
    }
    nombreLower[strlen(nombre)] = '\0';
    
    // Buscar
    int encontrados = 0;
    Paciente temp;
    
    for (int i = 0; i < header.cantidadRegistros && encontrados < maxResultados; i++) {
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente));
        
        // Solo pacientes no eliminados
        if (!temp.isEliminado()) {
            // Convertir nombre del paciente a minúsculas
            char nombrePaciente[50];
            strcpy(nombrePaciente, temp.getNombre());
            for (int j = 0; nombrePaciente[j]; j++) {
                nombrePaciente[j] = tolower(nombrePaciente[j]);
            }
            
            // Buscar coincidencia (subcadena)
            if (strstr(nombrePaciente, nombreLower) != nullptr) {
                resultados[encontrados] = temp;
                encontrados++;
            }
        }
    }
    
    delete[] nombreLower;
    archivo.close();
    
    return encontrados;
}


void OperacionesPacientes::listarPacientesEliminados() {
    // Leer header
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
    
    if (strcmp(header.tipoArchivo, "INVALIDO") == 0) {
        cout << "No hay archivo de pacientes" << endl;
        return;
    }
    
    int totalEliminados = header.cantidadRegistros - header.registrosActivos;
    
    if (totalEliminados == 0) {
        cout << "No hay pacientes eliminados" << endl;
        return;
    }
    
    // Abrir archivo
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        cout << "Error: No se puede abrir el archivo" << endl;
        return;
    }
    
    cout << "\n=== PACIENTES ELIMINADOS (" << totalEliminados << ") ===" << endl;
    cout << "=========================================" << endl;
    
    // Saltar header
    archivo.seekg(sizeof(ArchivoHeader));
    
    Paciente temp;
    int contador = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente));
        
        if (temp.isEliminado()) {
            contador++;
            cout << "\n--- Paciente " << contador << " ---" << endl;
            cout << "ID: " << temp.getId() << endl;
            cout << "Nombre: " << temp.getNombre() << " " << temp.getApellido() << endl;
            cout << "Cédula: " << temp.getCedula() << endl;
            cout << "Edad: " << temp.getEdad() << endl;
            cout << "Sexo: " << temp.getSexo() << endl;
        }
    }
    
    archivo.close();
    cout << "\n=========================================" << endl;
    cout << "Total: " << contador << " paciente(s) eliminado(s)" << endl;
}

int OperacionesPacientes::contarPacientes() {
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
    return header.registrosActivos;
}

int OperacionesPacientes::contarPacientesEliminados() {
    ArchivoHeader header = GestorArchivos::leerHeader("datos/pacientes.bin");
    return header.cantidadRegistros - header.registrosActivos;
}

int OperacionesPacientes::leerTodosPacientes(Paciente* pacientes, int maxPacientes, bool soloActivos) {
    // Implementación básica
    ifstream archivo("datos/pacientes.bin", ios::binary);
    if (!archivo.is_open()) return 0;

    ArchivoHeader header;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

    int contador = 0;
    Paciente temp;

    while (contador < maxPacientes && contador < header.cantidadRegistros &&
           archivo.read(reinterpret_cast<char*>(&temp), sizeof(Paciente))) {
        if (!soloActivos || !temp.isEliminado()) {
            pacientes[contador] = temp;
            contador++;
        }
    }

    archivo.close();
    return contador;
}
