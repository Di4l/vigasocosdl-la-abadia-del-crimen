// Marcador.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "../systems/CPC6128.h"

#include "Abad.h"
#include "Juego.h"
#include "Logica.h"
#include "Marcador.h"
#include "Paleta.h"
#include "Sprite.h"

using namespace Abadia;

/////////////////////////////////////////////////////////////////////////////
// duraci�n de las etapas del d�a
/////////////////////////////////////////////////////////////////////////////

int Marcador::duracionEtapasDia[7][7] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x00 },
	{ 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x00 },
	{ 0x0f, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00 },
	{ 0x0f, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0f, 0x00, 0x05, 0x00, 0x05, 0x00, 0x00 },
	{ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

/////////////////////////////////////////////////////////////////////////////
// inicializaci�n y limpieza
/////////////////////////////////////////////////////////////////////////////

Marcador::Marcador()
{
	cpc6128 = elJuego->cpc6128;
	roms = elJuego->roms;
}

Marcador::~Marcador()
{
}

/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con los d�as y los momentos del d�a
/////////////////////////////////////////////////////////////////////////////

// avanza el momento del d�a del marcador
void Marcador::muestraDiaYMomentoDia()
{
	// coloca una paleta seg�n el momento del d�a
	/* 
	if (laLogica->momentoDia < VISPERAS){
		elJuego->paleta->setGamePalette(2);
	} else {
		elJuego->paleta->setGamePalette(3);
	}
	*/
	if (laLogica->momentoDia == NOCHE ||
		laLogica->momentoDia == COMPLETAS){
		elJuego->paleta->setGamePalette(3);
	} else {
		elJuego->paleta->setGamePalette(2);
	}


	// dibuja el n�mero de d�a en el marcador
	dibujaDia(laLogica->dia);

	// hace que avance el momento del d�a, para mostrar el efecto de scroll en las letras del d�a
	laLogica->momentoDia = laLogica->momentoDia - 1;
	avanzaMomentoDia();
}

// avanza el momento del d�a
void Marcador::avanzaMomentoDia()
{
	laLogica->momentoDia = laLogica->momentoDia + 1;

	// si se han terminado los momentos del d�a, avanza al siguiente d�a
	if (laLogica->momentoDia > COMPLETAS){
		laLogica->momentoDia = 0;
		laLogica->dia = laLogica->dia + 1;

		// si se ha terminado el s�ptimo d�a, vuelve al primer d�a
		if (laLogica->dia > 7){
			laLogica->dia = 1;
		}

		// dibuja el nuevo d�a en el marcador
		dibujaDia(laLogica->dia);
	}

	// obtiene un puntero a los caracteres que forman el momento del d�a
	nombreMomentoDia = &roms[0x4fbc + 7*laLogica->momentoDia];

	// quedan 9 caracteres para completar el scroll del nombre del d�a
	numPosScrollDia = 9;

	// obtiene la duraci�n de esta etapa del d�a
	laLogica->duracionMomentoDia = duracionEtapasDia[laLogica->dia - 1][laLogica->momentoDia]*0x100;
}

// dibuja el d�a en el marcador
void Marcador::dibujaDia(int numDia)
{
	// indexa en la tabla de los d�as
	UINT8 *data = &roms[0x4fa7 + (numDia - 1)*3];

	// dibuja los 3 n�meros romanos que forman el d�a en el que se est�
	dibujaDigitoDia(data[0], 68, 165);
	dibujaDigitoDia(data[1], 68 + 8, 165);
	dibujaDigitoDia(data[2], 68 + 16, 165);
}

// dibuja un n�mero romano que forma el d�a en la posici�n que se le pasa
void Marcador::dibujaDigitoDia(int digito, int x, int y)
{
	// apunta a 8 pixels negros
	int despDigito = 0x5581;

	// si se le pas� una 'I'
	if (digito == 2){
		despDigito = 0xab49;
	} else if (digito == 1){
		// si se le pas� una 'V'
		despDigito = 0xab39;
	}

	// obtiene un puntero a los gr�ficos del d�gito
	UINT8 *data = &roms[despDigito];

	// rellena las 8 l�neas que ocupa la letra
	for (int j = 0; j < 8; j++){
		// cada d�gito tiene 8 pixels de ancho
		for (int i = 0; i < 2; i++){
			for (int k = 0; k < 4; k++){
				cpc6128->setMode1Pixel(x + 4*i + k, y + j, cpc6128->unpackPixelMode1(*data, k));
			}
			data++;
		}

		// si no hab�a que mostrar ning�n d�gito, mantiene el puntero en los pixels negros
		if (digito == 0){
			data = data - 2;
		}
	}
}

// realiza el efecto de scroll en la parte del marcador que muestra el momento del d�a
void Marcador::realizaScrollMomentoDia()
{
	// si todav�a quedan posiciones para desplazar
	if (numPosScrollDia != 0){
		numPosScrollDia--;

		int caracter = 0x20;

		// en las 2 primeras posiciones del scroll, se ponen caracteres de espacio
		if (numPosScrollDia < 7) {
			caracter = *nombreMomentoDia;
			nombreMomentoDia++;
		}

		// 8 l�neas de alto
		for (int j = 0; j < 8; j++){
			// desplaza 48/8 = 6 caracteres a la izquierda 1 caracter (cada caracter es de 8x8)
			for (int i = 0; i < 48; i++){
				cpc6128->setMode1Pixel(44 + i - 8, 180 + j, cpc6128->getMode1Pixel(44 + i, 180 + j));
			}
		}
		
		// imprime el caracter que toca
		imprimirCaracter(caracter, 84, 180, 3, 2);
	}
}
/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con el obsequium
/////////////////////////////////////////////////////////////////////////////

// decrementa la barra de obsequium
void Marcador::decrementaObsequium(int unidades)
{
	laLogica->obsequium = laLogica->obsequium - unidades;

	// si se ha terminado el obsequium
	if (laLogica->obsequium < 0){
		// si guillermo no ha muerto, cambia el estado del abad para que le eche de la abad�a
		if (!laLogica->haFracasado){
			laLogica->abad->estado = 0x0b;
		}

		laLogica->obsequium = 0;
	}

	// dibuja la parte de la barra correspondiente a la vida que tenemos
	dibujaBarra(laLogica->obsequium, 2, 240, 177);

	// dibuja la parte de la barra correspondiente a la vida que no tenemos
	dibujaBarra(31 - laLogica->obsequium + 1, 3, 240 + laLogica->obsequium, 177);
}

// dibuja una barra de la longitud y color especificados
void Marcador::dibujaBarra(int lgtud, int color, int x, int y)
{
	if (lgtud != 0){
		cpc6128->fillMode1Rect(x, y, lgtud, 6, color);
	}
}

/////////////////////////////////////////////////////////////////////////////
// dibujo del marcador
/////////////////////////////////////////////////////////////////////////////

// limpia el �rea que ocupa el marcador
void Marcador::limpiaAreaMarcador()
{
	cpc6128->fillMode1Rect(0, 160, 320, 40, 3);
}

// dibuja el marcador
void Marcador::dibujaMarcador()
{
	// apunta a los datos gr�ficos del marcador
	UINT8 *data = &roms[0x1e328];

	// dibuja las 32 l�neas que forman el marcador en la parte inferior de la pantalla
	for (int j = 0; j < 32; j++){
		for (int i = 0; i < 256/4; i++){
			for (int k = 0; k < 4; k++){
				cpc6128->setMode1Pixel(32 + 4*i + k, 160 + j, cpc6128->unpackPixelMode1(*data, k));
			}
			data++;
		}
	}
}

// dibuja los objetos que tenemos en el marcador
void Marcador::dibujaObjetos(int objetos, int mascara)
{
	int posX = 100;
	int posY = 176;
	Sprite **sprites = elJuego->sprites;

	// recorre los 6 huecos posibles
	for (int numHuecos = 0; numHuecos < 6; numHuecos++){
		// si se han procesado todos los objetos que hab�a que actualizar, sale
		if (mascara == 0){
			return;
		}

		// averigua si hay que comprobar el objeto actual
		if ((mascara & (1 << (Juego::numObjetos - 1))) != 0){
			// si tenemos el objeto, lo dibuja
			if ((objetos & (1 << (Juego::numObjetos - 1))) != 0){
				Sprite *spr = sprites[Juego::primerSpriteObjetos + numHuecos];

				// obtiene un puntero a los gr�ficos del objeto
				UINT8 *data = &roms[spr->despGfx];

				// dibuja el objeto
				for (int j = 0; j < spr->alto; j++){
					for (int i = 0; i < spr->ancho; i++){
						for (int k = 0; k < 4; k++){
							cpc6128->setMode1Pixel(posX + 4*i + k, posY + j, cpc6128->unpackPixelMode1(*data, k));
						}
						data++;
					}
				}
			} else {
				// en otro caso, limpia el hueco (12x16 pixels)
				for (int j = 0; j < 12; j++){
					for (int i = 0; i < 16; i++){
						cpc6128->setMode1Pixel(posX + i, posY + j, 0);
					}
				}
			}
		}

		// pasa al siguiente objeto
		mascara = mascara << 1;
		objetos = objetos << 1;

		// avanza la posici�n al siguiente hueco
		posX += 20;

		// al pasar del tercer al cuarto hueco, hay 4 pixels extra
		if (numHuecos == 2){
			posX += 4;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// dibujo de frases
/////////////////////////////////////////////////////////////////////////////

// limpia la zona del marcador en donde se muestran las frases
void Marcador::limpiaAreaFrases()
{
	cpc6128->fillMode1Rect(96, 164, 128, 8, 3);
}

// recorre los caracteres de la frase, mostr�ndolos por pantalla
void Marcador::imprimeFrase(std::string frase, int x, int y, int colorTexto, int colorFondo)
{
	for (unsigned int i = 0; i < frase.length(); i++){
		imprimirCaracter(frase[i], x + 8*i, y, colorTexto, colorFondo);
	}
}

void Marcador::imprimirCaracter(int caracter, int x, int y, int colorTexto, int colorFondo)
{
	// se asegura de que el caracter est� entre 0 y 127
	caracter &= 0x7f;

	// si es un caracter no imprimible, sale
	if ((caracter != 0x20) && (caracter < 0x2d)){
		return;
	}

	// inicialmente se apunta a los datos del espacio en blanco
	UINT8 *data = &roms[0x38e7];

	// si el caracter no es un espacio en blanco, modifica el puntero a los datos del caracter
	if (caracter != 0x20){
		data = &roms[0xb400 + 8*(caracter - 0x2d)];
	}

	// cada caracter es de 8x8 pixels
	for (int j = 0; j < 8; j++){
		int bit = 0x80;
		int valor = *data;

		for (int i = 0; i < 8; i++){
			cpc6128->setMode1Pixel(x + i, y + j, (valor & bit) ? colorTexto : colorFondo);
			bit = bit >> 1;
		}
		data++;
	}
}
