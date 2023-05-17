layout(location = 0) out vec2 uv;

void main() 
{
    vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
    uv = gl_Position.xy * 0.5 + 0.5;
}