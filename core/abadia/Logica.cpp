// Logica.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "../TimingHandler.h"

//666
#include "iostream"
#include "fstream"
using namespace std;

#include "Abad.h"
#include "AccionesDia.h"
#include "Adso.h"
#include "Berengario.h"
#include "Bernardo.h"
#include "BuscadorRutas.h"
#include "Controles.h"
#include "GestorFrases.h"
#include "Guillermo.h"
#include "Jorge.h"
#include "Juego.h"
#include "Logica.h"
#include "Malaquias.h"
#include "Marcador.h"
#include "Monje.h"
#include "MotorGrafico.h"
#include "Objeto.h"
#include "Personaje.h"
#include "PersonajeConIA.h"
#include "Puerta.h"
#include "RejillaPantalla.h"
#include "Severino.h"
#include "Sprite.h"
#include "SpriteLuz.h"
#include "SpriteMonje.h"

// 666 prueba para poder cambiar la paleta al cargar la partida
//#include "Paleta.h"

using namespace Abadia;

/////////////////////////////////////////////////////////////////////////////
// inicializaci�n y limpieza
/////////////////////////////////////////////////////////////////////////////

Logica::Logica(UINT8 *romData, UINT8 *buf, int lgtud)
{
	// crea los objetos usados por la l�gica
	accionesDia = new AccionesDia();
	buscRutas = new BuscadorRutas(buf, lgtud);
	gestorFrases = new GestorFrases();
	roms = romData;
}

Logica::~Logica()
{
	// borra los objetos usados por la l�gica
	delete accionesDia;
	delete buscRutas;
	delete gestorFrases;
}

/////////////////////////////////////////////////////////////////////////////
// incializaci�n de las variables para una nueva partida
/////////////////////////////////////////////////////////////////////////////

// inicia la l�gica
void Logica::inicia()
{
	// inicia las entidades del juego
	iniciaSprites();
	iniciaPersonajes();
	iniciaPuertas();
	iniciaObjetos();

	// inicia la l�gica relacionada con la habitaci�n del espejo
	iniciaHabitacionEspejo();

	// inicia las variables de la l�gica del juego
	dia = 1;
	momentoDia = NONA;
	duracionMomentoDia = 0;
	oldMomentoDia = 0;
	avanzarMomentoDia = false;
	obsequium = 31;
	haFracasado = false;
	investigacionCompleta = false;
	bonus = 0;

	mascaraPuertas = 0xef;

	seAcabaLaNoche = false;
	haAmanecido = false;
	usandoLampara = false;
	lamparaDesaparecida = true;
	tiempoUsoLampara = 0;
	cambioEstadoLampara = 0;
	cntTiempoAOscuras = 0;

	cntLeeLibroSinGuantes = 0;
	pergaminoGuardado = false;

	hayMovimiento = false;
	cntMovimiento = 0;

	// inicialmente la c�mara sigue a guillermo
	numPersonajeCamara = 0;
	opcionPersonajeCamara = 0;
	elMotorGrafico->personaje = guillermo;
	
	buscRutas->generadoCamino = false;
}

/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con el libro
/////////////////////////////////////////////////////////////////////////////

void Logica::compruebaLecturaLibro()
{
	// si guillermo no tiene el libro, sale
	if ((guillermo->objetos & LIBRO) == 0) return;

	// si guillermo tiene los guantes, sale
	if ((guillermo->objetos & GUANTES) != 0) return;

	cntLeeLibroSinGuantes++;

	// si guillermo ha leido un poco del libro sin los guantes, muere
	if ((cntLeeLibroSinGuantes & 0xff) == 0){

		// modifica el estado de fray guillermo para que suba al morir
		guillermo->estado = guillermo->sprite->posYPant/2;
		guillermo->incrPosY = -2;
		
		haFracasado = true;

		// escribe en el marcador la frase: ESTAIS MUERTO, FRAY GUILLERMO, HABEIS CAIDO EN LA TRAMPA
		gestorFrases->muestraFrase(0x22);
	}
}

/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con los bonus y los cambios de c�mara
/////////////////////////////////////////////////////////////////////////////

void Logica::actualizaBonusYCamara()
{
	// comprueba si hay que seguir a berengario
	if (((berengario->aDondeVa == POS_LIBRO) && (berengario->posX < 0x50) && (berengario->estaVivo)) ||	(berengario->aDondeVa == POS_ABAD)){
		// si va al scriptorium a por el libro o va a avisar al abad, indica el posible cambio de c�mara
		opcionPersonajeCamara = 4;

		return;
	}

	// comprueba si hay que seguir a bernardo gui
	if (bernardo->aDondeVa == POS_ABAD){
		// si va a avisar al abad, indica el posible cambio de c�mara
		opcionPersonajeCamara = 7;

		return;
	}

	// comprueba si hay que seguir al abad
	if (((momentoDia == SEXTA) && (abad->aDondeHaLlegado >= 2)) || (abad->estado == 0x15) || (abad->guillermoHaCogidoElPergamino) || (abad->estado == 0x0b)){
		// si en sexta va a alg�n lugar interesante o si va a dejar el pergamino a su celda o si berengario le ha dicho
		// que bernardo tiene el pergamino o si est� en estado de echar a guillermo, indica el posible cambio de c�mara
		opcionPersonajeCamara = 3;

		return;
	}

	// comprueba si hay que seguir a malaqu�as
	if ((malaquias->aDondeVa == POS_ABAD) || ((momentoDia == VISPERAS) && (malaquias->estado < 0x06))){
		// si va a avisar al abad o es v�speras y no ha llegado a la cocina, indica el posible cambio de c�mara
		opcionPersonajeCamara = 2;

		return;
	}

	// comprueba si hay que seguir a severino
	if (severino->aDondeVa == POS_GUILLERMO){
		// si va hacia la posici�n de guillermo, indica el posible cambio de c�mara
		opcionPersonajeCamara = 5;

		return;
	}

	// en otro caso, la c�mara sigue a guillermo
	opcionPersonajeCamara = 0;

	// actualiza los bonus dependiendo de si guillermo y adso tienen los objetos que dan bonus
	bonus |= (guillermo->objetos & (GUANTES | LLAVE1 | LLAVE2)) | (adso->objetos & LLAVE3);

	// si guillermo tiene el pergamino
	if ((guillermo->objetos & PERGAMINO) == PERGAMINO){
		// si es la noche del tercer d�a
		if ((dia == 3) && (momentoDia == NOCHE)){
			bonus |= 0x1000;
		}

		// si tiene las gafas
		if ((guillermo->objetos & GAFAS) == GAFAS){
			bonus |= 0x0100;
		}

		// si guillermo entra en la habitaci�n del abad
		if ((elMotorGrafico->numPantalla == 0x0d) && (numPersonajeCamara == 0)){
			bonus |= 0x2000;
		}
	}

	// si guillermo visita el ala izquierda por la noche
	if ((momentoDia == NOCHE) && (guillermo->posX < 0x60)){
		bonus |= 0x0001;
	}
	
	// si guillermo est� en la biblioteca
	if (guillermo->altura >= 0x16){
		// si tiene las gafas
		if ((guillermo->objetos & GAFAS) == GAFAS){
			bonus |= 0x0080;
		}

		// si adso ha cogido la l�mpara
		if ((adso->objetos & LAMPARA) == LAMPARA){
			bonus |= 0x0020;
		}

		bonus |= 0x0010;
	}

	// si ha entrado en la habitaci�n que hay detr�s del espejo
	if (elMotorGrafico->numPantalla == 0x72){
		bonus |= 0x0200;
	}
}

