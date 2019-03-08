// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_separate_shader_objects GL_ARB_gpu_shader_fp64
// check_link: true
// [end config]
//
// From the ARB_separate_shader_objects spec v.25:
//
//   "   * An output variable is considered to match an input variable
//         in the subequent shader if:
//
//         * the two variables match in name, type, and qualification;
//           or
//
//         * the two variables are declared with the same location
//           layout qualifier and match in type and qualification."
//
//   ...
//
//   " A program will fail to link if any two non-vertex shader input
//     variables are assigned to the same location."
#version 150
#extension GL_ARB_separate_shader_objects : require
#extension GL_ARB_gpu_shader_fp64 : require

uniform int i;

layout(location = 0) flat in dvec4 in1;
layout(location = 0) flat in dvec4 in2;

out vec4 color;

void main()
{
	if (i == 0)
		color = vec4(in1);
	else
		color = vec4(in2);
}
