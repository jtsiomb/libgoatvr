#ifndef RENDER_TEXTURE_H_
#define RENDER_TEXTURE_H_

namespace goatvr {

struct RenderTexture {
	unsigned int tex;
	int width, height;
	int tex_width, tex_height;
	int eye_xoffs[2], eye_yoffs[2];
	int eye_width[2], eye_height[2];
};

}	// namespace goatvr

#endif	// RENDER_TEXTURE_H_