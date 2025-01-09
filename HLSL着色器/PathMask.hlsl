#define D2D_INPUT_COUNT 1           // The pixel shader takes 1 input texture.
#define D2D_INPUT0_COMPLEX          // The first input is sampled in a complex manner: to calculate the output of a pixel,
                                    // the shader samples more than just the corresponding input coordinate.
#define D2D_REQUIRES_SCENE_POSITION // The pixel shader requires the SCENE_POSITION input.

// Note that the custom build step must provide the correct path to find d2d1effecthelpers.hlsli when calling fxc.exe.
#include "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\d2d1effecthelpers.hlsli"
D2D_PS_ENTRY(main)
{
    float4 input = D2DGetInput(0);

    return input;
}