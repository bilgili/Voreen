#include "mod_phong.frag"

void main()
{
    vec3 color = gl_Color.xyz;
    float alpha = gl_Color.w;

    if (usePhongShading_ == true) {
        vec3 phongTerms = (useForLines_ == false) ? phongShading() : phongShadingForLines();
        gl_FragColor = vec4((color * ka_) + (color * kd_ * phongTerms.y) + (color * ks_ * phongTerms.z), alpha);
    } else
        gl_FragColor = vec4(color, alpha);
}
