#version 150
#extension GL_ARB_gpu_shader_fp64 : require

in vec4 piglit_vertex;
out C {
    dvec4 fs_color;
};

void main() {
  gl_Position = piglit_vertex;
  fs_color = dvec4((piglit_vertex.x*2*piglit_vertex.y+3*piglit_vertex.z)*4.0, (piglit_vertex.x*20*piglit_vertex.y+3*piglit_vertex.z)*8.0, (piglit_vertex.x*2*piglit_vertex.y+3*piglit_vertex.z)*14.0, 1.0);
}
