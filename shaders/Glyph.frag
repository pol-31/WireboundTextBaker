#version 460 core

in vec2 texcoord;

uniform sampler2D tex_bitmap_ascii;
uniform sampler2D tex_bitmap_loc;
uniform sampler2D tex_ttf_glyph;
uniform sampler2D tex_temp;

uniform int tex_type;

out vec4 FragColor;

int tex_ascii_type = 0;
int tex_loc_type = 1;
int tex_ttf_glyph_type = 2;
int tex_temp_type = 3;

void main() {
    if (tex_type == tex_ascii_type) {
        FragColor = texture(tex_bitmap_ascii, texcoord);
    } else if (tex_type == tex_loc_type) {
        FragColor = texture(tex_bitmap_loc, texcoord);
    } else if (tex_type == tex_ttf_glyph_type) {
        FragColor = texture(tex_ttf_glyph, texcoord);
    } else if (tex_type == tex_temp_type) {
        FragColor = texture(tex_temp, texcoord);
    } else {
        FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
//    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}