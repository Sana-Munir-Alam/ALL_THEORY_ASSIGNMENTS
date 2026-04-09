#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Weather {
    float temperature;
    float rainfall;
    float windSpeed;
};
struct Crop {
    char cropType[50];
    char growthStage[50];
    float expectedYield;
    struct Weather *weather;
};
struct Equipment {
    char equipmentName[50];
    char operationalStatus[20];
    float fuelLevel;
    char activitySchedule[100];
};
struct Sensor {
    char sensorType[50];
    float soilNutrients;
    float pHLevel;
    int pestActivity;
};
struct Field {
    char gpsCoordinates[100];
    float soilHealth;
    float moistureLevel;
    struct Crop **crops;
    struct Equipment **equipment;
    struct Sensor **sensors;
    int numCrops;
    int numEquipment;
    int numSensors;
};
struct RegionalHub {
    char hubName[50];
    struct Field **fields;
    int numFields;
    float yieldPrediction;
};
struct AnalyticalServer {
    float regionalYieldPrediction;
    float overallSoilHealth;
    float equipmentStatusAnalysis;
};

// function to simply use fget and strcspn
void getInput(char *buffer, int size) {
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
}

void addCrop(struct Field *field) {
    int numCrops;
    printf("Enter number of crops: ");
    scanf("%d", &numCrops);
    getchar();

    for (int i = 0; i < numCrops; i++) {
        field->crops[field->numCrops] = (struct Crop *)malloc(sizeof(struct Crop));

        printf("Enter crop type: ");
        getInput(field->crops[field->numCrops]->cropType, sizeof(field->crops[field->numCrops]->cropType));

        printf("Enter growth stage: ");
        getInput(field->crops[field->numCrops]->growthStage, sizeof(field->crops[field->numCrops]->growthStage));

        printf("Enter expected yield: ");
        scanf("%f", &field->crops[field->numCrops]->expectedYield);

        field->crops[field->numCrops]->weather = (struct Weather *)malloc(sizeof(struct Weather));
        printf("Enter temperature: ");
        scanf("%f", &field->crops[field->numCrops]->weather->temperature);
        printf("Enter rainfall: ");
        scanf("%f", &field->crops[field->numCrops]->weather->rainfall);
        printf("Enter wind speed: ");
        scanf("%f", &field->crops[field->numCrops]->weather->windSpeed);
        getchar();

        field->numCrops++;
    }
}

void addEquipment(struct Field *field) {
    int numEquipment;
    printf("Enter number of equipment: ");
    scanf("%d", &numEquipment);
    getchar();

    for (int i = 0; i < numEquipment; i++) {
        field->equipment[field->numEquipment] = (struct Equipment *)malloc(sizeof(struct Equipment));

        printf("Enter equipment name: ");
        getInput(field->equipment[field->numEquipment]->equipmentName, sizeof(field->equipment[field->numEquipment]->equipmentName));

        printf("Enter operational status: ");
        getInput(field->equipment[field->numEquipment]->operationalStatus, sizeof(field->equipment[field->numEquipment]->operationalStatus));

        printf("Enter fuel level: ");
        scanf("%f", &field->equipment[field->numEquipment]->fuelLevel);
        getchar();

        printf("Enter activity schedule: ");
        getInput(field->equipment[field->numEquipment]->activitySchedule, sizeof(field->equipment[field->numEquipment]->activitySchedule));

        field->numEquipment++;
    }
}

void addSensor(struct Field *field) {
    int numSensors;
    printf("Enter number of sensors: ");
    scanf("%d", &numSensors);
    getchar();

    for (int i = 0; i < numSensors; i++) {
        field->sensors[field->numSensors] = (struct Sensor *)malloc(sizeof(struct Sensor));

        printf("Enter sensor type: ");
        getInput(field->sensors[field->numSensors]->sensorType, sizeof(field->sensors[field->numSensors]->sensorType));

        printf("Enter soil nutrients level: ");
        scanf("%f", &field->sensors[field->numSensors]->soilNutrients);

        printf("Enter pH level: ");
        scanf("%f", &field->sensors[field->numSensors]->pHLevel);

        printf("Enter pest activity: ");
        scanf("%d", &field->sensors[field->numSensors]->pestActivity);
        getchar();

        field->numSensors++;
    }
}

