// Pergamino.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "../systems/CPC6128.h"
#include "../TimingHandler.h"

#include "Controles.h"
#include "Juego.h"
#include "Paleta.h"
#include "Pergamino.h"

using namespace Abadia;

/////////////////////////////////////////////////////////////////////////////
// inicializaci�n y limpieza
/////////////////////////////////////////////////////////////////////////////

Pergamino::Pergamino()
{
	cpc6128 = elJuego->cpc6128;
	roms = elJuego->roms;
}

Pergamino::~Pergamino()
{
}

/////////////////////////////////////////////////////////////////////////////
// dibujo del pergamino
/////////////////////////////////////////////////////////////////////////////

// dibuja el pergamino
void Pergamino::dibuja()
{
	// limpia la memoria de video
	cpc6128->fillMode1Rect(0, 0, 320, 200, 0);

	// limpia los bordes del rect�ngulo que formar� el pergamino
	cpc6128->fillMode1Rect(0, 0, 64, 200, 1);
	cpc6128->fillMode1Rect(192 + 64, 0, 64, 200, 1);
	cpc6128->fillMode1Rect(0, 192, 320, 8, 1);

	// apunta a los datos del borde superior del pergamino
	UINT8* data = &roms[0x788a];

	// rellena la parte superior del pergamino
	dibujaTiraHorizontal(0, data);

	// rellena la parte derecha del pergamino
	data = &roms[0x7a0a];
	dibujaTiraVertical(248, data);

	// rellena la parte izquierda del pergamino
	data = &roms[0x7b8a];
	dibujaTiraVertical(64, data);

	// rellena la parte inferior del pergamino
	data = &roms[0x7d0a];
	dibujaTiraHorizontal(184, data);
}

// dibuja un borde horizontal del pergamino de 8 pixels de alto
void Pergamino::dibujaTiraHorizontal(int y, UINT8 *data)
{
	// recorre todo el ancho del pergamino
	for (int i = 0; i < 192/4; i++){
		// la parte superior ocupa 8 pixels de alto
		for (int j = 0; j < 8; j++){
			for (int k = 0; k < 4; k++){
				cpc6128->setMode1Pixel(64 + 4*i + k, j + y, cpc6128->unpackPixelMode1(*data, k));
			}
			data++;
		}
	}
}

