#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "funciones.h"

// Cargar datos historicos desde archivo TXT individual por zona
int cargarDatosHistoricos(DatosZona* zona, const char* nombreArchivo) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo %s\n", nombreArchivo);
        return 0;
    }
    for (int dia = 0; dia < NUM_DIAS; dia++) {
        if (fscanf(archivo, "%f %f %f %f",
            &zona->datosDiarios[dia].co2,
            &zona->datosDiarios[dia].so2,
            &zona->datosDiarios[dia].no2,
            &zona->datosDiarios[dia].pm25) != 4) {
            printf("Error al leer datos en %s para dia %d\n", nombreArchivo, dia+1);
            fclose(archivo);
            return 0;
        }
    }
    fclose(archivo);
    return 1;
}

// Cargar datos climaticos desde archivo TXT (un solo archivo para todas las zonas)
int cargarDatosClima(DatosClimaticos clima[NUM_ZONAS], const char* nombreArchivo) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo %s\n", nombreArchivo);
        return 0;
    }

    char linea[256];
    // Leer encabezado (si existe)
    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        printf("Error al leer encabezado del archivo %s\n", nombreArchivo);
        fclose(archivo);
        return 0;
    }

    const char* nombresZonas[NUM_ZONAS] = {"Centro", "Norte", "Sur", "Valle", "Pintag"};

    int idxZona;
    char zonaStr[32];
    float temp, viento, humedad;

    while (fgets(linea, sizeof(linea), archivo)) {
        if (strlen(linea) < 10) continue;

        int campos = sscanf(linea, "%31[^,],%f,%f,%f", zonaStr, &temp, &viento, &humedad);
        if (campos != 4) continue;

        idxZona = -1;
        for (int i = 0; i < NUM_ZONAS; i++) {
            if (strcmp(zonaStr, nombresZonas[i]) == 0) {
                idxZona = i;
                break;
            }
        }
        if (idxZona == -1) {
            printf("Zona desconocida en archivo clima: %s\n", zonaStr);
            continue;
        }

        clima[idxZona].temperatura = temp;
        clima[idxZona].velocidadViento = viento;
        clima[idxZona].humedad = humedad;
    }

    fclose(archivo);
    return 1;
}

// Guardar datos de contaminacion y predicciones en archivo de reporte
int guardarReporte(DatosZona zonas[NUM_ZONAS], Prediccion predicciones[NUM_ZONAS], const char* nombreArchivo) {
    FILE* archivo = fopen(nombreArchivo, "w");
    if (archivo == NULL) {
        printf("Error al abrir el archivo %s para escritura\n", nombreArchivo);
        return 0;
    }
    fprintf(archivo, "Reporte de contaminacion y predicciones\n\n");
    for (int zona = 0; zona < NUM_ZONAS; zona++) {
        fprintf(archivo, "Zona %d:\n", zona+1);
        fprintf(archivo, "Datos historicos (ultimos %d dias):\n", NUM_DIAS);
        fprintf(archivo, "Dia\tCO2\tSO2\tNO2\tPM2.5\n");
        for (int dia = 0; dia < NUM_DIAS; dia++) {
            fprintf(archivo, "%d\t%.2f\t%.2f\t%.2f\t%.2f\n", dia+1,
                zonas[zona].datosDiarios[dia].co2,
                zonas[zona].datosDiarios[dia].so2,
                zonas[zona].datosDiarios[dia].no2,
                zonas[zona].datosDiarios[dia].pm25);
        }
        fprintf(archivo, "Prediccion para las proximas 24 horas:\n");
        fprintf(archivo, "CO2: %.2f, SO2: %.2f, NO2: %.2f, PM2.5: %.2f\n\n",
            predicciones[zona].nivelesPredichos.co2,
            predicciones[zona].nivelesPredichos.so2,
            predicciones[zona].nivelesPredichos.no2,
            predicciones[zona].nivelesPredichos.pm25);
    }
    fclose(archivo);
    return 1;
}

// Calcular niveles actuales de contaminacion (ultimo dia) y comparar con limites
void calcularContaminacionActual(DatosZona zonas[NUM_ZONAS], int diaActual, int alertas[NUM_ZONAS]) {
    for (int zona = 0; zona < NUM_ZONAS; zona++) {
        alertas[zona] = 0;
        DatosContaminacion actual = zonas[zona].datosDiarios[diaActual];
        if (actual.co2 > LIMITE_CO2 || actual.so2 > LIMITE_SO2 || actual.no2 > LIMITE_NO2 || actual.pm25 > LIMITE_PM25) {
            alertas[zona] = 1;
        }
    }
}

