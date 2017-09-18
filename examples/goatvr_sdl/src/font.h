#ifndef FONT_H_
#define FONT_H_

struct glyph {
  int c;
  int x, y, width, height, orgx, orgy;
  int advance;
};

struct glyph *lookup_glyph(int c);
unsigned int font_texture(void);

float glyph_width(int c);
float string_width(const char *s);
void draw_string(const char *s, int align);

#endif	/* FONT_H_ */
