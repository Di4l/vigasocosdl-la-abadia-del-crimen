// Paleta.h
//
//	Clase con todos los m�todos relacionados con la paleta
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _PALETA_H_
#define _PALETA_H_


class CPC6128;				// definido en CPC6128.h
class IPalette;				// definido en IPalette.h

namespace Abadia {


class Paleta
{
// campos
protected:
	IPalette *paleta;		// paleta real del juego
	CPC6128	*cpc6128;		// objeto que presta ayuda para realizar operaciones gr�ficas del cpc6128

// m�todos
public:
	void setIntroPalette();
	void setGamePalette(int pal);

	// inicializaci�n y limpieza
	Paleta();
	~Paleta();

// paletas gr�ficas
protected:
	static int introPalette[16];
	static int palettes[4][4];
};


}

#endif	// _PALETA_H_
