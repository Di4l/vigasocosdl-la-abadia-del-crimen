// GeneradorPantallas.h
//
//	Clase que se encarga de generar los bloques que forman las pantallas, calcular la zona que
//	tapa el bloque y grabar toda esa informaci�n en la capas que forman el buffer de tiles.
//	Adem�s, esta clase se encarga de dibujar las pantallas una vez que se ha rellenado el buffer 
//	de tiles.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _GENERADOR_PANTALLAS_H_
#define _GENERADOR_PANTALLAS_H_


class CPC6128;							// definido en CPC6128.h

namespace Abadia {

class Comando;							// definido en Comando.h


class GeneradorPantallas
{
// constantes
public:
	static const int nivelesProfTiles = 3;

// tipos
public:
	struct TileInfo {
		UINT8 profX[nivelesProfTiles];	// profundidad del tile en x (en coordenadas locales)
		UINT8 profY[nivelesProfTiles];	// profundidad del tile en y (en coordenadas locales)
		UINT8 tile[nivelesProfTiles];	// n�mero de tile
	};

// campos
public:
	UINT8 *roms;						// puntero a los datos del juego
	CPC6128	*cpc6128;					// objeto de ayuda para realizar operaciones gr�ficas del cpc6128

	TileInfo bufferTiles[20][16];		// buffer de tiles (16x20 tiles)
	int mascaras[4][4];					// tablas de m�scaras and y or para cada uno de los colores

	UINT8 *datosPantalla;				// puntero a los datos que forman la pantalla
	int comandosBloque;					// desplazamiento a los datos de los comandos que forman un bloque
	int datosBloque[17];				// buffer donde guarda los datos para construir el bloque actual
	int tilePosX, tilePosY;				// posici�n actual en el buffer de tiles
	bool cambioSistemaCoord;			// indica si se ha cambiado el sistema de coordenadas
	int estadoOpsX[4];					// usado para cambiar el sentido de las x en algunas operaciones

protected:
	int pila[64];						// pila para evaluar los comandos de generaci�n de bloques
	int posPila;						// posici�n actual de la pila

	Comando* manejadores[0x1c];			// tabla con los manejadores para cada comando

// m�todos
public:
	// m�todos para la generaci�n de bloques
	void genera(UINT8 *datosPantalla);
	void iniciaInterpretacionBloque(UINT8 *tilesBloque, bool modificaTiles, int altura);

	// m�todos de dibujado de tiles
	void grabaTile(int tile);
	void actualizaTile(int tile, TileInfo *tileDesc);
	void limpiaPantalla(int color);
	void dibujaBufferTiles();

	// operaciones sobre registros y expresiones del generador de bloques
	int leeDatoORegistro(int *posReg);
	int obtenerRegistro(int reg, int *posReg);
	int actualizaRegistro(int reg, int delta);
	int evaluaExpresion(int rdo);

	// operaciones sobre la pila
	void push(int data);
	int pop();

	// inicializaci�n y limpieza
	GeneradorPantallas();
	~GeneradorPantallas();

	// m�todos de ayuda
	int obtenerDir(int direccion);
protected:
	void transformaCoordBloqueRejilla(int altura);
	void interpretaComandos();

	void generaMascaras();

	void dibujaTile(int x, int y, int num);
	void dibujaTira(int &x, int &y, int deltaX, int deltaY, int veces);
};


}

#endif	// _GENERADOR_PANTALLAS_H_
