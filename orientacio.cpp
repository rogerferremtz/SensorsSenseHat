// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/**
 * Program @file RTIMULibDrive10.cpp
 * Version   1.3
 *
 * @brief Programa de posicionament i resposta luminica a canvi de posició del dispositiu
 *
 * @author Roger Ferré Martínez
 * @author Xorxe Oural Martínez
 *
 * Copyright (C) 2020
 *
 * License GNU/GPL, see COPYING
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 */
/*
Makefile:

all: orientacio.cpp
	g++ -lRTIMULib -lsqlite3 orientacio.cpp   -o orientacio

clear:
	rm orientacio

 */
#include "RTIMULib.h"
#include "RTFusionRTQF.h"
#include "RTIMUSettings.h"
#include "sqlite3.h"
#include "stdint.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "signal.h"
#include "sys/time.h"

#include "func.h"


//Prototipus de les funcions.
int sensor(int, int, int);
int cridarsql(float eje_x, float eje_y, float eje_z, int id_X, int id_Y, int id_Z);
typedef void (timer_callback) (union sigval);

// -----------------------------------------------------------------------------------------------

/*
 * Aquest codi serveix per mesurar i enregistrar els valors de l'orientació de la placa SenseHat.
 * L'orientació, en estar mesurada en els tres eixos, tindrem en compte 3 sensors, en l'eix X,
 * en l'eix Y i en l'eix Z.
 */

void callback(union sigval si)
{
	int * msg = (int *) si.sival_ptr;
	int id_X = msg[0];
	int id_Y = msg[1];
	int id_Z = msg[2];

	//printf("\n\nID: X = %d, Y = %d, Z = %d\n\n", id_X, id_Y, id_Z);

	sensor(id_X, id_Y, id_Z);

    //printf("%s\n",msg);
}



//Aquest Callback revisa que el sensor d'orientació en l'eix X estigui registrat.
static int callback_X(void *punter, int argc, char **argv, char **azColName)
{
	int i, id = -1;

	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		id = atoi(argv[i]);
	}
	if (id == 0)
		return 1;
	else
		printf("Sensor d'orientació en l'eix X REGISTRAT\n");

	return 0;
}



//Aquest Callback revisa que el sensor d'orientació en l'eix Y estigui registrat.
static int callback_Y(void *punter, int argc, char **argv, char **azColName)
{
	int i, id = -1;

	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		id = atoi(argv[i]);
	}
	if (id == 0)
		return 1;
	else
		printf("Sensor d'orientació en l'eix Y REGISTRAT\n");

	return 0;
}



//Aquest Callback revisa que el sensor d'orientació en l'eix Z estigui registrat.
static int callback_Z(void *punter, int argc, char **argv, char **azColName)
{
	int i, id = -1;

	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		id = atoi(argv[i]);
	}
	if (id == 0)
		return 1;
	else
		printf("Sensor d'orientació en l'eix Z REGISTRAT\n");

	return 0;
}



//Aquest Callback ens retorna el valor del ID del sensor que li haguem demanat.
static int callback_id(void *punter, int argc, char **argv, char **azColName)
{
	int i, id = -1;
	int *punterint = (int *)punter;

	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		id = atoi(argv[i]);
	}
	*punterint = id;
	return 0;
}



int set_timer(timer_t * timer_id, float delay, float interval, timer_callback * func, int * data)
{
    int status =0;
    struct itimerspec ts;
    struct sigevent se;

    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = data;
    se.sigev_notify_function = func;
    se.sigev_notify_attributes = NULL;

    status = timer_create(CLOCK_REALTIME, &se, timer_id);

    ts.it_value.tv_sec = abs(delay);
    ts.it_value.tv_nsec = (delay-abs(delay)) * 1e09;
    ts.it_interval.tv_sec = abs(interval);
    ts.it_interval.tv_nsec = (interval-abs(interval)) * 1e09;

    status = timer_settime(*timer_id, 0, &ts, 0);
    return 0;
}



// -----------------------------------------------------------------------------------------------



