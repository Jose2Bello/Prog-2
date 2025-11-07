#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <ios>
using namespace std;


struct Producto {
    int codigo;
    char nombre[50];
    float precio;
    int stock;
    int stockminimo;
};

Producto* inventario = nullptr;
int cantidad = 0;
int capacidad = 0;




void liberarInventario(Producto*& inventario) {
    if (inventario != nullptr) {
        delete[] inventario;
        inventario = nullptr;
        cout << "Memoria del inventario liberada." << endl;
    }
}
void agregarProducto(Producto* inventario, int& cantidad) {
    const int MAX_PRODUCTOS = 100;

    if (cantidad >= MAX_PRODUCTOS) {
        cout << "Error: El inventario está lleno. No se pueden agregar más productos." << endl;
        return;
    }

    Producto nuevoProducto;

    cout << "=== AGREGAR NUEVO PRODUCTO ===" << endl;

    bool codigoExiste;
    do {
        codigoExiste = false;
        cout << "Código del producto: ";
        cin >> nuevoProducto.codigo;

        for (int i = 0; i < cantidad; i++) {
            if (inventario[i].codigo == nuevoProducto.codigo) {
                cout << "Error: El código ya existe. Ingrese un código diferente." << endl;
                codigoExiste = true;
                break;
            }
        }
    } while (codigoExiste);

    cin.ignore();

    cout << "Nombre del producto: ";
    cin.getline(nuevoProducto.nombre, 100);

    cout << "Precio del producto: ";
    cin >> nuevoProducto.precio;

    cout << "Stock del producto: ";
    cin >> nuevoProducto.stock;

    inventario[cantidad] = nuevoProducto;

    cantidad++;

    cout << "Producto agregado exitosamente!" << endl;
}





void mostrarProductos(Producto* inventario, int cantidad) {

    if (cantidad == 0) {
        cout << "No hay productos en el inventario." << endl;
        return;
    }

    cout << "\n";
    cout << "==================================================================================" << endl;
    cout << "                              INVENTARIO DE PRODUCTOS" << endl;
    cout << "==================================================================================" << endl;
    cout << left << setw(8) << "Código"
         << left << setw(25) << "Nombre"
         << left << setw(12) << "Precio"
         << left << setw(8) << "Stock"
         << left << setw(12) << "Stock Min." << endl;
    cout << "----------------------------------------------------------------------------------" << endl;


    for (int i = 0; i < cantidad; i++) {
        cout << left << setw(8) << inventario[i].codigo
             << left << setw(25) << inventario[i].nombre
             << left << setw(12) << fixed << setprecision(2) << inventario[i].precio
             << left << setw(8) << inventario[i].stock
             << left << setw(12) << inventario[i].stockminimo << endl;
    }

    cout << "==================================================================================" << endl;
    cout << "Total de productos: " << cantidad << endl;
    cout << endl;
}

int buscarProducto(Producto* inventario, int cantidad, int codigo) {

    for (int i = 0; i < cantidad; i++) {

        if (inventario[i].codigo == codigo) {
            return i;
        }
    }
    return -1;
}
void modificarStock(Producto* inventario, int cantidad) {
    int codigo;
    cout << "=== MODIFICAR STOCK ===" << endl;
    cout << "Ingrese el código del producto: ";
    cin >> codigo;

    int indice = buscarProducto(inventario, cantidad, codigo);

    if (indice == -1) {
        cout << "Error: Producto no encontrado." << endl;
        return;
    }

    cout << "\nProducto: " << inventario[indice].nombre << endl;
    cout << "Stock actual: " << inventario[indice].stock << endl;
    cout << "Stock mínimo: " << inventario[indice].stockminimo << endl;

    int nuevoStock;
    cout << "Nuevo stock: ";
    cin >> nuevoStock;

    if (nuevoStock < 0) {
        cout << "Error: Stock no puede ser negativo." << endl;
        return;
    }


    char confirmar;
    cout << "¿Está seguro de cambiar el stock de " << inventario[indice].stock
         << " a " << nuevoStock << "? (s/n): ";
    cin >> confirmar;

    if (confirmar == 's' || confirmar == 'S') {
        inventario[indice].stock = nuevoStock;
        cout << "Stock actualizado exitosamente!" << endl;


        if (nuevoStock < inventario[indice].stockminimo) {
            cout << "ADVERTENCIA: Stock por debajo del mínimo establecido." << endl;
        }
    } else {
        cout << "Operación cancelada." << endl;
    }
}


