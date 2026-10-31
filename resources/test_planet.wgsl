struct VertexIn {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) texcoord: vec2f,
}

struct VertexOut {
    @builtin(position) position: vec4f,
    @location(0) world_pos: vec3f,
    @location(1) normal: vec3f,
    @location(2) texcoord: vec2f,
}

struct Matrices {
    model: mat4x4f,
    viewProjection: mat4x4f,
}

@group(0) @binding(0) var<uniform> matrices: Matrices;

@vertex
fn vertexMain(in: VertexIn) -> VertexOut {
    let world = matrices.model * vec4f(in.position, 1.0);

    var out: VertexOut;
    out.position  = matrices.viewProjection * world;
    out.world_pos = world.xyz;
    out.normal    = normalize((matrices.model * vec4f(in.position, 0.0)).xyz);
    out.texcoord  = in.texcoord;
    
    return out;
}

@fragment
fn fragmentMain(in: VertexOut) -> @location(0) vec4f {
    let normal = normalize(in.normal);

    let light_pos = vec3f(3.0, 4.0, 2.0);
    let to_light = light_pos - in.world_pos;

    let ndotl = max(0.0, dot(normal, normalize(to_light)));

    let ambient = 0.1;
    let albedo = vec3f(1.0);
    let color = albedo * (ambient + ndotl);

    return vec4f(color, 1.0);
}