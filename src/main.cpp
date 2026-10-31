#include <wgpu_app.hpp>
#include <file_utils.hpp>
#include <obj_loader.hpp>
#include <math/math.hpp>

#include <webgpu/webgpu.h>

#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <unordered_set>

#include "mesh_generators.hpp"
#include "camera.hpp"


static std::filesystem::path const resourcesRoot = RESOURCES_ROOT;

typedef struct Transform {
    math::vector3f position;
    math::vector3f rotation;
    math::vector3f scale;

    
} Transform;

WGPUShaderModule createShaderModule(
    WGPUDevice device,
    std::filesystem::path const & path
) {
    auto const source = loadFile(path);

    WGPUShaderSourceWGSL shaderSourceWGSL = WGPU_SHADER_SOURCE_WGSL_INIT;
    shaderSourceWGSL.code = {source.data(), source.size()};

    WGPUShaderModuleDescriptor shaderModuleDescriptor = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    shaderModuleDescriptor.nextInChain = &shaderSourceWGSL.chain;

    return wgpuDeviceCreateShaderModule(device, &shaderModuleDescriptor);
}

template <typename T>
WGPUBuffer loadBuffer(
    WGPUDevice device,
    WGPUQueue queue,
    std::vector<T> const & data,
    WGPUBufferUsage usage
) {
    WGPUBufferDescriptor bufferDescriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
    bufferDescriptor.size = data.size() * sizeof(T);
    bufferDescriptor.usage = usage | WGPUBufferUsage_CopyDst;

    auto buffer = wgpuDeviceCreateBuffer(device, &bufferDescriptor);
    wgpuQueueWriteBuffer(queue, buffer, 0, data.data(), data.size() * sizeof(T));
    return buffer;
}

WGPUBindGroupLayout createTestBindGroupLayout(
    WGPUDevice device
) {
    std::vector<WGPUBindGroupLayoutEntry> bindGroupLayoutEntry(3, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT);
    
    bindGroupLayoutEntry[0].binding = 0;
    bindGroupLayoutEntry[0].visibility = WGPUShaderStage_Vertex;
    bindGroupLayoutEntry[0].buffer = WGPU_BUFFER_BINDING_LAYOUT_INIT;
    bindGroupLayoutEntry[0].buffer.type = WGPUBufferBindingType_Uniform;
    /*
    bindGroupLayoutEntry[1].binding = 1;
    bindGroupLayoutEntry[1].visibility = WGPUShaderStage_Fragment;
    bindGroupLayoutEntry[1].texture = WGPU_TEXTURE_BINDING_LAYOUT_INIT;
    bindGroupLayoutEntry[1].texture.viewDimension = WGPUTextureViewDimension_2D;
    bindGroupLayoutEntry[1].texture.sampleType = WGPUTextureSampleType_Float;

    bindGroupLayoutEntry[2].binding = 2;
    bindGroupLayoutEntry[2].visibility = WGPUShaderStage_Fragment;
    bindGroupLayoutEntry[2].sampler = WGPU_SAMPLER_BINDING_LAYOUT_INIT;
    bindGroupLayoutEntry[2].sampler.type = WGPUSamplerBindingType_Filtering;
    */
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    bindGroupLayoutDescriptor.label = {"Test Bind Group Layout", WGPU_STRLEN};
    bindGroupLayoutDescriptor.entryCount = 1;
    bindGroupLayoutDescriptor.entries = bindGroupLayoutEntry.data();

    auto bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDescriptor);
    return bindGroupLayout;
}

WGPUBindGroup createTestBindGroup(
    WGPUDevice device,
    WGPUBuffer buffer,
    WGPUTextureView textureView,
    WGPUSampler sampler,
    WGPUBindGroupLayout bindGroupLayout
) {
    std::vector<WGPUBindGroupEntry> bindGroupEntry(1, WGPU_BIND_GROUP_ENTRY_INIT);
    bindGroupEntry[0].binding = 0;
    bindGroupEntry[0].buffer = buffer;
    /*
    bindGroupEntry[1].binding = 1;
    bindGroupEntry[1].textureView = textureView;
    bindGroupEntry[2].binding = 2;
    bindGroupEntry[2].sampler = sampler;
    */
    WGPUBindGroupDescriptor bindGroupDescriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    bindGroupDescriptor.label = {"Test Bind Group", WGPU_STRLEN};
    bindGroupDescriptor.layout = bindGroupLayout;
    bindGroupDescriptor.entryCount = 1;
    bindGroupDescriptor.entries = bindGroupEntry.data();
    auto bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDescriptor);
    return bindGroup;
}