void displayField(struct Field *field) {
    printf("GPS Coordinates: %s\n", field->gpsCoordinates);
    printf("Soil Health: %.2f\n", field->soilHealth);
    printf("Moisture Level: %.2f\n", field->moistureLevel);

    for (int i = 0; i < field->numCrops; i++) {
        printf("Crop %d: %s, Growth Stage: %s, Expected Yield: %.2f\n",
               i + 1, field->crops[i]->cropType, field->crops[i]->growthStage, field->crops[i]->expectedYield);
    }

    for (int i = 0; i < field->numEquipment; i++) {
        printf("Equipment %d: %s, Status: %s, Fuel: %.2f\n",
               i + 1, field->equipment[i]->equipmentName, field->equipment[i]->operationalStatus, field->equipment[i]->fuelLevel);
    }

    for (int i = 0; i < field->numSensors; i++) {
        printf("Sensor %d: %s, Soil Nutrients: %.2f, pH: %.2f, Pest Activity: %d\n",
               i + 1, field->sensors[i]->sensorType, field->sensors[i]->soilNutrients, field->sensors[i]->pHLevel, field->sensors[i]->pestActivity);
    }
}

void freeField(struct Field *field) {
    for (int i = 0; i < field->numCrops; i++) {
        free(field->crops[i]->weather);
        free(field->crops[i]);
    }
    for (int i = 0; i < field->numEquipment; i++) {
        free(field->equipment[i]);
    }
    for (int i = 0; i < field->numSensors; i++) {
        free(field->sensors[i]);
    }
    free(field->crops);
    free(field->equipment);
    free(field->sensors);
    free(field);
}

void analyzeFieldData(struct AnalyticalServer *server, struct RegionalHub *hub) {
    float totalYield = 0;
    float totalSoilHealth = 0;
    float equipmentStatus = 0;
    int totalEquipment = 0;

    for (int i = 0; i < hub->numFields; i++) {
        for (int j = 0; j < hub->fields[i]->numCrops; j++) {
            totalYield += hub->fields[i]->crops[j]->expectedYield;
        }
        totalSoilHealth += hub->fields[i]->soilHealth;

        for (int j = 0; j < hub->fields[i]->numEquipment; j++) {
            if (strcmp(hub->fields[i]->equipment[j]->operationalStatus, "Operational") == 0) {
                equipmentStatus += 1;
            }
            totalEquipment++;
        }
    }

    server->regionalYieldPrediction = totalYield;
    server->overallSoilHealth = totalSoilHealth / hub->numFields;
    server->equipmentStatusAnalysis = equipmentStatus / totalEquipment * 100;
}

struct Field *createField() {
    struct Field *field = (struct Field *)malloc(sizeof(struct Field));
    printf("Enter GPS coordinates: ");
    getInput(field->gpsCoordinates, sizeof(field->gpsCoordinates));

    printf("Enter soil health: ");
    scanf("%f", &field->soilHealth);

    printf("Enter moisture level: ");
    scanf("%f", &field->moistureLevel);
    getchar();

    field->numCrops = 0;
    field->numEquipment = 0;
    field->numSensors = 0;

    field->crops = (struct Crop **)malloc(10 * sizeof(struct Crop *));
    field->equipment = (struct Equipment **)malloc(10 * sizeof(struct Equipment *));
    field->sensors = (struct Sensor **)malloc(10 * sizeof(struct Sensor *));

    addCrop(field);
    addEquipment(field);
    addSensor(field);

    return field;
}

int main() {
    int numFields, i;
    printf("Enter the number of fields in the hub: ");
    scanf("%d", &numFields);
    getchar();

    struct RegionalHub hub;
    strcpy(hub.hubName, "North Region Hub");
    hub.fields = (struct Field **)malloc(numFields * sizeof(struct Field *));
    hub.numFields = numFields;

    for (i = 0; i < numFields; i++) {
        printf("\nCreating Field %d:\n", i + 1);
        hub.fields[i] = createField();
    }

    struct AnalyticalServer server;
    analyzeFieldData(&server, &hub);

    printf("\nDisplaying All Fields:\n");
    for (i = 0; i < numFields; i++) {
        printf("\nField %d:\n", i + 1);
        displayField(hub.fields[i]);
    }

    printf("\nAnalytical Server Results:\n");
    printf("Regional Yield Prediction: %.2f\n", server.regionalYieldPrediction);
    printf("Overall Soil Health: %.2f\n", server.overallSoilHealth);
    printf("Equipment Status (Operational): %.2f%%\n", server.equipmentStatusAnalysis);

    for (i = 0; i < numFields; i++) {
        freeField(hub.fields[i]);
    }
    free(hub.fields);

    return 0;
}
