#include "aspch.h"
#include "Font.h"

#undef INFINITE
#include "msdf-atlas-gen.h"

namespace Astan
{
	Font::Font(const std::filesystem::path& filepath)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (ft)
		{
			std::string fileString = filepath.string();
			msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
			if (font)
			{
				msdfgen::Shape shape;
				if (msdfgen::loadGlyph(shape, font, 'A'))
				{
					shape.normalize();
					edgeColoringSimple(shape, 3.0);
					msdfgen::Bitmap<float, 3> msdf(32, 32);
					generateMSDF(msdf, shape, 4.0, 1.0,msdfgen::Vector2(4.0, 4.0));
					savePng(msdf, "output.png");
				}
				destroyFont(font);
			}
			deinitializeFreetype(ft);
		}
	}
}