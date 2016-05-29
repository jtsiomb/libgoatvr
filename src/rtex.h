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
#ifndef RENDER_TEXTURE_H_
#define RENDER_TEXTURE_H_

namespace goatvr {

class RenderTexture {
public:
	unsigned int tex;
	int width, height;
	int tex_width, tex_height;
	int eye_xoffs[2], eye_yoffs[2];
	int eye_width[2], eye_height[2];
	float fbscale;

	RenderTexture();

	void update(int xsz, int ysz);
};

}	// namespace goatvr

#endif	// RENDER_TEXTURE_H_