//Funció encarregada d'introduïr els valors dels sensors a la base de dades. Per fer-ho, abans
//cal saber els valors que entrem a la base de dades i els valors dels ID de cada sensor.
int cridarsql(float eje_X, float eje_Y, float eje_Z, int id_X, int id_Y, int id_Z)
{


	sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

	rc = sqlite3_open("/home/pi/Desktop/GIT/SensorsSenseHat/basedades_adstr.db", &db);

	if (rc != SQLITE_OK) {
		  fprintf(stderr, "Cannot open database.\n");
		  return 1;
	}



	char cadena_URI[1024];
	//char nom_servidor[32] = "84.88.55.9";
	char nom_servidor[32] = "iotlab.euss.cat";

	int hores, minuts, segons, dia, mes, any;

	time_t ara;

	//Obte l'hora actual

	time(&ara);

	struct tm *local = localtime(&ara);

	hores = local->tm_hour;
	minuts = local->tm_min;
	segons = local->tm_sec;

	dia = local->tm_mday;
	mes = local->tm_mon + 1;
	any = local->tm_year + 1900;


	if (hores < 12) //Ante Meridiem.
		printf("Ara son les : %02d:%02d:%02d am\n", hores, minuts, segons);

	else  //Post Meridiem.
		printf("Ara son les : %02d:%02d:%02d pm\n", hores - 12, minuts, segons);

	//Mostra la data.
	//printf("Avui estem a : %02d/%02d/%d\n", dia, mes, any);








	char sql_insertar_X[1024];

	sprintf(sql_insertar_X, "INSERT INTO mesures (id_sensor, valor) VALUES (%d, %f);", id_X, eje_X);

	printf("SQLITE3: %s\n", sql_insertar_X);

	rc = sqlite3_exec(db, sql_insertar_X, 0, 0, &zErrMsg);





	sprintf(cadena_URI, "/cloud/guardar_dades.php?id_sensor=1&valor=%f&temps=%02d-%02d-%02d+%02d%%3A%02d%%3A%02d", eje_X * 1000, any, mes, dia, hores, minuts, segons);

	http_get(nom_servidor, cadena_URI);






	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return 1;
	}









	char sql_insertar_Y[1024];

	sprintf(sql_insertar_Y, "INSERT INTO mesures (id_sensor, valor) VALUES (%d, %f);", id_Y, eje_Y);

	printf("SQLITE3: %s\n", sql_insertar_Y);

	rc = sqlite3_exec(db, sql_insertar_Y, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return 1;
	}




	sprintf(cadena_URI, "/cloud/guardar_dades.php?id_sensor=1&valor=%f&temps=%02d-%02d-%02d+%02d%%3A%02d%%3A%02d", eje_Y * 1000, any, mes, dia, hores, minuts, segons);

	http_get(nom_servidor, cadena_URI);






	char sql_insertar_Z[1024];

	sprintf(sql_insertar_Z, "INSERT INTO mesures (id_sensor, valor) VALUES (%d, %f);", id_Z, eje_Z);

	printf("SQLITE3: %s\n\n", sql_insertar_Z);

	rc = sqlite3_exec(db, sql_insertar_Z, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return 1;
	}




	sprintf(cadena_URI, "/cloud/guardar_dades.php?id_sensor=1&valor=%f&temps=%02d-%02d-%02d+%02d%%3A%02d%%3A%02d", eje_Z * 1000, any, mes, dia, hores, minuts, segons);

	http_get(nom_servidor, cadena_URI);



		sqlite3_close(db);
		return 0;
}



//Funció que llegeix el valor de l'orientació en els tres eixos i crida a la funció que els entrarà a la base de dades.
int sensor(int id_X, int id_Y, int id_Z)
{
    int sampleCount = 0;
    int sampleRate = 0;
    uint64_t rateTimer;
    uint64_t displayTimer;
    uint64_t now;

	// Fa servir l'arxiu .ini amb les preferències establertes en ell



    RTIMUSettings *settings = new RTIMUSettings("RTIMULib");  // using RTIMULib here allows it to use the .ini file generated by RTIMULibDemo.

    RTIMU *imu = RTIMU::createIMU(settings);
    //RTPressure *pressure = RTPressure::createPressure(settings);

    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU found\n");
        exit(1);
    }

    // set up IMU

    imu->IMUInit();


    imu->setGyroEnable(true);



	// Bucle de processament de les dades



	usleep(imu->IMUGetPollInterval() * 1000);

	if (imu->IMURead()) {
		RTIMU_DATA imuData = imu->getIMUData();
		RTVector3& vec = imuData.fusionPose;
		// Treiem per pantalla la inclinació a cada segon.

		float eje_x = vec.x() * RTMATH_RAD_TO_DEGREE;	//Obtenim valor en l'eix X.
		float eje_y = vec.y() * RTMATH_RAD_TO_DEGREE;	//Obtenim valor en l'eix Y.
		float eje_z = vec.z() * RTMATH_RAD_TO_DEGREE;	//Obtenim valor en l'eix Z.

		printf("Inclinació en els eixos. X:%f, Y:%f, Z:%f\n", eje_x, eje_y, eje_z);

		cridarsql(eje_x, eje_y, eje_z, id_X, id_Y, id_Z);	//Cridem la funció que els entra a la base de dades.
	}
}



