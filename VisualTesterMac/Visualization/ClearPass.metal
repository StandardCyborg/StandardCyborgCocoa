//
//  ClearPass.metal
//  VisualTesterMac
//
//  Created by Ricky Reusser on 9/13/18.
//

#include <metal_stdlib>
using namespace metal;

vertex float4 ClearPassVertex () {
    return float4(0);
}

fragment float4 ClearPassFragment () {
    return float4(0);
}
