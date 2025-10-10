#include <iostream>
#include <locale>
using namespace std;

// Función MUY simple para probar
void mostrarMenuSimple() {
    cout << "1. Crear arreglo" << endl;
    cout << "2. Mostrar arreglo" << endl;
    cout << "3. Salir" << endl;
    cout << "Opcion: ";
}

int main() {
    setlocale(LC_ALL, "spanish");
    
    cout << "=== PRUEBA SEGURA ===" << endl;
    
    int opcion = 0;
    
    while (opcion != 3) {
        mostrarMenuSimple();
        cin >> opcion;
        
        switch (opcion) {
            case 1:
                cout << "Creando arreglo..." << endl;
                break;
            case 2:
                cout << "Mostrando arreglo..." << endl;
                break;
            case 3:
                cout << "Saliendo..." << endl;
                break;
            default:
                cout << "Opcion invalida" << endl;
                break;
        }
    }
    
    return 0;
}
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
