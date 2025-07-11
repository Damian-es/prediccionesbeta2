#ifndef FUNCIONES_H
#define FUNCIONES_H

#define NUM_ZONAS 5
#define NUM_DIAS 30
#define CONTAMINANTES 4 // CO2, SO2, NO2, PM2_5

// Limites de contaminacion (valores de ejemplo, unidades ajustables)
#define LIMITE_CO2 1000.0
#define LIMITE_SO2 20.0
#define LIMITE_NO2 40.0
#define LIMITE_PM25 25.0

// Estructura para datos de contaminacion de una zona en un dia
typedef struct {
    float co2;
    float so2;
    float no2;
    float pm25;
} DatosContaminacion;

// Estructura para datos de contaminacion de una zona en varios dias
typedef struct {
    DatosContaminacion datosDiarios[NUM_DIAS];
} DatosZona;

// Estructura para datos climaticos actuales
typedef struct {
    float temperatura;
    float velocidadViento;
    float humedad;
} DatosClimaticos;

// Estructura para resultados de prediccion de una zona
typedef struct {
    DatosContaminacion nivelesPredichos;
} Prediccion;

// Prototipos de funciones

// Cargar datos historicos de contaminacion desde archivo TXT individual por zona
int cargarDatosHistoricos(DatosZona* zona, const char* nombreArchivo);

// Cargar datos climaticos desde archivo TXT (un solo archivo para todas las zonas)
int cargarDatosClima(DatosClimaticos clima[NUM_ZONAS], const char* nombreArchivo);

// Guardar datos de contaminacion y predicciones en archivo de reporte
int guardarReporte(DatosZona zonas[NUM_ZONAS], Prediccion predicciones[NUM_ZONAS], const char* nombreArchivo);

// Calcular niveles actuales de contaminacion (ultimo dia) y comparar con limites
void calcularContaminacionActual(DatosZona zonas[NUM_ZONAS], int diaActual, int alertas[NUM_ZONAS]);

// Predecir niveles futuros de contaminacion para las proximas 24 horas usando promedio ponderado
void predecirContaminacionFutura(DatosZona zonas[NUM_ZONAS], DatosClimaticos* clima, Prediccion predicciones[NUM_ZONAS]);

// Emitir alertas si los niveles predichos exceden los limites
void emitirAlertas(Prediccion predicciones[NUM_ZONAS], int alertas[NUM_ZONAS]);

// Calcular promedios historicos de 30 dias y comparar con limites OMS
void calcularPromediosHistoricos(DatosZona zonas[NUM_ZONAS], DatosContaminacion promedios[NUM_ZONAS], int alertas[NUM_ZONAS]);

// Generar recomendaciones de mitigacion basadas en niveles de contaminacion
void generarRecomendaciones(int alertas[NUM_ZONAS], char recomendaciones[NUM_ZONAS][256]);

#endif // FUNCIONES_H
