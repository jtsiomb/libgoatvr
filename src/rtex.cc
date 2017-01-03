/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include "opengl.h"
#include "rtex.h"
#include "goatvr_impl.h"

using namespace goatvr;

RenderTexture::RenderTexture()
{
	tex = 0;
	width = height = 0;
	tex_width = tex_height = 0;

	for(int i=0; i<2; i++) {
		eye_xoffs[i] = eye_yoffs[i] = 0;
		eye_width[i] = eye_height[i] = 0;
		fbscale = 1.0f;
	}
}

void RenderTexture::update(int xsz, int ysz)
{
	width = xsz;
	height = ysz;

	int new_tex_width = next_pow2(xsz);
	int new_tex_height = next_pow2(ysz);

	if(!tex) {
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		tex_width = -1;	// invalidate width to force a tex rebuild
	}

	if(tex_width != new_tex_width || tex_height != new_tex_height) {
		tex_width = new_tex_width;
		tex_height = new_tex_height;

		printf("goatvr: creating %dx%d texture for %dx%d framebuffer\n", tex_width, tex_height, xsz, ysz);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
}