void Logica::compruebaBonusYCambiosDeCamara()
{
	// comprueba si hay opci�n de seguir a alg�n monje y actualiza los bonus
	actualizaBonusYCamara();

	bool teclaPulsada = false;

	// si estamos en la conversaci�n con jorge sobre el libro, la c�mara sigue a jorge
	if (((guillermo->objetos & GUANTES) == GUANTES) && ((jorge->estado == 0x0d) || (jorge->estado == 0x0e) || (jorge->estado == 0x0f))){
		cntMovimiento = 0x32;
		opcionPersonajeCamara = 6;
	} else {
		// comprueba si se est� moviendo guillermo
		if ((losControles->estaSiendoPulsado(P1_UP)) || (losControles->estaSiendoPulsado(P1_LEFT)) || (losControles->estaSiendoPulsado(P1_RIGHT))){
			teclaPulsada = true;
		}
	}

	// si no se pulsa ninguna tecla y el contador llega al umbral, comprueba los cambios de c�mara
	if (!teclaPulsada){
		cntMovimiento++;

		// si no se ha llegado al l�mite, sale
		if (cntMovimiento < 0x32){
			return;
		}

		// si hay la opci�n de seguir a un personaje distinto, cambia de c�mara
		if (numPersonajeCamara != opcionPersonajeCamara){
			numPersonajeCamara = opcionPersonajeCamara;
			cntMovimiento = opcionPersonajeCamara;
		}
	} else {
		// en otro caso, la c�mara sigue a guillermo
		numPersonajeCamara = 0;
		cntMovimiento = 0;
	}

	// fija el personaje al que sigue la c�mara
	elMotorGrafico->personaje = elJuego->personajes[numPersonajeCamara & 0x7f];
}

/////////////////////////////////////////////////////////////////////////////
// m�todos para coger/dejar objetos
/////////////////////////////////////////////////////////////////////////////

// comprueba los objetos que pueden coger los personajes
void Logica::compruebaCogerObjetos()
{
	// para cada personaje
	for (int i = 0; i < Juego::numPersonajes; i++){
		Personaje *pers = elJuego->personajes[i];
		
		// si el personaje est� cogiendo o dejando un objeto, pasa al siguiente personaje
		pers->contadorObjetos--;
		if (pers->contadorObjetos != -1) return;
		pers->contadorObjetos++;

		// elimina de la m�scara de los objetos que podemos coger los que ya tenemos
		int objetosACoger = (pers->mascaraObjetos ^ pers->objetos) & pers->mascaraObjetos;

		int mascara = 1 << Juego::numObjetos;

		// recorre los objetos que se pueden coger
		for (int j = 0; j < Juego::numObjetos; j++){
			mascara = mascara >> 1;

			// si no hay que comprobar el objeto actual, pasa al siguiente
			if ((objetosACoger & mascara) == 0) continue;

			// si el personaje ha cogido el objeto, pasa al siguiente personaje
			if (elJuego->objetos[j]->seHaCogidoPor(pers, mascara)) break;
		}
	}
}


// comprueba si el personaje que se le pasa puede dejar un objeto
void Logica::dejaObjeto(Personaje *pers)
{
	int posXObj, posYObj, alturaObj;

	// comprueba si el personaje puede dejar un objeto
	int numObj = pers->puedeDejarObjeto(posXObj, posYObj, alturaObj);

	// si el personaje puede dejar un objeto, lo hace
	if (numObj != -1){
		elJuego->objetos[numObj]->dejar(pers, 1 << (Juego::numObjetos - 1 - numObj), posXObj, posYObj, alturaObj);
	}
}

