in vec2 v_uv;

uniform sampler2D u_world_color;


void main()
{
    vec2 uv = v_uv;

    vec4 gray_filter = texture(u_world_color, uv);

    gray_filter.xyz = vec3((gray_filter.x + gray_filter.y + gray_filter.z)/3.0);

    gl_FragColor = gray_filter;
}



