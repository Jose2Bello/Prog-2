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
