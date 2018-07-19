#pragma once

#include "base.h"
#include "curve.h"

// Najpierw chcemy uzyskać proste efekty:
// - emiter który jest w stanie emitować ileś tam cząsteczek na sekundę
// - cząsteczka która może zmieniać kolor, ma kierunek lotu, zmieniającą się alfę i czas życia
// - mamy też cały system cząstek (najczęściej w ramach pojedyńczego kafla)
//   - może być parametryzowany: parametry zmieniają trochę konfigurację danego systemu
//
// Szablony emiterów i cząsteczek umożliwiają robienie generycznych efektów (z pomocą ścieżek)
// ale chyba chcielibyśmy też mieć możliwość pisania specjalnych klas do konkretnych efektów
//
// Na początku możemy zrobić generyczny system (podobny do tego co mieliśmy w XLIB) a później
// jeśli generyczny system nie wystarczy, to dodać specjalizacje;
//
// Nad specjalizacjami zastanawiamy się w kontekście konkretnego efektu jaki chcemy uzyskać

// Defines behaviour of a given particle class
struct ParticleDef {
	Curve<float> life, alpha;
	Curve<float3> color;
	string texture_name;
};

struct EmitterDef {
	Curve<float> frequency, strength;
	Curve<float> spread; // in radians
	Curve<float3> direction; // makes sense only if spread < PI * 2

	float emission_time;
};