// -----------------------------------------------------------------------------------------------



//Funció main, la primera part d'aquesta s'encarrega de revisar que els sensors estiguin registrats a la
//base de dades, en cas que no ho siguin, els registra. Un cop registrats demana el seu ID. La segona part
//crida la funció que llegeix els valors de la SenseHat.
int main()
{
	int ret = 0, value_int;
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	int id_X, id_Y, id_Z;

	rc = sqlite3_open("/home/pi/Desktop/GIT/SensorsSenseHat/basedades_adstr.db", &db);

	if (rc != SQLITE_OK) {
		  fprintf(stderr, "Cannot open database.\n");
		  return 1;
	}

	char checksensor_orient_X[1024] = "SELECT EXISTS (SELECT id_sensor FROM sensors WHERE nom_sensor = 'Sensor_Orientacio_X');";
	char checksensor_orient_Y[1024] = "SELECT EXISTS (SELECT id_sensor FROM sensors WHERE nom_sensor = 'Sensor_Orientacio_Y');";
	char checksensor_orient_Z[1024] = "SELECT EXISTS (SELECT id_sensor FROM sensors WHERE nom_sensor = 'Sensor_Orientacio_Z');";


	char demanar_id_X[1024] = "SELECT id_sensor FROM sensors WHERE nom_sensor = 'Sensor_Orientacio_X';";
	char demanar_id_Y[1024] = "SELECT id_sensor FROM sensors WHERE nom_sensor = 'Sensor_Orientacio_Y';";
	char demanar_id_Z[1024] = "SELECT id_sensor FROM sensors WHERE nom_sensor = 'Sensor_Orientacio_Z';";


	rc = sqlite3_exec(db, checksensor_orient_X, callback_X, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		printf("El sensor anomenat 'Sensor_Orientacio_X' no està registrat, procedim a enregistrar-lo.\n");
		char entrar_sensor_X[1024] = "INSERT INTO sensors (nom_sensor, descripcio) VALUES('Sensor_Orientacio_X','Sensor que mesura orientacio en X');";

		rc = sqlite3_exec(db, entrar_sensor_X, 0, 0, &zErrMsg);
	}
		rc = sqlite3_exec(db, demanar_id_X, callback_id, &id_X, &zErrMsg);
		printf("Id_X es: %d\n\n", id_X);





	rc = sqlite3_exec(db, checksensor_orient_Y, callback_Y, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		printf("El sensor anomenat 'Sensor_Orientacio_Y' no està registrat, procedim a enregistrar-lo.\n");
		char entrar_sensor_Y[1024] = "INSERT INTO sensors (nom_sensor, descripcio) VALUES('Sensor_Orientacio_Y','Sensor que mesura orientacio en Y');";

		rc = sqlite3_exec(db, entrar_sensor_Y, 0, 0, &zErrMsg);
	}
		rc = sqlite3_exec(db, demanar_id_Y, callback_id, &id_Y, &zErrMsg);
		printf("Id_Y es: %d\n\n", id_Y);




	rc = sqlite3_exec(db, checksensor_orient_Z, callback_Z, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		printf("El sensor anomenat 'Sensor_Orientacio_Z' no està registrat, procedim a enregistrar-lo.\n");
		char entrar_sensor_Z[1024] = "INSERT INTO sensors (nom_sensor, descripcio) VALUES('Sensor_Orientacio_Z','Sensor que mesura orientacio en Z');";

		rc = sqlite3_exec(db, entrar_sensor_Z, 0, 0, &zErrMsg);
	}
		rc = sqlite3_exec(db, demanar_id_Z, callback_id, &id_Z, &zErrMsg);
		printf("Id_Z es: %d\n\n", id_Z);



	int ids[3];
	int *pointer = ids;
	ids[0] = id_X;
	ids[1] = id_Y;
	ids[2] = id_Z;


	timer_t tick;
	set_timer(&tick, 2, 5, callback, (int *) pointer);
	getchar();


	return ret;
}