WGPURenderPipeline createTestPipeline(
    WGPUDevice device,
    WGPUShaderModule shaderModule,
    WGPUTextureFormat surfaceFormat,
    WGPUBindGroupLayout const * bindGroupLayouts
) {   
    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pipelineLayoutDescriptor.label = {"Test Pipeline Layout", WGPU_STRLEN};
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts;

    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDescriptor);

    WGPUColorTargetState colorTargetState = WGPU_COLOR_TARGET_STATE_INIT;
    colorTargetState.format = surfaceFormat;
    colorTargetState.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragmentState = WGPU_FRAGMENT_STATE_INIT;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = {"fragmentMain", WGPU_STRLEN};
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    std::vector<WGPUVertexAttribute> attributes(3, WGPU_VERTEX_ATTRIBUTE_INIT);
    attributes[0].offset = 0;
    attributes[0].shaderLocation = 0;
    attributes[0].format = WGPUVertexFormat_Float32x3;
    attributes[1].offset = 12;
    attributes[1].shaderLocation = 1;
    attributes[1].format = WGPUVertexFormat_Float32x3;
    attributes[2].offset = 24;
    attributes[2].shaderLocation = 2;
    attributes[2].format = WGPUVertexFormat_Float32x2;

    WGPUVertexBufferLayout vertexBufferLayout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
    vertexBufferLayout.arrayStride = sizeof(ObjVertex);
    vertexBufferLayout.attributeCount = 3;
    vertexBufferLayout.attributes = attributes.data();
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

    WGPUDepthStencilState depthStencilState = WGPU_DEPTH_STENCIL_STATE_INIT;
    depthStencilState.format = WGPUTextureFormat_Depth24Plus;
    depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
    depthStencilState.depthCompare = WGPUCompareFunction_Less;

    WGPURenderPipelineDescriptor renderPipelineDescriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    renderPipelineDescriptor.layout = pipelineLayout;
    renderPipelineDescriptor.vertex.module = shaderModule;
    renderPipelineDescriptor.vertex.entryPoint = {"vertexMain", WGPU_STRLEN};
    renderPipelineDescriptor.vertex.buffers = &vertexBufferLayout;
    renderPipelineDescriptor.vertex.bufferCount = 1;
    renderPipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    renderPipelineDescriptor.primitive.cullMode = WGPUCullMode_Back;
    renderPipelineDescriptor.fragment = &fragmentState;
    renderPipelineDescriptor.depthStencil = &depthStencilState;

    WGPURenderPipeline renderPipeline = wgpuDeviceCreateRenderPipeline(device, &renderPipelineDescriptor);
    wgpuPipelineLayoutRelease(pipelineLayout);

    return renderPipeline;
}

math::matrix4f makeModelMatrix(math::vector3f const & pos, float scale) {
    math::matrix4f m = math::matrix4f::identity();
    m[0][0] = scale;
    m[1][1] = scale;
    m[2][2] = scale;
    m[0][3] = pos.x();
    m[1][3] = pos.y();
    m[2][3] = pos.z();
    return m;
}