void guardarInventario(Producto* inventario, int cantidad, const string& nombreArchivo) {

    ofstream archivo(nombreArchivo, ios::binary);

    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo " << nombreArchivo << " para escritura." << endl;
        return;
    }

    archivo.write(reinterpret_cast<const char*>(&cantidad), sizeof(cantidad));


    for (int i = 0; i < cantidad; i++) {
        archivo.write(reinterpret_cast<const char*>(&inventario[i]), sizeof(Producto));
    }

    archivo.close();

    cout << "Inventario guardado exitosamente en " << nombreArchivo << endl;
    cout << "Total de productos guardados: " << cantidad << endl;
}

void cargarInventario(Producto*& inventario, int& cantidad, int& capacidad, const string& nombreArchivo) {

    ifstream archivo(nombreArchivo, ios::binary);

    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo " << nombreArchivo << " para lectura." << endl;
        cout << "Se iniciará con un inventario vacío." << endl;
        return;
    }
    int cantidadLeida;
    archivo.read(reinterpret_cast<char*>(&cantidadLeida), sizeof(cantidadLeida));
    if (cantidadLeida <= 0 || cantidadLeida > capacidad) {
        cout << "Error: Cantidad de productos inválida en el archivo." << endl;
        archivo.close();
        return;
    }

    for (int i = 0; i < cantidadLeida; i++) {

        if (cantidad >= capacidad) {
            cout << "Advertencia: Capacidad del inventario excedida. Se cargarán solo "
                 << capacidad << " productos." << endl;
            break;
        }

        archivo.read(reinterpret_cast<char*>(&inventario[cantidad]), sizeof(Producto));
        cantidad++;
    }

    archivo.close();

    cout << "Inventario cargado exitosamente desde " << nombreArchivo << endl;
    cout << "Productos cargados: " << cantidadLeida << endl;
}
void reporteStockBajo(Producto* inventario, int cantidad) {
    int productosConStockBajo = 0;
    int totalProductosBajo = 0;

    cout << "\n";
    cout << "================================================================" << endl;
    cout << "                   REPORTE DE STOCK BAJO" << endl;
    cout << "================================================================" << endl;

    bool hayStockBajo = false;

    for (int i = 0; i < cantidad; i++) {
        if (inventario[i].stock < inventario[i].stockminimo) {
            if (!hayStockBajo) {
                cout << left << setw(8) << "Código"
                     << left << setw(25) << "Nombre"
                     << left << setw(8) << "Stock"
                     << left << setw(12) << "Stock Min."
                     << left << setw(10) << "Déficit" << endl;
                cout << "----------------------------------------------------------------" << endl;
                hayStockBajo = true;
            }

            int deficit = inventario[i].stockminimo - inventario[i].stock;
            cout << left << setw(8) << inventario[i].codigo
                 << left << setw(25) << inventario[i].nombre
                 << left << setw(8) << inventario[i].stock
                 << left << setw(12) << inventario[i].stockminimo
                 << left << setw(10) << deficit << endl;

            productosConStockBajo++;
            totalProductosBajo += deficit;
        }
    }
    cout << "================================================================" << endl;

    if (!hayStockBajo) {
        cout << "No hay productos con stock bajo." << endl;
    } else {
        cout << "ESTADÍSTICAS:" << endl;
        cout << "• Productos con stock bajo: " << productosConStockBajo << " de " << cantidad << endl;
        cout << "• Porcentaje de productos bajos: " << fixed << setprecision(1)
             << (productosConStockBajo * 100.0 / cantidad) << "%" << endl;
        cout << "• Total de unidades en déficit: " << totalProductosBajo << endl;

        int productosSinStock = 0;
        for (int i = 0; i < cantidad; i++) {
            if (inventario[i].stock == 0) {
                productosSinStock++;
            }
        }

        if (productosSinStock > 0) {
            cout << "CRÍTICO: " << productosSinStock << " producto(s) sin stock disponible." << endl;
        }
    }
    cout << endl;
}

