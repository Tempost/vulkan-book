┌ ┘ ┐ └
─ │ ┤ ├
# Graphics pipeline
  ┌─────────────────────┐
  │ Vertex/Index buffer │
  └─────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Input Assembler *                                                          │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Collects raw vertex data from specified buffers, use index buffer to       │
  │ repeat elements without duplication.                                       │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Vertex Shader                                                              │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Will run for every vertex and applies transformations to go from model     │
  │ space into screen space. Passes per-vertex data down pipeline.             │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Tessellation                                                               │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Subdivide Geometry based on rules to increase quality of mesh. IE make the │
  │ surface of a brick texture look less flat.                                 │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Geometry Shader                                                            │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Runs for every primative (triangle, line, point). Destructive and addative.│
  │ Performance is poor on everything except Intel integrated GPUS. SKIPPABLE! │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Rasterization *                                                            │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Turns primatives into Fragments, or pixel elements. Fragments outside of   │
  │ screen space are discared. Data from vertex shader is interpolated across  │
  │ the fragments. Fragments behind another fragment are also discard.         │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Fragment Shader                                                            │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Runs for every fragment that lives through Rasterization stage, chooses    │
  │ framebuffers to write fragments to using data from vertex shader step      │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌────────────────────────────────────────────────────────────────────────────┐
  │ Color Blender *                                                            │
  ├────────────────────────────────────────────────────────────────────────────┤
  │ Mix colors of pixels that line up with each other, uses fragments.         │
  └────────────────────────────────────────────────────────────────────────────┘
       ↓
  ┌─────────────┐
  │ Framebuffer │
  └─────────────┘

  Titles with * are fixed-function stages, they are tweakable via parameters but
  are mostly predefined by vulkan.

  Non starred titles are programmable, we can upload our OWN code to the GPU to
  define how the step will run and behave.
