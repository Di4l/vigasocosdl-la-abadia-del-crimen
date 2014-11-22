// GestorFrases.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "../systems/CPC6128.h"
#include "Juego.h"
#include "GestorFrases.h"
#include "Marcador.h"

using namespace Abadia;

/////////////////////////////////////////////////////////////////////////////
// tabla de frases
/////////////////////////////////////////////////////////////////////////////

//char * GestorFrases::frases[0x38] = {
//666, prueba, prueba
char * GestorFrases::frases[0x38+1] = {
	"SECRETUM FINISH AFRICAE, MANUS SUPRA XXX AGE PRIMUM ET SEPTIMUM DE QUATOR",
	"BIENVENIDO A ESTA ABADIA, HERMANO. OS RUEGO QUE ME SIGAIS. HA SUCEDIDO ALGO TERRIBLE",
	"TEMO QUE UNO DE LOS MONJES HA COMETIDO UN CRIMEN. OS RUEGO QUE LO ENCONTREIS ANTES DE QUE LLEGUE BERNARDO GUI, PUES NO DESEO QUE SE MANCHE EL NOMBRE DE ESTA ABADIA",
	"DEBEIS RESPETAR MIS ORDENES Y LAS DE LA ABADIA. ASISTIR A LOS OFICIOS Y A LA COMIDA. DE NOCHE DEBEIS ESTAR EN VUESTRA CELDA",
	"DEJAD EL MANUSCRITO DE VENACIO O ADVERTIRE AL ABAD",
	"DADME EL MANUSCRITO, FRAY GUILLERMO",
	"LLEGAIS TARDE, FRAY GUILLERMO",
	"ESTA ES VUESTRA CELDA, DEBO IRME",
	"OS ORDENO QUE VENGAIS",
	"DEBEIS ABANDONAR EL EDIFICIO, HERMANO",
	"ADVERTIRE AL ABAD",
	"DEBEMOS IR A LA IGLESIA, MAESTRO",
	"DEBEMOS IR AL REFECTORIO, MAESTRO",
	"PODEIS IR A VUESTRAS CELDAS",
	"NO HABEIS RESPETADO MIS ORDENES. ABANDONAD PARA SIEMPRE ESTA ABADIA",
	"ESCUCHAD HERMANO, HE ENCONTRADO UN EXTRA�O LIBRO EN MI CELDA",
	"ENTRAD EN VUESTRA CELDA, FRAY GUILLERMO",
	"HA LLEGADO BERNARDO, DEBEIS ABANDONAR LA INVESTIGACION",
	"�DORMIMOS?, MAESTRO",
	"DEBEMOS ENCONTRAR UNA LAMPARA, MAESTRO",
	"VENID AQUI, FRAY GUILLERMO",
	"HERMANOS, VENACIO HA SIDO ASESINADO",
	"DEBEIS SABER QUE LA BIBLIOTECA ES UN LUGAR SECRETO. SOLO MALAQUIAS PUEDE ENTRAR. PODEIS IROS",
	"OREMOS",
	"HERMANOS, BERENGARIO HA DESAPARECIDO. TEMO QUE SE HAYA COMETIDO OTRO CRIMEN",
	"PODEIS COMER, HERMANOS",
	"HERMANOS, HAN ENCONTRADO A BERENGARIO ASESINADO",
	"VENID, FRAY GUILLERMO, DEBEMOS ENCONTRAR A SEVERINO",
	"DIOS SANTO... HAN ASESINADO A SEVERINO Y LE HAN ENCERRADO",
	"BERNARDO ABANDONARA HOY LA ABADIA",
	"MA�ANA ABANDONAREIS LA ABADIA",
	"ERA VERDAD, TENIA EL PODER DE MIL ESCORPIONES",
	"MALAQUIAS HA MUERTO",
	"SOIS VOS, GUILERMO... PASAD, OS ESTABA ESPERANDO. TOMAD, AQUI ESTA VUESTRO PREMIO",
	"ESTAIS MUERTO, FRAY GUILLERMO, HABEIS CAIDO EN LA TRAMPA",
	"VENERABLE JORGE, VOIS NO PODEIS VERLO, PERO MI MAESTRO LLEVA GUANTES. PARA SEPARAR LOS FOLIOS TENDRIA QUE HUMEDECER LOS DEDOS EN LA LENGUA, HASTA QUE HUBIERA RECIBIDO SUFICIENTE VENENO",
	"SE ESTA COMIENDO EL LIBRO, MAESTRO",
	"DEBEIS ABANDONAR YA LA ABADIA",
	"ES MUY EXTRA�O, HERMANO GUILLERMO. BERENGARIO TENIA MANCHAS NEGRAS EN LA LENGUA Y EN LOS DEDOS",
	"PRONTO AMANECERA, MAESTRO",
	"LA LAMPARA SE AGOTA",
	"HABEIS ENTRADO EN MI CELDA",
	"SE HA AGOTADO LA LAMPARA",
	"JAMAS CONSEGUIREMOS SALIR DE AQUI",
	"ESPERAD, HERMANO",
	"OCUPAD VUESTRO SITIO, FRAY GUILLERMO",
	"ES EL COENA CIPRIANI DE ARISTOTELES. AHORA COMPRENDEREIS POR QUE TENIA QUE PROTEGERLO. CADA PALABRA ESCRITA POR EL FILOSOFO HA DESTRUIDO UNA PARTE DEL SABER DE LA CRISTIANDAD. SE QUE HE ACTUADO SIGUIENDO LA VOLUNTAD DEL SE�OR... LEEDLO, PUES, FRAY GUILLERMO. DESPUES TE LO MOSTRATE A TI MUCHACHO",
	"FUE UNA BUENA IDEA �VERDAD?; PERO YA ES TARDE",
	"QUIERO QUE CONOZCAIS AL HOMBRE MAS VIEJO Y SABIO DE LA ABADIA",
	"VENERABLE JORGE, EL QUE ESTA ANTE VOS ES FRAY GUILLERMO, NUESTRO HUESPED",
	"SED BIENVENIDO, VENERABLE HERMANO; Y ESCUCHAD LO QUE OS DIGO. LAS VIAS DEL ANTICRISTO SON LENTAS Y TORTUOSAS. LLEGA CUANDO MENOS LO ESPERAS. NO DESPERDICIEIS LOS ULTIMOS DIAS",
	"LO SIENTO, VENERABLE HERMANO, NO PODEIS SUBIR A LA BIBLIOTECA",
	"SI LO DESEAIS, BERENGARIO OS MOSTRARA EL SCRIPTORIUM",
	"AQUI TRABAJAN LOS MEJORES COPISTAS DE OCCIDENTE",
	"AQUI TRABAJABA VENACIO",
	"VENERABLE HERMANO, SOY SEVERINO, EL ENCARGADO DEL HOSPITAL. QUIERO ADVERTIROS QUE EN ESTA ABADIA SUCEDEN COSAS MUY EXTRA�AS. ALGUIEN NO QUIERE QUE LOS MONJES DECIDAN POR SI SOLOS LO QUE DEBEN SABER"
		// 666 PRUEBA, PRUEBA ...
	,	"CARGANDO PARTIDA..."
};

