#extension GL_ARB_explicit_attrib_location: require

layout(location = 2) attribute vec3 color;

vec3 function(void)
{
	return color * 3.0;
}