// dibuja el pergamino
void Pergamino::dibujaTiraVertical(int x, UINT8 *data)
{
	// recorre el alto del pergamino
	for (int j = 0; j < 192; j++){
		// lee 8 pixels y los escribe en pantalla
		for (int i = 0; i < 2; i++){
			for (int k = 0; k < 4; k++){
				cpc6128->setMode1Pixel(x + 4*i + k, j, cpc6128->unpackPixelMode1(*data, k));
			}
			data++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// escritura de texto en el pergamino
/////////////////////////////////////////////////////////////////////////////

void Pergamino::muestraTexto(char *texto)
{
	// pone la paleta negra
	elJuego->paleta->setGamePalette(0);

	// dibuja el pergamino
	dibuja();

	// pone la paleta del pergamino
	elJuego->paleta->setGamePalette(1);
	
	// dibuja el texto que se le pasa
	dibujaTexto(texto);
}

void Pergamino::dibujaTexto(char *texto)
{
	// obtiene acceso al temporizador y a las entradas
	TimingHandler *timer = elJuego->timer;

	// posici�n inicial del texto en el pergamino
	int posX = 76;
	int posY = 16;

	// puntero a la tabla de punteros a los gr�ficos de los caracteres
	UINT16* charTable = (UINT16*) &roms[0x680c];

	// repite hasta que se pulse el bot�n 1
	while (true){
		losControles->actualizaEstado();

		// si se puls� el bot�n 1 o espacio, termina
		if (losControles->estaSiendoPulsado(P1_BUTTON1) || losControles->estaSiendoPulsado(KEYBOARD_SPACE)){
			break;
		} else {
			// dependiendo del car�cter leido
			switch (*texto){
				case 0x1a:			// f�n de pergamino
					break;
				case 0x0d:			// salto de l�nea
					posX = 76;
					posY += 16;
					timer->sleep(600);

					// si hay que pasar p�gina del pergamino
					if (posY > 164){
						posX = 76;
						posY = 16;
						timer->sleep(2000);
						pasaPagina();
					}
					break;
				case 0x20:			// espacio
					posX += 10;
					timer->sleep(30);
					break;
				case 0x0a:			// salto de p�gina
					posX = 76;
					posY = 16;
					timer->sleep(3*525);
					pasaPagina();
					break;

				default:			// car�cter imprimible
					// elige un color dependiendo de si es may�sculas o min�sculas
					int color = (((*texto) & 0x60) == 0x40) ? 3 : 2;

					// obtiene el desplazamiento a los datos de formaci�n del car�cter
					int charOffset = charTable[(*texto) - 0x20];

					// si el caracter no est� definido, muestra una 'z'
					if (charTable[(*texto) - 0x20] == 0){
						charOffset = charTable['z' - 0x20];
					}

					// mientras queden trazos del car�cter
					while ((roms[charOffset] & 0xf0) != 0xf0){
						// halla el desplazamiento del trazo
						int newPosX = posX + (roms[charOffset] & 0x0f);
						int newPosy = posY + ((roms[charOffset] >> 4) & 0x0f);

						// dibuja el trazo del car�cter
						cpc6128->setMode1Pixel(newPosX, newPosy, color);

						charOffset++;

						// espera un poco para que se pueda apreciar como se traza el car�cter
						timer->sleep(8);
					}

					// avanza la posici�n hasta el siguiente car�cter
					posX += roms[charOffset] & 0x0f;
			}

			// apunta al siguiente car�cter a imprimir
			if (*texto != 0x1a){
				texto++;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// paso de p�gina del pergamino
/////////////////////////////////////////////////////////////////////////////

// dibuja un tri�ngulo rect�ngulo de color1 con catetos paralelos a los ejes x e y, y limpia los 4 
//  pixels a la derecha de la hipotenusa del tri�ngulo con el color2
void Pergamino::dibujaTriangulo(int x, int y, int lado, int color1, int color2)
{
	lado = lado*4;

	for (int j = 0; j < lado; j++){
		// dibuja el tri�ngulo
		for (int i = 0; i <= j; i++){
			cpc6128->setMode1Pixel(x + i, y + j, color1);
		}

		// elimina restos de una ejecuci�n anterior
		for (int i = 0; i < 4; i++){
			cpc6128->setMode1Pixel(x + j + i + 1, y + j, 0);
		}
	}
}

// restaura un trozo de 8x8 pixels de la parte superior y otro de la parte derecha del pergamino
void Pergamino::restauraParteSuperiorYDerecha(int x, int y, int lado)
{
	x = x + 4;

	// apunta a los datos borrados del borde superior del pergamino
	UINT8* data = &roms[0x788a + (48 - lado)*4*2];

	// 8 pixels de ancho
	for (int i = 0; i < 2; i++){
		// 8 pixels de alto
		for (int j = 0; j < 8; j++){
			for (int k = 0; k < 4; k++){
				cpc6128->setMode1Pixel(x + 4*i + k, y + j, cpc6128->unpackPixelMode1(*data, k));
			}
			data++;
		}
	}

	x = 248;
	y = (lado - 3)*4;

	// apunta a los datos borrados de la parte derecha del pergamino
	data = &roms[0x7a0a + y*2];

	// 8 pixels de alto
	for (int j = 0; j < 8; j++){
		// 8 pixels de ancho
		for (int i = 0; i < 2; i++){
			for (int k = 0; k < 4; k++){
				cpc6128->setMode1Pixel(x + 4*i + k, y + j, cpc6128->unpackPixelMode1(*data, k));
			}
			data++;
		}
	}
}

// restaura un trozo de 4x8 pixels de la parte inferior del pergamino
void Pergamino::restauraParteInferior(int x, int y, int lado)
{
	x = 64 + lado*4;
	y = 184;

	// apunta a los datos borrados del borde inferior del pergamino
	UINT8* data = &roms[0x7d0a + lado*4*2];

	// dibuja un trozo de 4x8 pixels de la parte inferior del pergamino
	for (int j = 0; j < 8; j++){
		for (int k = 0; k < 4; k++){
			cpc6128->setMode1Pixel(x + k, y + j, cpc6128->unpackPixelMode1(*data, k));
		}
		data++;
	}
}

// realiza el efecto de pasar una p�gina del pergamino
void Pergamino::pasaPagina()
{
	// obtiene acceso al temporizador
	TimingHandler *timer = elJuego->timer;

	int x = 240;
	int y = 0;
	int dim = 3;

	// realiza el efecto del paso de p�gina desde la esquina superior derecha hasta la mitad de la p�gina
	for (int num = 0; num < 45; num++){
		dibujaTriangulo(x, y, dim, 1, 0);
		timer->sleep(20);
		restauraParteSuperiorYDerecha(x, y, dim);

		x = x - 4;
		dim++;
	}
	restauraParteSuperiorYDerecha(x, y, dim);

	x = 64;
	y = 4;
	dim = 47;

	// realiza el efecto del paso de p�gina desde la mitad de la p�gina hasta terminar en la esquina inferior izquierda
	for (int num = 0; num < 46; num++){
		dibujaTriangulo(x, y, dim, 1, 0);
		timer->sleep(20);

		y = y - 4;

		// apunta a los datos borrados del borde izquierdo del pergamino
		UINT8* data = &roms[0x7b8a + y*2];

		// dibuja un trozo de 8x4 de la parte izquierda del pergamino
		for (int j = 0; j < 4; j++){
			for (int i = 0; i < 2; i++){
				for (int k = 0; k < 4; k++){
					cpc6128->setMode1Pixel(x + 4*i + k, y + j, cpc6128->unpackPixelMode1(*data, k));
				}
				data++;
			}
		}

		// restaura un trozo de 4x8 pixels de la parte inferior del pergamino
		restauraParteInferior(x, y, dim);

		y = y + 8;
		dim--;
	}
	restauraParteInferior(x, y, 1);
	restauraParteInferior(x, y, 0);
}