/////////////////////////////////////////////////////////////////////////////
// inicializaci�n y limpieza
/////////////////////////////////////////////////////////////////////////////

GestorFrases::GestorFrases()
{
	cpc6128 = elJuego->cpc6128;
	contadorActualizacion = 0;
	reproduciendoFrase = mostrandoFrase = false;
	frasePergamino = frases[0];
}

GestorFrases::~GestorFrases()
{
}

/////////////////////////////////////////////////////////////////////////////
// m�todos para mostrar una frase
/////////////////////////////////////////////////////////////////////////////

// muestra una frase por el marcador siempre y cuando no hubiera otra puesta
void GestorFrases::muestraFrase(int numFrase)
{
	// si ya est� reproduciendo una frase, sale
	if (mostrandoFrase){
		return;
	}

	// prepara todo para que se muestre la frase
	dibujaFrase(numFrase);
}

// muestra una frase por el marcador. Si hab�a otra, la para
void GestorFrases::muestraFraseYa(int numFrase)
{
	// para la frase actual
	reproduciendoFrase = mostrandoFrase = false;

	// limpia la parte del marcador donde se muestran las frases 
	elJuego->marcador->limpiaAreaFrases();

	// prepara todo para que se muestre la frase
	dibujaFrase(numFrase);
}