void Logica::compruebaCogerDejarObjetos()
{
	// guarda una copia de los objetos que tenemos
	int objetosGuillermo = guillermo->objetos;
	int objetosAdso = adso->objetos;

	// comprueba si los personajes cogen alg�n objeto
	compruebaCogerObjetos();

	// comprueba si los personajes dejan alg�n objeto
	// si se pulsa el espacio, deja un objeto (si tiene)
	if (losControles->estaSiendoPulsado(P1_BUTTON1)){
		dejaObjeto(guillermo);
	}

	// actualiza las puertas a las que pueden entrar guillermo y adso
	guillermo->permisosPuertas = (guillermo->permisosPuertas) | ((guillermo->objetos & LLAVE1) >> 3) | (guillermo->objetos & LLAVE2);
	adso->permisosPuertas = (adso->permisosPuertas & 0xef) | ((adso->objetos & LLAVE3) << 3);

	// calcula los objetos que se han cambiado
	int difObjetos = guillermo->objetos ^ objetosGuillermo;

	// si ha cambiado el estado de las gafas o el pergamino, y si tenemos los 2 objetos, comprueba 
	// si hay que generar el n�mero del espejo y muestra el texto del pergamino
	if ((difObjetos & (PERGAMINO | GAFAS)) != 0){
		if ((guillermo->objetos & (PERGAMINO | GAFAS)) == (PERGAMINO | GAFAS)){
			generaNumeroRomano();
		}
	}

	// si han cambiado los objetos de guillermo, actualiza el marcador
	if (objetosGuillermo != guillermo->objetos){
		elMarcador->dibujaObjetos(guillermo->objetos, difObjetos);
	}

	// recorre los objetos indicando que ya no se est�n cogiendo
	for (int i = 0; i < Juego::numObjetos; i++){
		elJuego->objetos[i]->seEstaCogiendo = false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con las puertas
/////////////////////////////////////////////////////////////////////////////

void Logica::compruebaAbrirCerrarPuertas()
{
	for (int i = 0; i < Juego::numPuertas; i++){
		Puerta *puerta = elJuego->puertas[i];

		// inicialmente no hay que redibujar la puerta
		puerta->hayQueRedibujar = false;

		// comprueba si hay que abrir o cerrar la puerta
		puerta->compruebaAbrirCerrar(elJuego->personajes, Juego::numPersonajes);

		int posXPant, posYPant, sprPosY;

		// actualiza la posici�n del sprite seg�n la c�mara
		if (elMotorGrafico->actualizaCoordCamara(puerta, posXPant, posYPant, sprPosY) != -1){
			puerta->notificaVisibleEnPantalla(posXPant, posYPant, sprPosY);
		} else {
			puerta->sprite->esVisible = false;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// acciones programadas
/////////////////////////////////////////////////////////////////////////////

void Logica::ejecutaAccionesMomentoDia()
{
	// obtiene el estado actualizado del gestor de frases
	gestorFrases->actualizaEstado();

	// si el personaje que muestra la c�mara est� en medio de una animaci�n, sale
	if ((elMotorGrafico->personaje->contadorAnimacion & 0x01) != 0) return;

	if (!avanzarMomentoDia){
		accionesDia->ejecutaAccionesProgramadas();

		return;
	}

	// si est� mostrando una frase, sale
	if (elGestorFrases->mostrandoFrase) return;

	// si ha cambiado el momento del d�a, ejecuta unas acciones dependiendo del momento del d�a
	avanzarMomentoDia = false;
	elMarcador->avanzaMomentoDia();
	accionesDia->ejecutaAccionesProgramadas();
}

/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con el tiempo
/////////////////////////////////////////////////////////////////////////////

// comprueba si se ha agotado la l�mpara
void Logica::compruebaFinLampara()
{
	// si adso no tiene la l�mpara, sale
	if ((adso->objetos & LAMPARA) == 0) return;

	// si no se est� usando la l�mpara, sale
	if (!usandoLampara) return;

	// si est� en una pantalla iluminada, sale
	if (elMotorGrafico->pantallaIluminada) return;

	// si llega aqu� es porque se est� usando la l�mpara
	tiempoUsoLampara++;

	// cada 0x100 veces, comprueba el estado
	if ((tiempoUsoLampara & 0xff) != 0) return;

	// si no se ha procesado todav�a el �ltimo cambio en el estado de la l�mpara, sale
	if (cambioEstadoLampara != 0) return;

	if (((tiempoUsoLampara >> 8) & 0xff) == 3){
		// si el tiempo de uso de la l�mpara llega a 0x300, indica que se est� agotando la l�mpara
		cambioEstadoLampara = 1;
	} else if (((tiempoUsoLampara >> 8) & 0xff) == 6){
		// si el tiempo de uso de la l�mpara llega a 0x600, indica que se ha agotado la l�mpara
		cambioEstadoLampara = 2;
	}
}

// comprueba si se est� acabando la noche
void Logica::compruebaFinNoche()
{
	seAcabaLaNoche = false;

	// si esta etapa del d�a no tiene una duraci�n programada, sale
	if (duracionMomentoDia == 0) return;

	// cada 0x100 veces, comprueba si se est� acabando la noche
	if (((duracionMomentoDia & 0xff) == 0) && (momentoDia == NOCHE)){
		if (((duracionMomentoDia >> 8) & 0xff) == 2){
			seAcabaLaNoche = true;
		} else {
			if (((duracionMomentoDia >> 8) & 0xff) == 0){
				haAmanecido = true;
			}
		}
	}
}

// actualiza las variables relacionadas con el paso del tiempo
void Logica::actualizaVariablesDeTiempo()
{
	// comprueba si hay que pasar al siguiente momento del d�a
	compruebaFinMomentoDia();

	// comprueba si se ha agotado la l�mpara
	compruebaFinLampara();

	// comprueba si se est� acabando la noche
	compruebaFinNoche();
}

// comprueba si hay que pasar al siguiente momento del d�a
void Logica::compruebaFinMomentoDia()
{
	// si se pulsa intro, avanza el momento del d�a (s�lo en modo informaci�n)
	// NOTA: usar esto con mucho cuidado ya que puede romper la l�gica normal del juego al no producirse algunos eventos
	if (losControles->seHaPulsado(KEYBOARD_INTRO) && elJuego->modoInformacion){
		elMarcador->avanzaMomentoDia();
	}

	// si esta etapa del d�a tiene una duraci�n programada, comprueba si ha terminado
	if (duracionMomentoDia != 0){
		duracionMomentoDia--;

		if (duracionMomentoDia == 0){
			elMarcador->avanzaMomentoDia();
		}
	}
}

// calcula el porcentaje de misi�n completada. Si se ha completado el juego, muestra el final
int Logica::calculaPorcentajeMision()
{
	if (!investigacionCompleta){
		// asigna un porcentaje seg�n el tiempo que haya pasado de misi�n
		int porc = 7*(dia - 1) + momentoDia;

		// modifica el porcentaje seg�n los bonus obtenidos
		for (int i = 0; i < 16; i++){
			if ((bonus & (1 << i)) != 0){
				porc += 4;
			}
		}

		// si no hemos obtenido un porcentaje >= 5%, pone el porcentaje a 0
		if (porc < 5){
			porc = 0;
		}

		return porc;
	} else {
		// si se ha completado la investigaci�n, muestra el pergamino del final
		elJuego->muestraFinal();

		return 0;
	}
}

void Logica::reiniciaContadoresLampara()
{
	// si malaqu�as no tiene la l�mpara y no se ha usado, sale
	if (((malaquias->objetos & LAMPARA) == 0) && (tiempoUsoLampara == 0)) return;

	// pone a 0 el tiempo de uso de la l�mpara e indica que no se est� usando
	tiempoUsoLampara = 0;
	usandoLampara = false;

	// se asegura de que ni adso ni malaqu�as tengan la l�mpara
	adso->objetos = adso->objetos & ~LAMPARA;
	malaquias->objetos = malaquias->objetos & ~LAMPARA;

	// desaparece la l�mpara
	lamparaDesaparecida = true;
	elJuego->objetos[7]->seHaCogido = false;
	elJuego->objetos[7]->seEstaCogiendo = false;
	elJuego->objetos[7]->personaje = 0;
	elJuego->objetos[7]->posX = 0;
	elJuego->objetos[7]->posY = 0;
	elJuego->objetos[7]->altura = 0;
	elJuego->objetos[7]->orientacion = DERECHA;
}

/////////////////////////////////////////////////////////////////////////////
// m�todos relacionados con la habitaci�n del espejo
/////////////////////////////////////////////////////////////////////////////

// si el espejo est� cerrado, actualiza los sprites de los reflejos de adso y guillermo
void Logica::realizaReflejoEspejo()
{
	if (espejoCerrado){
		if (!reflejaPersonaje(guillermo, elJuego->sprites[Juego::spritesReflejos])){
			elJuego->sprites[Juego::spritesReflejos]->esVisible = false;
		}
		if (!reflejaPersonaje(adso, elJuego->sprites[Juego::spritesReflejos + 1])){
			elJuego->sprites[Juego::spritesReflejos + 1]->esVisible = false;
		}
	} else {
		elJuego->sprites[Juego::spritesReflejos]->esVisible = false;
		elJuego->sprites[Juego::spritesReflejos + 1]->esVisible = false;
	}
}

// comprueba si un personaje est� enfrente del espejo, y si es as�, rellena el sprite con su reflejo
bool Logica::reflejaPersonaje(Personaje *pers, Sprite *spr)
{
	// si no se est� en la habitaci�n del espejo, sale
	if (elMotorGrafico->rejilla->minPosX != 0x1c) return false;
	if (elMotorGrafico->rejilla->minPosY != 0x5c) return false;
	if (elMotorGrafico->obtenerAlturaBasePlanta(pers->altura) != 0x16) return false;

	// si el personaje no est� a una altura donde puede reflejarse en el espejo, sale
	if ((pers->altura - elMotorGrafico->obtenerAlturaBasePlanta(pers->altura)) >= 8) return false;

	// si el personaje no est� en frente del espejo, sale
	int posX = pers->posX - 0x20;
	if ((posX < 0) || (posX >= 10)) return false;
	int posY = pers->posY - 0x62;
	if ((posY < 0) || (posY >= 10)) return false;

	// guarda los valores del personaje que se modificar�n
	int orientacion = pers->orientacion;
	int contadorAnimacion = pers->contadorAnimacion;
	int oldPosX = pers->posX;

	// refleja el personaje con respecto al espejo
	pers->contadorAnimacion = contadorAnimacion ^ 2;
	pers->posX = 0x21 - (oldPosX - 0x21);
	if ((orientacion & 0x01) == 0){
		pers->orientacion = orientacion ^ 2;
	}
	if (pers->enDesnivel){
		pers->posX--;
	}

	// actualiza los valores del sprite
	spr->oldPosXPant = spr->posXPant;
	spr->oldPosYPant = spr->posYPant;
	spr->oldAlto = spr->alto;
	spr->oldAncho = spr->ancho;

	// calcula los valores del sprite reflejado
	Sprite *sprTemp = pers->sprite;
	pers->sprite = spr;
	pers->actualizaSprite();
	pers->sprite = sprTemp;

	// restaura los valores del personaje
	pers->orientacion = orientacion;
	pers->contadorAnimacion = contadorAnimacion;
	pers->posX = oldPosX;

	return true;
}

// comprueba si se est� delante del espejo y si se ha pulsado la Q y la R en alguna de las escaleras
int Logica::pulsadoQR()
{
	// si no est� delante del espejo, sale
	if ((guillermo->posX != 0x22) || (guillermo->altura != 0x1a)){
		return 0;
	}

	// si no se ha pulsado la Q y la R, sale
	if (!losControles->estaSiendoPulsado(KEYBOARD_Q) || !losControles->estaSiendoPulsado(KEYBOARD_R)){
		return 0;
	}

	// comprueba si se ha pulsado la Q y la R en una de las escaleras
	switch (guillermo->posY){
		case 0x6d:	// si est� en la escalera de la izquierda, sale devolviendo 1
			return 1;
		case 0x69:	// si est� en la escalera del centro, sale devolviendo 2
			return 2;
		case 0x65:	// si est� en la escalera de la derecha, sale devolviendo 3
			return 3;
		default:	// en otro caso, devuelve 0
			return 0;
	}
}

// comprueba si se ha pulsado QR en la habitaci�n del espejo y act�a en consecuencia
void Logica::compruebaAbreEspejo()
{
	// si se ha abierto el espejo, sale
	if (!espejoCerrado) return;

	// comprueba si se est� delante del espejo y si se ha pulsado la Q y la R en alguna de las escaleras
	int estadoQR = pulsadoQR();

	// si no se ha pulsado QR en alguna escalera del espejo, sale
	if (estadoQR == 0) return;

	// marca como conseguido el bonus de abrir el espejo
	bonus |= 0x0400;

	// si puls� QR en el lugar correcto
	if (estadoQR == numeroRomano){
		// modifica los datos de altura de la habitaci�n del espejo para que guillermo puede atravesarlo
		roms[despDatosAlturaEspejo] = 0xff;

		// cambia los datos de un bloque de la habitaci�n del espejo para que el espejo est� abierto
		roms[despBloqueEspejo] = 0x51;
	} else {
		// en otro caso, cambia el estado de guillermo y lo mata
		guillermo->estado = 0x14;
		haFracasado = true;

		// cambia los datos de un bloque de la habitaci�n del espejo para que se abra una trampa y se caiga guillermo
		roms[despBloqueEspejo - 2] = 0x6b;

		// escribe en el marcador la frase: ESTAIS MUERTO, FRAY GUILLERMO, HABEIS CAIDO EN LA TRAMPA
		gestorFrases->muestraFrase(0x22);
	}

	// indica que se ha abierto el espejo y hay que la pantalla ha cambiado
	elMotorGrafico->posXPantalla = elMotorGrafico->posYPantalla = -1;
	espejoCerrado = false;
}

// si no se hab�a generado el n�mero romano para el enigma de la habitaci�n del espejo, lo genera
void Logica::generaNumeroRomano()
{
	static char* tablaNumerosRomanos[3] = {
		"IXX",
		"XIX",
		"XXI"
	};

	if (numeroRomano == 0){
		// genera un n�mero aleatorio entre 1 y 3
		srand((unsigned int)elJuego->timer->getTime());
		numeroRomano = rand() & 0x03;

		if (numeroRomano == 0){
			numeroRomano = 1;
		}

		// copia el n�mero romano a la frase que se muestra al leer el manuscrito
		for (int i = 0; i < 3; i++){
			gestorFrases->frasePergamino[37 + i] = tablaNumerosRomanos[numeroRomano - 1][i];
		}
	}

	// escribe en el marcador la frase: SECRETUM FINISH AFRICAE, MANUS SUPRA XXX AGE PRIMUM ET SEPTIMUM DE QUATOR
	gestorFrases->muestraFrase(0x00);
}

/////////////////////////////////////////////////////////////////////////////
// inicializaci�n de la habitaci�n del espejo
/////////////////////////////////////////////////////////////////////////////

// obtiene el desplazamiento hasta los datos de la habitaci�n del espejo
void Logica::despHabitacionEspejo()
{
	// apunta a datos de altura de la segunda planta de la abad�a
	int desp = 0x18000 + 0x1056;

	// busca el f�n de la tabla
	while (roms[desp] != 0xff){
		desp = ((roms[desp] & 0x08) == 0x08) ? desp + 5 : desp + 4;
	}

	// guarda la direcci�n para luego
	despDatosAlturaEspejo = desp;

	// apunta al inicio de los datos de los bloques que forman las pantallas
	desp = 0x1c000;

	// avanza hasta la habitaci�n del espejo
	for (int i = 0; i < 0x72; i++){
		desp = desp + roms[desp];
	}

	despBloqueEspejo = 0;

	// recorre los datos que forman la habitaci�n del espejo buscando el bloque del espejo
	for (int i = 0; i < 0x100; i++){
		if (roms[desp] == 0x1f){
			// si encuentra el bloque que forma el espejo y est� abierto
			if ((roms[desp + 1] == 0xaa) && (roms[desp + 2] == 0x51)){
				desp = desp + 2;

				// modifica el bloque para que el espejo se muestre cerrado
				roms[desp] = 0x11;

				// guarda el desplazamiento al bloque para despu�s
				despBloqueEspejo = desp;
				break;
			}
		}

		desp++;
	}
}

// fija el estado inicial de la habitaci�n del espejo
void Logica::iniciaHabitacionEspejo()
{
	// inicialmente, el espejo est� cerrado y no se ha generado el n�mero romano para el enigma del espejo
	espejoCerrado = true;
    numeroRomano = 0;

	int datosAltura[] = { 0xf5, 0x20, 0x62, 0x0b, 0xff };

	// modifica los datos de altura de la habitaci�n del espejo
	for (int i = 0; i < 5; i++){
		roms[despDatosAlturaEspejo + i] = datosAltura[i];
	}

	// modifica la habitaci�n del espejo para que el espejo aparezca cerrado
	roms[despBloqueEspejo] = 0x11;

	// modifica la habitaci�n del espejo para que la trampa no est� abierta
	roms[despBloqueEspejo - 2] = 0x1f;
}

/////////////////////////////////////////////////////////////////////////////
// inicializaci�n de las entidades del juego
/////////////////////////////////////////////////////////////////////////////

// inicia los sprites del juego poni�ndolos como no visibles
void Logica::iniciaSprites()
{
	for (int i = 0; i < Juego::numSprites; i++){
		elJuego->sprites[i]->esVisible = false;
	}
}

// inicia los personajes del juego
void Logica::iniciaPersonajes()
{
	// obtiene los personajes del juego
	guillermo = (Guillermo *)elJuego->personajes[0];
	adso = (Adso *)elJuego->personajes[1];
	malaquias = (Malaquias *)elJuego->personajes[2];
	abad = (Abad *)elJuego->personajes[3];
	berengario = (Berengario *)elJuego->personajes[4];
	severino = (Severino *)elJuego->personajes[5];
	jorge = (Jorge *)elJuego->personajes[6];
	bernardo = (Bernardo *)elJuego->personajes[7];

	// recorre los personajes e inicia sus caracter�sticas comunes
	for (int i = 0; i < Juego::numPersonajes; i++){
		Personaje *pers = elJuego->personajes[i];
		pers->contadorAnimacion = 0;
		pers->orientacion = DERECHA;
		pers->bajando = false;
		pers->giradoEnDesnivel = false;
		pers->enDesnivel = false;

		if (pers == guillermo) continue;

		PersonajeConIA *persIA = (PersonajeConIA *)elJuego->personajes[i];
		persIA->numBitAcciones = 0;
		persIA->pensarNuevoMovimiento = false;
		persIA->posAcciones = 0;
		persIA->bufAcciones[0] = 0x10;	// acci�n para que piense un nuevo movimiento
	}

	// guillermo
	guillermo->objetos = GAFAS;
	guillermo->mascaraObjetos = LIBRO | GUANTES | GAFAS | PERGAMINO | LLAVE1 | LLAVE2;
	guillermo->permisosPuertas = 0x08;
	guillermo->posX = 0x88;
	guillermo->posY = 0xa8;
	guillermo->altura = 0x00;
	guillermo->estado = 0x00;
	guillermo->incrPosY = 2;

	// adso
	adso->valorPosicion = 0x20;
	adso->mascaraObjetos = LLAVE3 | LAMPARA;
	adso->permisosPuertas = 0x08;
	adso->posX = guillermo->posX - 2;
	adso->posY = guillermo->posY + 2;
	adso->altura = guillermo->altura;
	adso->estado = 0;
	adso->aDondeHaLlegado = -1;
	adso->oldEstado = 0;
	adso->movimientosFrustados = 0;
	adso->cntParaDormir = 0;
	
	// malaqu�as
	malaquias->posX = 0x26;
	malaquias->posY = 0x26;
	malaquias->altura = 0x0f;
	malaquias->mascaraObjetos = LLAVE3 | LAMPARA;
	malaquias->permisosPuertas = 0x1f;
	malaquias->estado = 0;
	malaquias->aDondeHaLlegado = -6;
	malaquias->estado2 = 0;
	malaquias->estaMuerto = 0;

	// el abad
	abad->posX = 0x88;
	abad->posY = 0x84;
	abad->altura = 0x02;
	abad->mascaraObjetos = PERGAMINO;
	abad->permisosPuertas = 0x19;
	abad->puedeQuitarObjetos = true;
	abad->estado = 0;
	abad->aDondeHaLlegado = -6;
	abad->guillermoHaCogidoElPergamino = false;
	abad->contador = 0;
	abad->numFrase = 0;

	// berengario
	berengario->posX = 0x28;
	berengario->posY = 0x48;
	berengario->altura = 0x0f;
	berengario->mascaraObjetos = 0x00;
	berengario->permisosPuertas = 0x1f;
	berengario->fijaCapucha(false);
	berengario->estado = 0;
	berengario->aDondeHaLlegado = -6;
	berengario->estado2 = 0;
	berengario->estaVivo = true;
	berengario->contadorPergamino = 0;

	// severino
	severino->posX = 0xc8;
	severino->posY = 0x28;
	severino->altura = 0x00;
	severino->mascaraObjetos = 0x00;
	severino->permisosPuertas = 0x0c;
	severino->estado = 0;
	severino->aDondeHaLlegado = -6;
	severino->estaVivo = true;

	// jorge
	jorge->posX = 0x00;
	jorge->posY = 0x00;
	jorge->altura = 0x00;
	jorge->mascaraObjetos = 0x00;
	jorge->permisosPuertas = 0x1f;
	jorge->estado = 0;
	jorge->aDondeHaLlegado = -6;
	jorge->estaActivo = false;
	jorge->contadorHuida = 0;

	// bernardo gui
	bernardo->posX = 0x00;
	bernardo->posY = 0x00;
	bernardo->altura = 0x00;
	bernardo->permisosPuertas = 0x1f;
	bernardo->puedeQuitarObjetos = true;
	bernardo->estado = 0;
	bernardo->aDondeHaLlegado = -6;
	bernardo->estaEnLaAbadia = false;
}

// inicia las puertas del juego
void Logica::iniciaPuertas()
{
	Puerta **puertas = elJuego->puertas;

	// puerta de la habitaci�n del abad
	puertas[0]->identificador = 0x01;
	puertas[0]->orientacion = ABAJO;
	puertas[0]->posX = 0x61;
	puertas[0]->posY = 0x37;
	puertas[0]->altura = 0x02;
	puertas[0]->haciaDentro = true;

	// puerta de la habitaci�n de los monjes
	puertas[1]->identificador = 0x02;
	puertas[1]->orientacion = IZQUIERDA;
	puertas[1]->posX = 0xb7;
	puertas[1]->posY = 0x1e;
	puertas[1]->altura = 0x02;
	puertas[1]->haciaDentro = true;

	// puerta de la habitaci�n de severino
	puertas[2]->identificador = 0x04;
	puertas[2]->orientacion = DERECHA;
	puertas[2]->posX = 0x66;
	puertas[2]->posY = 0x5f;
	puertas[2]->altura = 0x02;
	puertas[2]->haciaDentro = false;

	// puerta que conecta las habitaciones con la iglesia
	puertas[3]->identificador = 0x08;
	puertas[3]->orientacion = ARRIBA;
	puertas[3]->posX = 0x9e;
	puertas[3]->posY = 0x28;
	puertas[3]->altura = 0x02;
	puertas[3]->haciaDentro = true;

	// puerta del pasadizo de detr�s de la cocina
	puertas[4]->identificador = 0x10;
	puertas[4]->orientacion = ARRIBA;
	puertas[4]->posX = 0x7e;
	puertas[4]->posY = 0x26;
	puertas[4]->altura = 0x02;
	puertas[4]->haciaDentro = false;

	// primera puerta que cierra el paso a la parte izquierda de la planta baja de la abad�a
	puertas[5]->identificador = 0x00;
	puertas[5]->orientacion = IZQUIERDA;
	puertas[5]->posX = 0x60;
	puertas[5]->posY = 0x76;
	puertas[5]->altura = 0x00;
	puertas[5]->haciaDentro = true;
	puertas[5]->estaFija = true;
	puertas[5]->estaAbierta = true;

	// segunda puerta que cierra el paso a la parte izquierda de la planta baja de la abad�a
	puertas[6]->identificador = 0x00;
	puertas[6]->orientacion = IZQUIERDA;
	puertas[6]->posX = 0x60;
	puertas[6]->posY = 0x7b;
	puertas[6]->altura = 0x00;
	puertas[6]->haciaDentro = false;
	puertas[6]->estaFija = true;
	puertas[6]->estaAbierta = true;
}

// inicia los objetos del juego
void Logica::iniciaObjetos()
{
	Objeto **objetos = elJuego->objetos;

	// libro
	objetos[0]->orientacion = ABAJO;
	objetos[0]->posX = 0x34;
	objetos[0]->posY = 0x5e;
	objetos[0]->altura = 0x13;

	// guantes
	objetos[1]->orientacion = DERECHA;
	objetos[1]->posX = 0x6b;
	objetos[1]->posY = 0x55;
	objetos[1]->altura = 0x06;

	// gafas
	objetos[2]->orientacion = DERECHA;
	objetos[2]->seHaCogido = true;
	objetos[2]->personaje = guillermo;

	// pergamino
	objetos[3]->orientacion = ABAJO;
	objetos[3]->posX = 0x36;
	objetos[3]->posY = 0x5e;
	objetos[3]->altura = 0x13;

	// llave 1
	objetos[4]->orientacion = DERECHA;
	objetos[4]->posX = 0x00;
	objetos[4]->posY = 0x00;
	objetos[4]->altura = 0x00;

	// llave 2
	objetos[5]->orientacion = DERECHA;
	objetos[5]->posX = 0x00;
	objetos[5]->posY = 0x00;
	objetos[5]->altura = 0x00;

	// llave 3
	objetos[6]->orientacion = DERECHA;
	objetos[6]->posX = 0x35;
	objetos[6]->posY = 0x35;
	objetos[6]->altura = 0x13;

	// l�mpara
	objetos[7]->orientacion = DERECHA;
	objetos[7]->posX = 0x08;
	objetos[7]->posY = 0x08;
	objetos[7]->altura = 0x02;
}

//666
void Logica::save(ofstream &out)
{
		out << dia;
		out << endl;
		out << momentoDia;
		out << endl;
		out << duracionMomentoDia;
		out << endl;
		out << oldMomentoDia;
		out << endl;
		out << avanzarMomentoDia;
		out << endl;
		out << obsequium;
		out << endl;
		out << haFracasado;
		out << endl;
		out << investigacionCompleta;
		out << endl;
		out << bonus;
		out << endl;
		out << mascaraPuertas;
		out << endl;
		out << espejoCerrado;
		out << endl;
		out << numeroRomano;
		out << endl;
		out << despDatosAlturaEspejo;
		out << endl;
		out << despBloqueEspejo;
		out << endl;
		out << seAcabaLaNoche;
		out << endl;
		out << haAmanecido;
		out << endl;
		out << usandoLampara;
		out << endl;
		out << lamparaDesaparecida;
		out << endl;
		out << tiempoUsoLampara;
		out << endl;
		out << cambioEstadoLampara;
		out << endl;
		out << cntTiempoAOscuras;
		out << endl;
		out << cntLeeLibroSinGuantes;
		out << endl;
		out << pergaminoGuardado;
		out << endl;
		out << numeroAleatorio;
		out << endl;
		out << hayMovimiento;
		out << endl;
		out << cntMovimiento;
		out << endl;
		out << numPersonajeCamara;
		out << endl;
		out << opcionPersonajeCamara;
		out << endl;


	for (int i = 0; i < Juego::numSprites; i++){
		out << elJuego->sprites[i]->esVisible;
		out << endl;
	}
	// obtiene los personajes del juego
	guillermo = (Guillermo *)elJuego->personajes[0];
	adso = (Adso *)elJuego->personajes[1];
	malaquias = (Malaquias *)elJuego->personajes[2];
	abad = (Abad *)elJuego->personajes[3];
	berengario = (Berengario *)elJuego->personajes[4];
	severino = (Severino *)elJuego->personajes[5];
	jorge = (Jorge *)elJuego->personajes[6];
	bernardo = (Bernardo *)elJuego->personajes[7];

	// recorre los personajes e inicia sus caracter�sticas comunes
	for (int i = 0; i < Juego::numPersonajes; i++){
		Personaje *pers = elJuego->personajes[i];
		out << pers->estado;
		out << endl;
		out << pers->contadorAnimacion;
		out << endl;
		out << pers->bajando;
		out << endl;
		out << pers->orientacion;
		out << endl;
		out << pers->enDesnivel;
		out << endl;
		out << pers->giradoEnDesnivel;
		out << endl;
		out << pers->flipX;
		out << endl;
		out << pers->despFlipX;
		out << endl;
		out << pers->despX;
		out << endl;
		out << pers->despY;
		out << endl;
		out << pers->valorPosicion;
		out << endl;
		out << pers->puedeQuitarObjetos;
		out << endl;
		out << pers->objetos;
		out << endl;
		out << pers->mascaraObjetos;
		out << endl;
		out << pers->contadorObjetos;
		out << endl;
		out << pers->permisosPuertas;
		out << endl;
		out << pers->numFotogramas; //?es necesario guardar esto??
		out << endl;
		out << pers->orientacion; 
		out << endl;
		out << pers->posX; 
		out << endl;
		out << pers->posY; 
		out << endl;
		out << pers->altura; 
		out << endl;

		if (pers == guillermo) continue;

		PersonajeConIA *persIA = (PersonajeConIA *)elJuego->personajes[i];

		/* parece que esto no se graba bien en ascii,
		 * voy a quitarlo y luego a probar a grabar en binario
		// ?? a lo mejor aqui no hace falta grabar nada, y al cargar
		// es suficiente con poner pensarNuevoMovimiento a true
		out << persIA->numBitAcciones;
		out << endl;
		out << persIA->pensarNuevoMovimiento;
		out << endl;
		out << persIA->posAcciones;
		out << endl;
		for (int j=0;j<sizeof(persIA->bufAcciones);j++)
		{
			out << persIA->bufAcciones[j];
			out << endl;
		}*/

		//?lugares??

		out << persIA->mascarasPuertasBusqueda; 
		out << endl;
		out << persIA->aDondeVa; 
		out << endl;
		out << persIA->aDondeHaLlegado; 
		out << endl;
	}

	// guillermo
	out << guillermo->incrPosY;
		out << endl;

	// adso
	out << adso->oldEstado;
		out << endl;
	out << adso->movimientosFrustados;
		out << endl;
	out << adso->cntParaDormir;
		out << endl;
	
	// malaqu�as
	out << malaquias->estaMuerto;
		out << endl;
	out << malaquias->estado2;
		out << endl;
		// vaya, esto es protected... 666
		// se solucionara cuando cada clase tenga
		// su operador << 
//	out << malaquias->contadorEnScriptorium;
//		out << endl;

	// el abad
	out << abad->contador;
		out << endl;
	out << abad->numFrase;
		out << endl;
	out << abad->guillermoBienColocado;
		out << endl;
	out << abad->lleganLosMonjes;
		out << endl;
	out << abad->guillermoHaCogidoElPergamino;
		out << endl;

	// berengario
//666	out << berengario->fijaCapucha;
//aqui habra que guardar otra cosa y luego llamar al fijaCapucha
	out << berengario->estado2;
		out << endl;
	out << berengario->estaVivo;
		out << endl;
	out << berengario->contadorPergamino;
		out << endl;

	// severino
	out << severino->estaVivo;
		out << endl;

	// jorge
	out << jorge->estaActivo;
		out << endl;
	out << jorge->contadorHuida;
		out << endl;

	// bernardo gui
	out << bernardo->estaEnLaAbadia;
		out << endl;

	Puerta **puertas = elJuego->puertas;

	for (int i = 0; i < Juego::numPuertas; i++){
		out <<  puertas[i]->identificador;
		out << endl;
		out <<  puertas[i]->estaAbierta;
		out << endl;
		out <<  puertas[i]->haciaDentro;
		out << endl;
		out <<  puertas[i]->estaFija;
		out << endl;
		out <<  puertas[i]->hayQueRedibujar;
		out << endl;
		// Parte EntidadJuego
		out <<  puertas[i]->orientacion;
		out << endl;
		out <<  puertas[i]->posX;
		out << endl;
		out <<  puertas[i]->posY;
		out << endl;
		out <<  puertas[i]->altura;
		out << endl;
	}

	Objeto **objetos = elJuego->objetos;

	for (int i = 0; i < Juego::numObjetos; i++){
		int j;

		out << objetos[i]->seEstaCogiendo;
		out << endl;
		out << objetos[i]->seHaCogido;
		out << endl;

//666		out << objetos[i]->personaje;
		for (	j = Juego::numPersonajes-1;
			( objetos[i]->personaje != elJuego->personajes[j] &&
			  j>=0 );
			j--) ;
		out << j;
		out << endl;

		// Parte EntidadJuego
		out << objetos[i]->orientacion;
		out << endl;
		out << objetos[i]->posX;
		out << endl;
		out << objetos[i]->posY;
		out << endl;
		out << objetos[i]->altura;
		out << endl;
//uhmm, como grabamos esto ...
	} 
		out << endl;
}

void Logica::load(ifstream &in)
{
		in >> dia;
		
		in >> momentoDia;
//666 falta esto
/* Hay que fijar la paleta segun el momento del dia
 ya que en el juego solo se hace al entrar en prima
 o en completas 
	// fija la paleta de noche
	elJuego->paleta->setGamePalette(3);
	// fija la paleta de d�a
	elJuego->paleta->setGamePalette(2);
	*/
		/* no va , porque en Marcador::muestraDiaYMomentoDia
		 * se vuelve a cambiar
		if (	momentoDia==COMPLETAS ||
			momentoDia==NOCHE )
		{
			elJuego->paleta->setGamePalette(3);
		}
		else
		{
			elJuego->paleta->setGamePalette(2);
		} */

		
		in >> duracionMomentoDia;
		
		in >> oldMomentoDia;
		
		in >> avanzarMomentoDia;
		
		in >> obsequium;
		
		in >> haFracasado;
		
		in >> investigacionCompleta;
		
		in >> bonus;
		
		in >> mascaraPuertas;
		
		in >> espejoCerrado;
		
		in >> numeroRomano;
		
		in >> despDatosAlturaEspejo;
		
		in >> despBloqueEspejo;
		
		in >> seAcabaLaNoche;
		
		in >> haAmanecido;
		
		in >> usandoLampara;
		
		in >> lamparaDesaparecida;
		
		in >> tiempoUsoLampara;
		
		in >> cambioEstadoLampara;
		
		in >> cntTiempoAOscuras;
		
		in >> cntLeeLibroSinGuantes;
		
		in >> pergaminoGuardado;
		
		in >> numeroAleatorio;
		
		in >> hayMovimiento;
		
		in >> cntMovimiento;
		
		in >> numPersonajeCamara;
		
		in >> opcionPersonajeCamara;
			
		
	for (int i = 0; i < Juego::numSprites; i++){
		in >> elJuego->sprites[i]->esVisible;
		
	}
	// obtiene los personajes del juego
	guillermo = (Guillermo *)elJuego->personajes[0];
	adso = (Adso *)elJuego->personajes[1];
	malaquias = (Malaquias *)elJuego->personajes[2];
	abad = (Abad *)elJuego->personajes[3];
	berengario = (Berengario *)elJuego->personajes[4];
	severino = (Severino *)elJuego->personajes[5];
	jorge = (Jorge *)elJuego->personajes[6];
	bernardo = (Bernardo *)elJuego->personajes[7];

	// recorre los personajes e inicia sus caracter�sticas comunes
	for (int i = 0; i < Juego::numPersonajes; i++){
		Personaje *pers = elJuego->personajes[i];
		in >> pers->estado;
		
		in >> pers->contadorAnimacion;
		
		in >> pers->bajando;
		
		in >> pers->orientacion;
		
		in >> pers->enDesnivel;
		
		in >> pers->giradoEnDesnivel;
		
		in >> pers->flipX;
		
		in >> pers->despFlipX;
		
		in >> pers->despX;
		
		in >> pers->despY;
		
		in >> pers->valorPosicion;
		
		in >> pers->puedeQuitarObjetos;
		
		in >> pers->objetos;
		
		in >> pers->mascaraObjetos;
		
		in >> pers->contadorObjetos;
		
		in >> pers->permisosPuertas;
		
		in >> pers->numFotogramas; //?es necesario guardar esto??
		
		in >> pers->orientacion; 
		
		in >> pers->posX; 
		
		in >> pers->posY; 
		
		in >> pers->altura; 
		

		if (pers == guillermo) continue;

		PersonajeConIA *persIA = (PersonajeConIA *)elJuego->personajes[i];
		// ?? a lo mejor aqui no hace falta grabar nada, y al cargar
		// es suficiente con poner pensarNuevoMovimiento a true
		/*
		in >> persIA->numBitAcciones;
		
		in >> persIA->pensarNuevoMovimiento;
		
		in >> persIA->posAcciones;
		
		for (int j=0;j<sizeof(persIA->bufAcciones);j++)
		{
			in >> persIA->bufAcciones[j];
			
		} */
		persIA->pensarNuevoMovimiento=true;

		//?lugares??

		in >> persIA->mascarasPuertasBusqueda; 
		
		in >> persIA->aDondeVa; 
		
		in >> persIA->aDondeHaLlegado; 
		
	}

	// guillermo
	in >> guillermo->incrPosY;
		

	// adso
	in >> adso->oldEstado;
		
	in >> adso->movimientosFrustados;
		
	in >> adso->cntParaDormir;
		
	
	// malaqu�as
	in >> malaquias->estaMuerto;
		
	in >> malaquias->estado2;
		
		// vaya, esto es protected... 666
//	in >> malaquias->contadorEnScriptorium;
		

	// el abad
	in >> abad->contador;
		
	in >> abad->numFrase;
		
	in >> abad->guillermoBienColocado;
		
	in >> abad->lleganLosMonjes;
		
	in >> abad->guillermoHaCogidoElPergamino;
		

	// berengario
//666	in >> berengario->fijaCapucha;
//aqui habra que guardar otra cosa y luego llamar al fijaCapucha
	in >> berengario->estado2;
		
	in >> berengario->estaVivo;
		
	in >> berengario->contadorPergamino;
		

	// severino
	in >> severino->estaVivo;
		

	// jorge
	in >> jorge->estaActivo;
		
	in >> jorge->contadorHuida;
		

	// bernardo gui
	in >> bernardo->estaEnLaAbadia;
		

	Puerta **puertas = elJuego->puertas;

	for (int i = 0; i < Juego::numPuertas; i++){
		in >>  puertas[i]->identificador;
		
		in >>  puertas[i]->estaAbierta;
		
		in >>  puertas[i]->haciaDentro;
		
		in >>  puertas[i]->estaFija;
		
		in >>  puertas[i]->hayQueRedibujar;
		
		// Parte EntidadJuego
		in >>  puertas[i]->orientacion;
		
		in >>  puertas[i]->posX;
		
		in >>  puertas[i]->posY;
		
		in >>  puertas[i]->altura;
		
	}

	Objeto **objetos = elJuego->objetos;

	for (int i = 0; i < Juego::numObjetos; i++){
		int tmp;

		in >> objetos[i]->seEstaCogiendo;
		
		in >> objetos[i]->seHaCogido;
		
//666		in >> objetos[i]->personaje;
		in >> tmp;
		if ( tmp==-1 )
		{
			objetos[i]->personaje = NULL;
		}
		else
		{
			objetos[i]->personaje=elJuego->personajes[tmp];
		}
		
		// Parte EntidadJuego
		in >> objetos[i]->orientacion;
		
		in >> objetos[i]->posX;
		
		in >> objetos[i]->posY;
		
		in >> objetos[i]->altura;
		
//uhmm, como grabamos esto ...
	} 
		
}	
