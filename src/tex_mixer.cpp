#include "fwk/gfx/texture.h"
#include "fwk/sys/stream.h"
#include "fwk/sys/assert.h"

using namespace fwk;

// This program creates textures suitable for blendAdd mode
int main(int argc, char **argv) {
	ASSERT(argc == 4);

	Texture colorAlphaTex, burnTex;
	Loader(argv[1]) >> colorAlphaTex;
	Loader(argv[2]) >> burnTex;

	ASSERT_EQ(colorAlphaTex.size(), burnTex.size());

	Texture resultTex(colorAlphaTex.size());

	for(int y = 0; y < resultTex.height(); y++)
		for(int x = 0; x < resultTex.width(); x++) {
			FColor colorAlpha(colorAlphaTex(x, y));
			float alpha = colorAlpha.a;
			float burn = 1.0f;//float(burnTex(x, y).a) * (1.0f / 255.0f);

			float3 newCol = colorAlpha.rgb() * clamp(alpha * burn + 1.0f - burn, 0.0f, 1.0f);
			float newAlpha = alpha * burn;
			//print("%:% ", newCol, newAlpha);
			resultTex(x, y) = IColor(FColor(newCol, newAlpha));
		}
	Saver(argv[3]) << resultTex;

	return 0;

}
