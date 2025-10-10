#include <iostream>
#include <locale>
#include <iomanip>
using namespace std;

int* crearArreglo(int tamanio) {
    if (tamanio <= 0) {
        cout << "El tamaño debe ser un número positivo." << endl;
        return nullptr;
    }
    int* arreglo = new int[tamanio];
    if (arreglo == nullptr) {
        cout << "No se pudo asignar memoria." << endl;
        return nullptr;
    }
    cout << "Arreglo de tamaño " << tamanio << " creado exitosamente." << endl;
    return arreglo;
}
void llenarArreglo(int* arreglo, int tamanio) {

    if (arreglo == nullptr) {
        cout << "El arreglo es invalido" << endl;
        return;
    }
    if (tamanio <= 0) {
        cout << "Tamaño invalido, ingrese de nuevo." << endl;
        return;
    }
    cout << "Ingrese los valores para el arreglo:" << endl;
    for (int i = 0; i < tamanio; i++) {
        cout << "Valor" << i << ":";
        cin >> arreglo[i];
    }
    cout << "Arreglo lleno." << endl;
}

void mostrarArreglo(int* arreglo, int tamanio) {
    if (arreglo == nullptr) {
        cout << "Error: El arreglo no está inicializado (nullptr)." << endl;
        return;
    }
    
    if (tamanio <= 0) {
        cout << "El arreglo está vacío o tiene tamaño 0." << endl;
        return;
    }
    cout << "Elementos del arreglo:" << endl;
    for (int s = 0; s < tamanio; s++) {
        cout << "[" << s << "] = " << arreglo[s] << endl;
    }
}

// Función para encontrar el número mayor
int encontrarMayor(int* arreglo, int tamanio) {
    if (arreglo == nullptr || tamanio <= 0) {
        cout << "Error: Arreglo inválido o tamaño incorrecto."<< endl;
        return;
    }
    int mayor = arreglo[0];
    for (int i = 1; i < tamanio; i++) {
        if (arreglo[i] > mayor) {
            mayor = arreglo[i];
        }
    }
    return mayor; 
}

// Función para calcular el promedio
float calcularPromedio(int* arreglo, int tamanio) {
 if (arreglo == nullptr || tamanio <= 0) {
    cout << "Error: Arreglo inválido o tamaño incorrecto."<< endl;
    return 0.0f;
}
int promediototal = 0;
for (int u = 0; u < tamanio; u++){
    promediototal += arreglo[u];
}
float promedio = static_cast<float>(promediototal) / tamanio;
return promedio;
}

// Función para liberar la memoria del arreglo
void liberarArreglo(int*& arreglo) {
      if (arreglo == nullptr) {
        cout << "Error: Arreglo inválido o tamaño incorrecto."<< endl;
        return;
    }
    delete[] arreglo;
    arreglo = nullptr;
    cout <<"Memoria liberada correctamente" << endl;
}

// Función para mostrar el menú
void mostrarMenu() {
    cout << "\n=== GESTIÓN DE ARREGLOS DINÁMICOS ===" << endl;
    cout << "1. Crear y llenar arreglo" << endl;
    cout << "2. Mostrar arreglo" << endl;
    cout << "3. Encontrar número mayor" << endl;
    cout << "4. Calcular promedio" << endl;
    cout << "5. Salir" << endl;
    cout << "Seleccione una opción: ";
}

int main() {
    setlocale(LC_ALL, "spanish");
    
    // Variables principales
    int* arreglo = nullptr;
    int tamanio = 0;
    
    cout << "=== GESTIÓN DE ARREGLOS DINÁMICOS ===" << endl;
    cout << "Implemente las funciones marcadas con TODO para completar el ejercicio." << endl;
    
    int opcion;
    do {
        mostrarMenu();
        cin >> opcion;
        
        switch (opcion) {
            case 1: {
                // Crear y llenar arreglo
                cout << "Ingrese el tamaño del arreglo: ";
                cin >> tamanio;
                
                arreglo = crearArreglo(tamanio);
                if (arreglo != nullptr) {
                    llenarArreglo(arreglo, tamanio);
                    cout << "Arreglo creado y llenado correctamente." << endl;
                } else {
                    cout << "Error al crear el arreglo." << endl;
                }
                break;
            }
            
            case 2: {
                // Mostrar arreglo
                if (arreglo != nullptr) {
                    mostrarArreglo(arreglo, tamanio);
                } else {
                    cout << "No hay arreglo creado. Use la opción 1 primero." << endl;
                }
                break;
            }
            
            case 3: {
                // Encontrar número mayor
                if (arreglo != nullptr && tamanio > 0) {
                    int mayor = encontrarMayor(arreglo, tamanio);
                    cout << "El número mayor es: " << mayor << endl;
                } else {
                    cout << "No hay arreglo creado o está vacío." << endl;
                }
                break;
            }
            
            case 4: {
                // Calcular promedio
                if (arreglo != nullptr && tamanio > 0) {
                    float promedio = calcularPromedio(arreglo, tamanio);
                    cout << "El promedio es: " << fixed << setprecision(2) << promedio << endl;
                } else {
                    cout << "No hay arreglo creado o está vacío." << endl;
                }
                break;
            }
            
            case 5: {
                cout << "Saliendo del programa..." << endl;
                break;
            }
            
            default: {
                cout << "Opción inválida. Intente nuevamente." << endl;
                break;
            }
        }
    } while (opcion != 5);
    
    // Liberar memoria antes de salir
    liberarArreglo(arreglo);
    
    cout << "Programa finalizado. Memoria liberada." << endl;
    return 0;
}

/*
 * PISTAS PARA LA IMPLEMENTACIÓN:
 * 
 * 1. crearArreglo(int tamanio):
 *    - Verificar que tamanio > 0
 *    - Crear arreglo con: int* nuevoArreglo = new int[tamanio];
 *    - Verificar que new no retorne nullptr
 *    - Retornar puntero al arreglo
 * 
 * 2. llenarArreglo(int* arreglo, int tamanio):
 *    - Verificar que arreglo != nullptr
 *    - Usar un ciclo for para pedir valores
 *    - Usar cin para leer cada valor
 *    - Asignar a arreglo[i]
 * 
 * 3. mostrarArreglo(int* arreglo, int tamanio):
 *    - Verificar que arreglo != nullptr
 *    - Usar un ciclo for para mostrar valores
 *    - Mostrar cada elemento con cout
 * 
 * 4. encontrarMayor(int* arreglo, int tamanio):
 *    - Verificar que arreglo != nullptr y tamanio > 0
 *    - Inicializar variable mayor con arreglo[0]
 *    - Usar ciclo for para comparar con el resto
 *    - Actualizar mayor si encuentra uno más grande
 *    - Retornar mayor
 * 
 * 5. calcularPromedio(int* arreglo, int tamanio):
 *    - Verificar que arreglo != nullptr y tamanio > 0
 *    - Inicializar variable suma = 0
 *    - Usar ciclo for para sumar todos los elementos
 *    - Dividir suma entre tamanio
 *    - Retornar promedio como float
 * 
 * 6. liberarArreglo(int*& arreglo):
 *    - Verificar que arreglo != nullptr
 *    - Usar: delete[] arreglo;
 *    - Asignar: arreglo = nullptr;
 * 
 * CONCEPTOS CLAVE:
 * - new int[tamanio] crea un arreglo dinámico
 * - delete[] libera memoria de arreglos
 * - nullptr indica que un puntero no apunta a nada
 * - Paso por referencia (&) permite modificar el puntero
 * - Siempre verificar punteros antes de usar
 */
