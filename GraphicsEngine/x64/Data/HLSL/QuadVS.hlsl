#include "Common.hlsli"

VS_OUTPUT main(Quad input)
{
    VS_OUTPUT test;
        
    test.pos = input.pos;
    //test.pos = input.pos;
    //test.posWorld = input.pos;
    test.tex = input.tex;
    
    return test;
}