int main() try {
    WgpuApp app("Planetary System WebGPU", 1280, 720, false);
    app.grab_mouse(true);

    WGPUShaderModule testShaderModule = createShaderModule(app.device(), resourcesRoot / "test_planet.wgsl");
    WGPUBindGroupLayout testBindGroupLayout = createTestBindGroupLayout(app.device());
    WGPURenderPipeline testRenderPipeline = createTestPipeline(app.device(), testShaderModule, app.surfaceFormat(), &testBindGroupLayout);

    ObjMesh uvSphere = generateUvSphere(16, 8);
    ObjMesh icoSphere = generateIcoSphere(2);

    WGPUBuffer uvVertexBuffer = loadBuffer(app.device(), app.queue(), uvSphere.vertices, WGPUBufferUsage_Vertex);
    WGPUBuffer uvIndexBuffer  = loadBuffer(app.device(), app.queue(), uvSphere.indices,  WGPUBufferUsage_Index);

    WGPUBuffer icoVertexBuffer = loadBuffer(app.device(), app.queue(), icoSphere.vertices, WGPUBufferUsage_Vertex);
    WGPUBuffer icoIndexBuffer  = loadBuffer(app.device(), app.queue(), icoSphere.indices,  WGPUBufferUsage_Index);

    std::array<WGPUBuffer, 2> uniformBuffers{};
    std::array<WGPUBindGroup, 2> bindGroups{};

    WGPUBufferDescriptor uniformBufferDescriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
    uniformBufferDescriptor.label = {"Uniform Buffer", WGPU_STRLEN};
    uniformBufferDescriptor.size = 2 * sizeof(math::matrix4f);
    uniformBufferDescriptor.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;

    uniformBuffers[0] = wgpuDeviceCreateBuffer(app.device(), &uniformBufferDescriptor);
    bindGroups[0] = createTestBindGroup(app.device(), uniformBuffers[0], nullptr, nullptr, testBindGroupLayout);

    uniformBuffers[1] = wgpuDeviceCreateBuffer(app.device(), &uniformBufferDescriptor);
    bindGroups[1] = createTestBindGroup(app.device(), uniformBuffers[1], nullptr, nullptr, testBindGroupLayout);
    
    struct SphereDesc {
        math::vector3f position;
        float scale;
    };
    std::array<SphereDesc, 2> spheres = {{
        { math::vector3f{-1.5f, 0.0f, 0.0f}, 1.0f },
        { math::vector3f{ 1.5f, 0.0f, 0.0f}, 0.6f },
    }};

    WGPUTexture     depthBuffer     = nullptr;
    WGPUTextureView depthBufferView = nullptr;

    auto lastFrameStart = std::chrono::high_resolution_clock::now();
    float time = 0.f;

    Transform cameraTransform;

    Camera camera;
    std::unordered_set<SDL_Keycode> keydown;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                app.resize(event.window.data1, event.window.data2);
                break;
            case SDL_EVENT_KEY_DOWN:
                keydown.insert(event.key.key);
                break;
            case SDL_EVENT_KEY_UP:
                keydown.erase(event.key.key);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                camera.onMouseMove(
                    static_cast<float>(event.motion.xrel),
                    static_cast<float>(event.motion.yrel)
                );
                break;
            }
        }

        std::optional<WGPUSurfaceTexture> surfaceTexture = app.beginFrame();
        if (!surfaceTexture) {
            continue;
        }

        if (!depthBuffer || wgpuTextureGetWidth(depthBuffer) != app.width() || wgpuTextureGetHeight(depthBuffer) != app.height()) {
            if (depthBufferView) wgpuTextureViewRelease(depthBufferView);
            if (depthBuffer)     wgpuTextureRelease(depthBuffer);

            WGPUTextureDescriptor depthBufferDescriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
            depthBufferDescriptor.usage = WGPUTextureUsage_RenderAttachment;
            depthBufferDescriptor.dimension = WGPUTextureDimension_2D;
            depthBufferDescriptor.size = {(std::uint32_t)app.width(), (std::uint32_t)app.height(), 1};
            depthBufferDescriptor.format = WGPUTextureFormat_Depth24Plus;
            depthBuffer = wgpuDeviceCreateTexture(app.device(), &depthBufferDescriptor);

            WGPUTextureViewDescriptor depthBufferViewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
            depthBufferViewDescriptor.format = WGPUTextureFormat_Depth24Plus;
            depthBufferViewDescriptor.dimension = WGPUTextureViewDimension_2D;
            depthBufferViewDescriptor.mipLevelCount = 1;
            depthBufferViewDescriptor.arrayLayerCount = 1;
            depthBufferViewDescriptor.aspect = WGPUTextureAspect_DepthOnly;
            depthBufferViewDescriptor.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TransientAttachment;
            depthBufferView = wgpuTextureCreateView(depthBuffer, &depthBufferViewDescriptor);
        }

        auto const now = std::chrono::high_resolution_clock::now();
        float const dt = std::chrono::duration<float>(now - lastFrameStart).count();
        time += dt;
        lastFrameStart = now;

        math::vector3f move{0.f, 0.f, 0.f};
        if (keydown.contains(SDLK_W)) move += camera.forward();
        if (keydown.contains(SDLK_S)) move -= camera.forward();
        if (keydown.contains(SDLK_A)) move -= camera.right();
        if (keydown.contains(SDLK_D)) move += camera.right();
        if (keydown.contains(SDLK_E)) move += math::cross(camera.right(), camera.forward());
        if (keydown.contains(SDLK_Q)) move -= math::cross(camera.right(), camera.forward());
        if (math::length_squared(move) > 0.f) {
            move = math::normalized(move);
        }

        camera.position += move * (camera.moveSpeed * dt);

        math::matrix4f const view = camera.view();
        math::matrix4f const projection = math::perspective(
            std::numbers::pi_v<float> / 3.f, 
            app.width() * 1.f / app.height(), 
            0.01f, 
            100.f
        );
        auto viewProjectionTranspose = math::transpose(projection * view);

        for (int i = 0; i < 2; ++i) {
            auto const modelTranspose = math::transpose(
                makeModelMatrix(spheres[i].position, spheres[i].scale)
            );
            wgpuQueueWriteBuffer(app.queue(), uniformBuffers[i], 0, &modelTranspose, sizeof(modelTranspose));
            wgpuQueueWriteBuffer(app.queue(), uniformBuffers[i], 64, &viewProjectionTranspose, sizeof(viewProjectionTranspose));
        }

        WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture->texture, nullptr);

        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(app.device(), nullptr);

        WGPURenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        colorAttachment.view = targetView;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = {0.0, 0.0, 0.0, 1.0};

        WGPURenderPassDepthStencilAttachment depthStencilAttachment = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
        depthStencilAttachment.view = depthBufferView;
        depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
        depthStencilAttachment.depthStoreOp = WGPUStoreOp_Discard;
        depthStencilAttachment.depthClearValue = 1.f;
        depthStencilAttachment.depthReadOnly = WGPU_FALSE;

        WGPURenderPassDescriptor renderPassDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        renderPassDescriptor.colorAttachmentCount = 1;
        renderPassDescriptor.colorAttachments = &colorAttachment;
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);

        wgpuRenderPassEncoderSetPipeline(renderPass, testRenderPipeline);

        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroups[0], 0, nullptr);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, uvVertexBuffer, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderSetIndexBuffer(renderPass, uvIndexBuffer, WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderDrawIndexed(renderPass, uvSphere.indices.size(), 1, 0, 0, 0);

        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroups[1], 0, nullptr);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, icoVertexBuffer, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderSetIndexBuffer(renderPass, icoIndexBuffer, WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderDrawIndexed(renderPass, icoSphere.indices.size(), 1, 0, 0, 0);

        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, nullptr);
        wgpuCommandEncoderRelease(encoder);

        wgpuQueueSubmit(app.queue(), 1, &commandBuffer);
        wgpuCommandBufferRelease(commandBuffer);

        wgpuSurfacePresent(app.surface());

        wgpuTextureViewRelease(targetView);
        wgpuTextureRelease(surfaceTexture->texture);
    }
    
    if (depthBufferView) wgpuTextureViewRelease(depthBufferView);
    if (depthBuffer)     wgpuTextureRelease(depthBuffer);

    for (int i = 0; i < 2; ++i) {
        if (bindGroups[i])       wgpuBindGroupRelease(bindGroups[i]);
        if (uniformBuffers[i])   wgpuBufferRelease(uniformBuffers[i]);
    }

    if (icoIndexBuffer)     wgpuBufferRelease(icoIndexBuffer);
    if (icoVertexBuffer)    wgpuBufferRelease(icoVertexBuffer);
    if (uvIndexBuffer)     wgpuBufferRelease(uvIndexBuffer);
    if (uvVertexBuffer)    wgpuBufferRelease(uvVertexBuffer);

    wgpuRenderPipelineRelease(testRenderPipeline);
    wgpuBindGroupLayoutRelease(testBindGroupLayout);
    wgpuShaderModuleRelease(testShaderModule);
} catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
