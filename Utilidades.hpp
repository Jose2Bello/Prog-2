#ifndef UTILIDADES_HPP
#define UTILIDADES_HPP

#include <string>
#include <ctime>

class Validaciones {
public:
    // Validaciones esenciales
    static bool validarFecha(const char* fecha);  // YYYY-MM-DD
    static bool validarHora(const char* hora);    // HH:MM
    static bool validarEmail(const char* email);
    static bool validarCedula(const char* cedula);
    static bool validarEdad(int edad);
    static bool validarCosto(float costo);
    static bool validarSexo(char sexo);
};

class Formatos {
public:
    // Formateo esencial
    static const char* formatearFecha(time_t tiempo);
    static void limpiarPantalla();
    static void pausarPantalla();
    static void setColor(int color);
};

class EntradaUsuario {
public:
    // Entrada esencial
    static int leerEntero(const std::string& mensaje, int min, int max);
    static std::string leerTexto(const std::string& mensaje, int maxLongitud);
    static bool leerConfirmacion(const std::string& mensaje);
};

#endif