void mostrarMenu() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════╗" << endl;
    cout << "║           SISTEMA DE INVENTARIO          ║" << endl;
    cout << "╠══════════════════════════════════════════╣" << endl;
    cout << "║  1. Agregar nuevo producto               ║" << endl;
    cout << "║  2. Mostrar inventario completo          ║" << endl;
    cout << "║  3. Buscar producto por código           ║" << endl;
    cout << "║  4. Modificar stock de producto          ║" << endl;
    cout << "║  5. Generar reporte de stock bajo        ║" << endl;
    cout << "║  6. Guardar inventario (archivo binario) ║" << endl;
    cout << "║  7. Cargar inventario (archivo binario)  ║" << endl;
    cout << "║  8. Salir del sistema                    ║" << endl;
    cout << "╚══════════════════════════════════════════╝" << endl;
    cout << "Opción: ";
}

void inicializarInventario(Producto*& inventario, int& capacidad, int capacidadInicial = 50) {
    if (inventario != nullptr) {
        delete[] inventario;
    }

    inventario = new Producto[capacidadInicial];
    capacidad = capacidadInicial;
    cout << "Inventario inicializado con capacidad para " << capacidadInicial << " productos." << endl;
}



int main() {
    int MAX_PRODUCTOS = 100;
    Producto* inventario = new Producto[MAX_PRODUCTOS];
    int cantidad = 0;
    string nombreArchivo = "inventario.bin";
    int opcion;

    cout << "=== SISTEMA DE GESTIÓN DE INVENTARIO ===" << endl;

    cargarInventario(inventario, cantidad, MAX_PRODUCTOS, nombreArchivo);

    do {
        system("cls");
        mostrarMenu();
        cin >> opcion;

        switch (opcion) {
            case 1:
                agregarProducto(inventario, cantidad);
        
                break;
            case 2:
                mostrarProductos(inventario, cantidad);
                break;
            case 3:
                {
                    int codigo;
                    cout << "Ingrese código a buscar: ";
                    cin >> codigo;
                    int indice = buscarProducto(inventario, cantidad, codigo);
                    if (indice != -1) {
                        cout << "Producto encontrado: " << inventario[indice].nombre << endl;
                        cout << "Precio: $" << inventario[indice].precio << endl;
                        cout << "Stock: " << inventario[indice].stock << endl;
                    } else {
                        cout << "Producto no encontrado." << endl;
                    }
                }
                break;
            case 4:
                modificarStock(inventario, cantidad);
                break;
            case 5:
                guardarInventario(inventario, cantidad, nombreArchivo);
                break;
            case 6:
                cargarInventario(inventario, cantidad, MAX_PRODUCTOS, nombreArchivo);
                break;
            case 7:
                reporteStockBajo(inventario, cantidad);
                break;
            case 8:
                cout << "Saliendo del sistema..." << endl;
                break;
            default:
                cout << "Opción inválida. Intente de nuevo." << endl;
        }

        if (opcion != 8) {
            cout << "\nPresione Enter para continuar...";
            cin.ignore();
            cin.get();
        }

    } while (opcion != 8);

    guardarInventario(inventario, cantidad, nombreArchivo);
    delete[] inventario;

    cout << "Inventario guardado. ¡Hasta pronto!" << endl;
    return 0;
}