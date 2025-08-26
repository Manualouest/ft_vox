#version 330 core
out vec4 FragColor;

uniform sampler2D textureAtlas;

in vec2 fragPos;

vec4	getBlockTexture(int ID)
{
	vec2	baseUV = fragPos;
	baseUV = fract(baseUV);

	int row = 15 - (ID / 16);;
    int col = ID % 16;

    vec2 cellSize = vec2(16.0 / 256.0);
    vec2 atlasOffset = vec2(col, row) * cellSize;
    vec2 atlasUV = atlasOffset + baseUV * cellSize;

	return (texture(textureAtlas, atlasUV));
}

uniform int blockID;

void main()
{
    vec4 texColor = getBlockTexture(blockID);
    if (texColor.r == 1 && texColor.g == 0 && texColor.b == 1)
		discard ;
    FragColor = texColor;
}
