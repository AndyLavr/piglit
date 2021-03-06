static const char passthrough_vertex_shader_source_spv[] =
	"; SPIR-V\n"
	"; Version: 1.0\n"
	"; Generator: Khronos Glslang Reference Front End; 6\n"
	"; Bound: 24\n"
	"; Schema: 0\n"
	"               OpCapability Shader\n"
	"          %1 = OpExtInstImport \"GLSL.std.450\"\n"
	"               OpMemoryModel Logical GLSL450\n"
	"               OpEntryPoint Vertex %main \"main\" %_ %piglit_vertex %gl_VertexID %gl_InstanceID\n"
	"               OpSource GLSL 450\n"
	"               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position\n"
	"               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize\n"
	"               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance\n"
	"               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance\n"
	"               OpDecorate %gl_PerVertex Block\n"
	"               OpDecorate %piglit_vertex Location 0\n"
	"               OpDecorate %gl_VertexID BuiltIn VertexId\n"
	"               OpDecorate %gl_InstanceID BuiltIn InstanceId\n"
	"       %void = OpTypeVoid\n"
	"          %3 = OpTypeFunction %void\n"
	"      %float = OpTypeFloat 32\n"
	"    %v4float = OpTypeVector %float 4\n"
	"       %uint = OpTypeInt 32 0\n"
	"     %uint_1 = OpConstant %uint 1\n"
	"%_arr_float_uint_1 = OpTypeArray %float %uint_1\n"
	"%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1\n"
	"%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex\n"
	"          %_ = OpVariable %_ptr_Output_gl_PerVertex Output\n"
	"        %int = OpTypeInt 32 1\n"
	"      %int_0 = OpConstant %int 0\n"
	"%_ptr_Input_v4float = OpTypePointer Input %v4float\n"
	"%piglit_vertex = OpVariable %_ptr_Input_v4float Input\n"
	"%_ptr_Output_v4float = OpTypePointer Output %v4float\n"
	"%_ptr_Input_int = OpTypePointer Input %int\n"
	"%gl_VertexID = OpVariable %_ptr_Input_int Input\n"
	"%gl_InstanceID = OpVariable %_ptr_Input_int Input\n"
	"       %main = OpFunction %void None %3\n"
	"          %5 = OpLabel\n"
	"         %18 = OpLoad %v4float %piglit_vertex\n"
	"         %20 = OpAccessChain %_ptr_Output_v4float %_ %int_0\n"
	"               OpStore %20 %18\n"
	"               OpReturn\n"
	"               OpFunctionEnd\n"
	;
