// Section 4.3.8.1 (Input Layout Qualifiers) of the GLSL 1.50 spec
// includes the following examples of compile-time errors:
//
//   // code sequence within one shader...
//   in vec4 Color1[];    // size unknown
//   ...Color1.length()...// illegal, length() unknown
//   in vec4 Color2[2];   // size is 2
//   ...Color1.length()...// illegal, Color1 still has no size
//   in vec4 Color3[3];   // illegal, input sizes are inconsistent
//   layout(lines) in;    // legal, input size is 2, matching
//   in vec4 Color4[3];   // illegal, contradicts layout
//   ...Color1.length()...// legal, length() is 2, Color1 sized by layout()
//   layout(lines) in;    // legal, matches other layout() declaration
//   layout(triangles) in;// illegal, does not match earlier layout() declaration
//
// This test verifies that when a layout declaration causes a
// previously unsized geometry shader input array to become sized, if
// an intervening usage of that input array was consistent with the
// new size, there is no error.  This test verifies the case for input
// interface blocks.
//
// [config]
// expect_result: pass
// glsl_version: 1.50
// check_link: false
// [end config]

#version 150

in blk {
  vec4 Color;
} inst[];

vec4 foo()
{
  return inst[2].Color;
}

layout(triangles) in;
