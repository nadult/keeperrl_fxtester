#include "fx_color.h"

#include "renderer.h"

// TODO: move Color to separate file
Color::Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) : SDL_Color{r, g, b, a} {}

namespace fx {

FColor::FColor(Color col) { *this = FColor(col.r, col.g, col.b, col.a) * (1.0f / 255.0f); }
FColor::FColor(IColor col) { *this = FColor(col.r, col.g, col.b, col.a) * (1.0f / 255.0f); }

FColor::operator IColor() const {
	using u8 = unsigned char;
	return {(u8)clamp(r * 255.0f, 0.0f, 255.0f), (u8)clamp(g * 255.0f, 0.0f, 255.0f),
			(u8)clamp(b * 255.0f, 0.0f, 255.0f), (u8)clamp(a * 255.0f, 0.0f, 255.0f)};
}

FColor::operator Color() const {
	using u8 = SDL::Uint8;
	return {(u8)clamp(r * 255.0f, 0.0f, 255.0f), (u8)clamp(g * 255.0f, 0.0f, 255.0f),
			(u8)clamp(b * 255.0f, 0.0f, 255.0f), (u8)clamp(a * 255.0f, 0.0f, 255.0f)};
}

IColor::IColor(Color col) : r(col.r), g(col.g), b(col.b), a(col.a) {}
IColor::operator Color() const { return {r, g, b, a}; }
}
