#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
    vec4 c = texture(texture0, fragTexCoord);

    // detect "whiteness" — white has all channels close to 1
    float whiteness = min(c.r, min(c.g, c.b)); // 1.0 for pure white, 0.0 for pure color

    // invert only white/near-white pixels, leave colored glow alone
    vec3 inverted = vec3(1.0 - c.r, 1.0 - c.g, 1.0 - c.b);
    vec3 result   = mix(c.rgb, inverted, whiteness);

    finalColor = vec4(result, c.a);
}