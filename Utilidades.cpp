#include "../include/Utilidades.hpp"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <windows.h>
#include <conio.h>
#include <string>

using std::cout;
using std::endl;
using std::string;


using namespace std;

// ==================== VALIDACIONES ====================

bool Validaciones::validarFecha(const char* fecha) {
    if (!fecha || strlen(fecha) != 10) return false;
    if (fecha[4] != '-' || fecha[7] != '-') return false;
    
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(fecha[i])) return false;
    }
    
    int anio = atoi(fecha);
    int mes = atoi(fecha + 5);
    int dia = atoi(fecha + 8);
    
    if (mes < 1 || mes > 12) return false;
    if (dia < 1 || dia > 31) return false;
    if (anio < 2020 || anio > 2030) return false;
    
    return true;
}

bool Validaciones::validarHora(const char* hora) {
    if (!hora || strlen(hora) != 5) return false;
    if (hora[2] != ':') return false;
    
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) return false;
    }
    
    int horas = atoi(hora);
    int minutos = atoi(hora + 3);
    return (horas >= 0 && horas <= 23 && minutos >= 0 && minutos <= 59);
}

bool Validaciones::validarEmail(const char* email) {
    if (!email || strlen(email) < 5) return false;
    
    const char* arroba = strchr(email, '@');
    if (!arroba || arroba == email) return false;
    
    const char* punto = strchr(arroba + 1, '.');
    if (!punto || punto == arroba + 1) return false;
    
    return (strchr(email, ' ') == nullptr);
}

bool Validaciones::validarCedula(const char* cedula) {
    if (!cedula || strlen(cedula) == 0) return false;
    
    for (int i = 0; cedula[i]; i++) {
        if (!isdigit(cedula[i])) return false;
    }
    
    return (strlen(cedula) >= 6 && strlen(cedula) <= 20);
}

bool Validaciones::validarEdad(int edad) {
    return (edad >= 0 && edad <= 150);
}

bool Validaciones::validarCosto(float costo) {
    return (costo >= 0);
}

bool Validaciones::validarSexo(char sexo) {
    return (sexo == 'M' || sexo == 'F');
}

// ==================== FORMATOS ====================

const char* Formatos::formatearFecha(time_t tiempo) {
    static char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", localtime(&tiempo));
    return buffer;
}

void Formatos::limpiarPantalla() {
    system("cls");
}

void Formatos::pausarPantalla() {
    cout << "\nPresione Enter para continuar...";
    cin.ignore();
    cin.get();
}

void Formatos::setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

}


void Formatos::centrartexto(const string& texto) {
    // Definiciones de consola de Windows
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int anchoConsola;

    // Obtener el tamaño del buffer de la pantalla de la consola
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        // Calcular el ancho visible de la ventana
        anchoConsola = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
        // Fallback si no se puede obtener la información de la consola (ej. 80 columnas)
        anchoConsola = 80; 
    }

    int espacios = (anchoConsola - texto.length()) / 2;

    // Asegurar que haya al menos 0 espacios
    if (espacios < 0) {
        espacios = 0;
    }
    
    // Imprimir el texto centrado
    cout << string(espacios, ' ') << texto << endl;
}

// ==================== ENTRADA USUARIO ====================

int EntradaUsuario::leerEntero(const std::string& mensaje, int min, int max) {
    int valor;
    while (true) {
        cout << mensaje << " [" << min << "-" << max << "]: ";
        cin >> valor;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Error: Por favor ingrese un numero valido." << endl;
            continue;
        }
        
        cin.ignore();
        
        if (valor >= min && valor <= max) {
            return valor;
        } else {
            cout << "Error: Valor fuera de rango. Use [" << min << "-" << max << "]" << endl;
        }
    }
}

std::string EntradaUsuario::leerTexto(const std::string& mensaje, int maxLongitud) {
    std::string entrada;
    while (true) {
        cout << mensaje << ": ";
        getline(cin, entrada);
        
        if (entrada.empty()) {
            cout << "Error: Este campo es obligatorio." << endl;
            continue;
        }
        
        if (entrada.length() > maxLongitud) {
            cout << "Error: Texto muy largo (maximo " << maxLongitud << " caracteres)." << endl;
            continue;
        }
        
        return entrada;
    }
}

bool EntradaUsuario::leerConfirmacion(const std::string& mensaje) {
    char respuesta;
    while (true) {
        cout << mensaje << " (s/n): ";
        cin >> respuesta;
        cin.ignore();
        
        respuesta = tolower(respuesta);
        if (respuesta == 's' || respuesta == 'n') {
            return (respuesta == 's');
        } else {
            cout << "Error: Por favor ingrese 's' o 'n'." << endl;
        }
    }
}