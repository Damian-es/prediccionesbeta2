#include <stdio.h>
#include <stdlib.h>
#include "funciones.h"

void mostrarTabla(int indiceZona, const char* nombreZona, DatosZona* zona, DatosContaminacion promedio, Prediccion prediccion, int alertaActual, char* recomendacion) {
    printf("\nResultados para la zona: %s\n", nombreZona);
    printf("-------------------------------------------------------------\n");
    printf("| Dia |   CO2   |   SO2   |   NO2   |  PM2.5  |\n");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < NUM_DIAS; i++) {
        printf("| %3d | %7.2f | %7.2f | %7.2f | %7.2f |\n", i + 1,
            zona->datosDiarios[i].co2,
            zona->datosDiarios[i].so2,
            zona->datosDiarios[i].no2,
            zona->datosDiarios[i].pm25);
    }
    printf("-------------------------------------------------------------\n");
    printf("Promedios historicos:\n");
    printf("CO2: %.2f, SO2: %.2f, NO2: %.2f, PM2.5: %.2f\n",
        promedio.co2, promedio.so2, promedio.no2, promedio.pm25);
    printf("Prediccion para las proximas 24 horas:\n");
    printf("CO2: %.2f, SO2: %.2f, NO2: %.2f, PM2.5: %.2f\n",
        prediccion.nivelesPredichos.co2,
        prediccion.nivelesPredichos.so2,
        prediccion.nivelesPredichos.no2,
        prediccion.nivelesPredichos.pm25);
    printf("Alerta actual: %s\n", alertaActual ? "SI" : "NO");
    printf("Recomendacion: %s\n\n", recomendacion);
}

#include <string.h>

// Helper function to convert a string to lowercase
void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + ('a' - 'A');
        }
    }
}

// Function to search zones by substring (case-insensitive) and display matches
void buscarZonas(const char* zonasNombres[], int numZonas, const char* terminoBusqueda) {
    printf("Zonas que coinciden con '%s':\n", terminoBusqueda);
    int found = 0;
    char terminoLower[100];
    strncpy(terminoLower, terminoBusqueda, sizeof(terminoLower));
    terminoLower[sizeof(terminoLower) - 1] = '\0';
    toLowerCase(terminoLower);

    for (int i = 0; i < numZonas; i++) {
        char zonaLower[100];
        strncpy(zonaLower, zonasNombres[i], sizeof(zonaLower));
        zonaLower[sizeof(zonaLower) - 1] = '\0';
        toLowerCase(zonaLower);

        if (strstr(zonaLower, terminoLower) != NULL) {
            printf("%d. %s\n", i + 1, zonasNombres[i]);
            found = 1;
        }
    }
    if (!found) {
        printf("No se encontraron zonas que coincidan con '%s'.\n", terminoBusqueda);
    }
}

int main() {
    DatosZona zonas[NUM_ZONAS];
    Prediccion predicciones[NUM_ZONAS];
    int alertasActual[NUM_ZONAS];
    int alertasPrediccion[NUM_ZONAS];
    int alertasHistoricas[NUM_ZONAS];
    DatosContaminacion promedios[NUM_ZONAS];
    char recomendaciones[NUM_ZONAS][256];

    const char* zonasNombres[NUM_ZONAS] = {
        "Centro",
        "Norte",
        "Sur",
        "Valle",
        "Pintag"
    };

    const char* archivosDatos[NUM_ZONAS] = {
        "historico_centro.txt",
        "historico_norte.txt",
        "historico_sur.txt",
        "historico_valle.txt",
        "historico_pintag.txt"
    };

    const char* archivoClima = "datos_del_clima.txt";
    const char* archivoReporte = "reporte_contaminacion.txt";

    // Cargar datos historicos para cada zona
    for (int i = 0; i < NUM_ZONAS; i++) {
        if (!cargarDatosHistoricos(&zonas[i], archivosDatos[i])) {
            printf("No se pudo cargar los datos historicos para %s. Saliendo.\n", zonasNombres[i]);
            return 1;
        }
    }

    // Cargar datos climaticos
    DatosClimaticos clima[NUM_ZONAS];
    if (!cargarDatosClima(clima, archivoClima)) {
        printf("No se pudo cargar los datos climaticos. Saliendo.\n");
        return 1;
    }

    // Calcular contaminacion actual (ultimo dia)
    calcularContaminacionActual(zonas, NUM_DIAS - 1, alertasActual);

    // Predecir contaminacion futura
    predecirContaminacionFutura(zonas, clima, predicciones);

    // Emitir alertas para prediccion
    emitirAlertas(predicciones, alertasPrediccion);

    // Calcular promedios historicos
    calcularPromediosHistoricos(zonas, promedios, alertasHistoricas);

    // Generar recomendaciones basadas en alertas actuales y predicciones
    for (int i = 0; i < NUM_ZONAS; i++) {
        alertasActual[i] = alertasActual[i] || alertasPrediccion[i];
    }
    generarRecomendaciones(alertasActual, recomendaciones);

    int opcion = -1;
    while (opcion != 0) {
        printf("\nMenu:\n");
        printf("1. Mostrar lista de zonas y seleccionar una\n");
        printf("2. Cargar y mostrar datos de todas las zonas\n");
        printf("3. Guardar reporte\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        if (scanf("%d", &opcion) != 1) {
            while(getchar() != '\n'); // limpiar buffer
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }
        if (opcion == 1) {
            printf("Zonas disponibles:\n");
            for (int i = 0; i < NUM_ZONAS; i++) {
                printf("%d. %s\n", i + 1, zonasNombres[i]);
            }
            printf("Seleccione una zona para ver los resultados: ");
            int zonaSeleccionada;
            if (scanf("%d", &zonaSeleccionada) != 1 || zonaSeleccionada < 1 || zonaSeleccionada > NUM_ZONAS) {
                while(getchar() != '\n');
                printf("Seleccion invalida.\n");
                continue;
            }
            mostrarTabla(zonaSeleccionada - 1, zonasNombres[zonaSeleccionada - 1], &zonas[zonaSeleccionada - 1], promedios[zonaSeleccionada - 1], predicciones[zonaSeleccionada - 1], alertasActual[zonaSeleccionada - 1], recomendaciones[zonaSeleccionada - 1]);
        } else if (opcion == 2) {
            printf("Mostrando datos de todas las zonas:\n");
            for (int i = 0; i < NUM_ZONAS; i++) {
                mostrarTabla(i, zonasNombres[i], &zonas[i], promedios[i], predicciones[i], alertasActual[i], recomendaciones[i]);
            }
        } else if (opcion == 3) {
            if (!guardarReporte(zonas, predicciones, archivoReporte)) {
                printf("Error al guardar el reporte.\n");
            } else {
                printf("Reporte guardado en %s\n", archivoReporte);
            }
        } else if (opcion != 0) {
            printf("Opcion no valida. Intente de nuevo.\n");
        }
    }

    printf("Programa finalizado.\n");
    return 0;
}
