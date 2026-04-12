## PBR material extension.

The spec can be found in either

https://benhouston3d.com/blog/extended-wavefront-obj-mtl-for-pbr/

or Internet Archive: https://web.archive.org/web/20230210121526/http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr

* Kd/map_Kd (base/diffuse) // reuse
* Ks/map_Ks (specular) // reuse
* d or Tr (opacity) // reuse
* map_d/map_Tr (opacitymap) // reuse
* Tf (translucency) // reuse
* bump/-bm (bump map) // reuse
* disp (displacement map) // reuse

PBR material parameters as defined by the Disney PBR.

* Pr/map_Pr (roughness) // new
* Pm/map_Pm (metallic) // new
* Ps/map_Ps (sheen) // new
* Pc (clearcoat thickness) // new
* Pcr (clearcoat roughness) // new
* Ke/map_Ke (emissive) // new
* aniso (anisotropy) // new
* anisor (anisotropy rotation) // new
* norm (normal map) // new

EoL.