// Predecir niveles futuros de contaminacion para las proximas 24 horas usando promedio ponderado
void predecirContaminacionFutura(DatosZona zonas[NUM_ZONAS], DatosClimaticos* clima, Prediccion predicciones[NUM_ZONAS]) {
    float pesos[NUM_DIAS];
    float sumaPesos = 0.0;
    for (int i = 0; i < NUM_DIAS; i++) {
        pesos[i] = (float)(i + 1);
        sumaPesos += pesos[i];
    }
    for (int zona = 0; zona < NUM_ZONAS; zona++) {
        float co2 = 0.0, so2 = 0.0, no2 = 0.0, pm25 = 0.0;
        for (int dia = 0; dia < NUM_DIAS; dia++) {
            co2 += zonas[zona].datosDiarios[dia].co2 * pesos[dia];
            so2 += zonas[zona].datosDiarios[dia].so2 * pesos[dia];
            no2 += zonas[zona].datosDiarios[dia].no2 * pesos[dia];
            pm25 += zonas[zona].datosDiarios[dia].pm25 * pesos[dia];
        }
        co2 /= sumaPesos;
        so2 /= sumaPesos;
        no2 /= sumaPesos;
        pm25 /= sumaPesos;

        float factorViento = 1.0 - (clima[zona].velocidadViento * 0.05f);
        if (factorViento < 0.5f) factorViento = 0.5f;
        predicciones[zona].nivelesPredichos.co2 = co2 * factorViento;
        predicciones[zona].nivelesPredichos.so2 = so2 * factorViento;
        predicciones[zona].nivelesPredichos.no2 = no2 * factorViento;
        predicciones[zona].nivelesPredichos.pm25 = pm25 * factorViento;
    }
}

// Emitir alertas si los niveles predichos exceden los limites
void emitirAlertas(Prediccion predicciones[NUM_ZONAS], int alertas[NUM_ZONAS]) {
    for (int zona = 0; zona < NUM_ZONAS; zona++) {
        alertas[zona] = 0;
        DatosContaminacion p = predicciones[zona].nivelesPredichos;
        if (p.co2 > LIMITE_CO2 || p.so2 > LIMITE_SO2 || p.no2 > LIMITE_NO2 || p.pm25 > LIMITE_PM25) {
            alertas[zona] = 1;
        }
    }
}

// Calcular promedios historicos de 30 dias y comparar con limites OMS
void calcularPromediosHistoricos(DatosZona zonas[NUM_ZONAS], DatosContaminacion promedios[NUM_ZONAS], int alertas[NUM_ZONAS]) {
    for (int zona = 0; zona < NUM_ZONAS; zona++) {
        float sumaCo2 = 0.0, sumaSo2 = 0.0, sumaNo2 = 0.0, sumaPm25 = 0.0;
        for (int dia = 0; dia < NUM_DIAS; dia++) {
            sumaCo2 += zonas[zona].datosDiarios[dia].co2;
            sumaSo2 += zonas[zona].datosDiarios[dia].so2;
            sumaNo2 += zonas[zona].datosDiarios[dia].no2;
            sumaPm25 += zonas[zona].datosDiarios[dia].pm25;
        }
        promedios[zona].co2 = sumaCo2 / NUM_DIAS;
        promedios[zona].so2 = sumaSo2 / NUM_DIAS;
        promedios[zona].no2 = sumaNo2 / NUM_DIAS;
        promedios[zona].pm25 = sumaPm25 / NUM_DIAS;

        alertas[zona] = 0;
        if (promedios[zona].co2 > LIMITE_CO2 || promedios[zona].so2 > LIMITE_SO2 || promedios[zona].no2 > LIMITE_NO2 || promedios[zona].pm25 > LIMITE_PM25) {
            alertas[zona] = 1;
        }
    }
}

// Generar recomendaciones basadas en niveles de contaminacion
void generarRecomendaciones(int alertas[NUM_ZONAS], char recomendaciones[NUM_ZONAS][256]) {
    for (int zona = 0; zona < NUM_ZONAS; zona++) {
        if (alertas[zona]) {
            snprintf(recomendaciones[zona], 256,
                "Reducir trafico vehicular, cerrar industrias temporalmente, suspender actividades al aire libre en zona %d.",
                zona + 1);
        } else {
            snprintf(recomendaciones[zona], 256,
                "Niveles de contaminacion aceptables en zona %d. Mantener monitoreo constante.",
                zona + 1);
        }
    }
}
