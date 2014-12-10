#pragma version(1)
#pragma rs java_package_name(android.renderscript.cts)

typedef struct atan2_f32_in {
    float first;
    float second;
} input;

void atan2_f32_1(const input* in, float* out){
    *out = atan2(in->first, in->second);
}

typedef struct atan2_f32_2_in {
    float2 first;
    float2 second;
} input2;

void atan2_f32_2(const input2* in, float2* out){
    *out = atan2(in->first, in->second);
}

typedef struct atan2_f32_3_in {
    float3 first;
    float3 second;
} input3;

void atan2_f32_3(const input3* in, float3* out){
    *out = atan2(in->first, in->second);
}

typedef struct atan2_f32_4_in {
    float4 first;
    float4 second;
} input4;

void atan2_f32_4(const input4* in, float4* out){
    *out = atan2(in->first, in->second);
}
