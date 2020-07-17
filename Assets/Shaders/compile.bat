"..\..\Tools\DXC\bin\dxc.exe" -spirv -T vs_6_4 -E mainVS unlit.hlsl -Fo unlit.vert.spv -fvk-use-scalar-layout
"..\..\Tools\DXC\bin\dxc.exe" -spirv -T ps_6_4 -E mainPS unlit.hlsl -Fo unlit.frag.spv -fvk-use-scalar-layout

"..\..\Tools\DXC\bin\dxc.exe" -spirv -T vs_6_4 -E mainVS imgui.hlsl -Fo imgui.vert.spv -fvk-use-scalar-layout
"..\..\Tools\DXC\bin\dxc.exe" -spirv -T ps_6_4 -E mainPS imgui.hlsl -Fo imgui.frag.spv -fvk-use-scalar-layout