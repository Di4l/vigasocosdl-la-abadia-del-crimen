// Pergamino.h
//
//	Clase que representa el pergamino
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _PERGAMINO_H_
#define _PERGAMINO_H_


#include "../Types.h"

class CPC6128;						// definido en CPC6128.h

namespace Abadia {


class Pergamino
{
// campos
public:
	static char *pergaminoInicio;	// texto del pergamino de la presentaci�n del juego
	static char *pergaminoFinal;	// texto del pergamino del final del juego

protected:
	CPC6128	*cpc6128;				// objeto que presta ayuda para realizar operaciones gr�ficas del cpc6128
	UINT8 *roms;

// m�todos
public:
	void muestraTexto(char *texto);

	// inicializaci�n y limpieza
	Pergamino();
	~Pergamino();

// m�todos de ayuda
protected:
	void dibuja();
	void dibujaTexto(char *texto);

	void dibujaTiraHorizontal(int y, UINT8 *data);
	void dibujaTiraVertical(int x, UINT8 *data);

	void dibujaTriangulo(int x, int y, int lado, int color1, int color2);
	void restauraParteSuperiorYDerecha(int x, int y, int lado);
	void restauraParteInferior(int x, int y, int lado);
	void pasaPagina();
};


}

#endif	// _PERGAMINO_H_
