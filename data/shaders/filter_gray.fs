varying vec4 normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_world_color
uniform float u_time;
uniform float u_tex_tiling;

void main()
{
    vec2 uv = v_uv;
    vec3 N = normalize(v_normal);

    
    vec4 gray_filter = texture2D(u_world_color, uv * 30.0);

    float f = smoothstep(10.0, 20.0, v_world_position.y);
    vec4 color = mix(gray_filter, u_color, f);

    float slope = 1.0-smoothstep(0.75,0.8,N.y);
    color = mix(color, gray_filter, slope);

    gl_FragColor = color * u_color;
}