// actualiza el estado de la reproducci�n de las frases
void GestorFrases::actualizaEstado()
{
	mostrandoFrase = reproduciendoFrase;
}

/////////////////////////////////////////////////////////////////////////////
// dibujo de la frase
/////////////////////////////////////////////////////////////////////////////

// inicia el proceso para mostrar una frase por el marcador 
void GestorFrases::dibujaFrase(int numFrase)
{
	// inicia la frase
	fraseTerminada = false;
	reproduciendoFrase = mostrandoFrase = true;

	// guarda un puntero a la frase que se va a mostrar
	frase = frases[numFrase];

	// si se va a mostrar la frase del pergamino, cambia el puntero ya que en C++ no se puede modificar un static char *
	if (numFrase == 0){
		frase = (char *)frasePergamino.c_str();
	}

	// indica que a�n no se ha terminado la frase
	espaciosParaFin = 0;
}


void GestorFrases::procesaFraseActual()
{
	contadorActualizacion++;

	// s�lo actualiza las frases 1 vez cada 45 llamadas
	if (contadorActualizacion != 45){
		return;
	} else {
		contadorActualizacion = 0;
	}

	// si no se est� mostrando una frase en el marcador, sale
	if (!reproduciendoFrase){
		return;
	}
	
	// si no se ha terminado la frase actual, muestra otro caracter en el marcador
	if (!fraseTerminada){
		int caracter = *frase;

		// cambia los caracteres que no coinciden con el c�digo ASCII
		switch (*frase){
			case ',': 
				caracter = 0x3c; break;
			case '.': 
				caracter = 0x3d; break;
			case '�': 
				caracter = 0x40; break;
			case '�': 
				caracter = 0x57; break;
		}

		frase++;

		if (*frase == 0){
			fraseTerminada = true;
			espaciosParaFin = 17;
		}

		// realiza el scroll de la frase que se muestra en el marcador e imprime el nuevo car�cter
		scrollFrase();
		elJuego->marcador->imprimirCaracter(caracter, 216, 164, 2, 3);
	} else if (espaciosParaFin != 0){
		// si la frase se ha terminado y solo se est�n poniendo espacios para que se borre del marcador
		espaciosParaFin--;

		if (espaciosParaFin != 0){
			// si el marcador est� limpi�ndose, realiza el scroll y muestra espacios en blanco
			scrollFrase();
			elJuego->marcador->imprimirCaracter(0x20, 216, 164, 2, 3);
		} else {
			// si llega aqu�, la frase ha terminado
			reproduciendoFrase = false;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// m�todos de ayuda
/////////////////////////////////////////////////////////////////////////////

void GestorFrases::scrollFrase()
{
	// 8 l�neas de alto
	for (int j = 0; j < 8; j++){
		// desplaza 120/8 = 15 caracteres a la izquierda 1 caracter (cada caracter es de 8x8)
		for (int i = 0; i < 120; i++){
			cpc6128->setMode1Pixel(104 + i - 8, 164 + j, cpc6128->getMode1Pixel(104 + i, 164 + j));
		}
	}
}
