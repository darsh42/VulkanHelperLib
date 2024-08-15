#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "vkInit.h"

#define X 0
#define Y 0
#define W 640
#define H 480

int main(void) {
    /** context specification */
    VK_CONTEXT ctx                                    = {};
    VK_DEVICE_SPECIFICATION device_specification      = {};
    VK_SWAPCHAIN_SUPPORT_DETAILS swapchain_details    = {};
    VK_SUBPASS_SPECIFICATION subpass_specification    = {};
    VK_PIPELINE_SPECIFICATION pipeline_specification  = {};

    VK_ATTACHMENT_COLOR_BLEND_SPECIFICATION attachment_color_blend_specification = {};
    VK_ATTACHMENT_DESCRIPTION_SPECIFICATION attachment_description_specification = {};

    const char *application_name = "example app";
    const char *engine_name      = "example app engine";
    const char **required_instance_extensions;
    const char *required_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    const char *required_layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };
    const char *shader_files[] = {
        "shaders/triangle.vert.spv",
        "shaders/triangle.frag.spv",
    };
    uint32_t required_instance_extension_count = 0;
    uint32_t required_device_extension_count   = 1;
    uint32_t required_layer_count              = 1;
    uint32_t shader_files_count                = 2;
    
    /** SDL context definition */
    SDL_Init(SDL_INIT_VIDEO);
    ctx.window = SDL_CreateWindow("example app", X, Y, W, H, SDL_WINDOW_VULKAN);
    SDL_CHECK(SDL_Vulkan_GetInstanceExtensions(ctx.window, &required_instance_extension_count, NULL));
    required_instance_extensions = malloc(sizeof(char *) * required_instance_extension_count);
    SDL_CHECK(SDL_Vulkan_GetInstanceExtensions(ctx.window, &required_instance_extension_count, required_instance_extensions));
    
    /***** vulkan context creation *****/
    /* create the instance */
    vk_create_instance
    (
         &ctx, 
         application_name, 
         engine_name, 
         required_instance_extensions, 
         required_layers, 
         required_instance_extension_count, 
         required_layer_count
    );
    /* create the surface */
    vk_create_surface(&ctx);
    /* specify the device characteristics */
    device_specification = (VK_DEVICE_SPECIFICATION) {
        .supported_types[VK_PHYSICAL_DEVICE_TYPE_CPU] = 1,
    };
    /* select a device per your specifications */
    vk_select_physical_device
    (
        &ctx, 
        device_specification, 
        required_device_extensions, 
        required_device_extension_count
    );
    /* create a corresponding logical device */
    vk_create_logical_device
    (
        &ctx,
        required_device_extensions,
        required_device_extension_count
    );
    /* create the device queues */
    vk_create_queues(&ctx);
    /* specify the swapchain details */
    swapchain_details = (VK_SWAPCHAIN_SUPPORT_DETAILS) {
        .present_mode = VK_PRESENT_MODE_FIFO_KHR,
        .suggestion = true,
    };
    /* create the swapchain */
    vk_create_swapchain
    (
         &ctx,
         swapchain_details
    );
    /* create the image views */
    vk_create_image_views(&ctx);
    /* specify the pipeline specifications */
    pipeline_specification = (VK_PIPELINE_SPECIFICATION) {
        /** vertex descriptions */
        .vertex_binding_descriptions_count   = 0,
        .vertex_attribute_descriptions_count = 0,
        .vertex_binding_descriptions         = NULL,
        .vertex_attribute_descriptions       = NULL,

        /** input assembly */
        .topology                 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitive_restart_enable = VK_FALSE,

        /** viewport */
        .x      = X,
        .y      = Y,
        .width  = W,
        .height = H,

        /** scissor */
        .scissor = {},

        /** rasterizer */
        .depth_clamp_enable         = VK_FALSE,
        .rasterizer_discard_enable  = VK_FALSE,
        .polygon_mode               = VK_POLYGON_MODE_FILL,
        .cull_mode                  = VK_CULL_MODE_BACK_BIT,
        .front_face                 = VK_FRONT_FACE_CLOCKWISE,
        .depth_bias_enable          = VK_FALSE,
        .depth_bias_constant_factor = 0.0f,
        .depth_bias_clamp           = 0.0f,
        .depth_bias_slope_factor    = 0.0f,
        .line_width                 = 1.0f,

        /** multisampler */
        .sample_shading_enable    = VK_FALSE,
        .rasterization_samples    = VK_SAMPLE_COUNT_1_BIT,
        .min_sample_shading       = 1.0f,
        .p_sample_mask            = NULL,
        .alpha_to_coverage_enable = VK_FALSE,
        .alpha_to_one_enable      = VK_FALSE,

        /** color blending */
        .logic_op_enable                    = VK_FALSE,
        .logic_op                           = VK_LOGIC_OP_COPY,
        /** need to create clr_blnd_attch_st_cnt */
        .color_blend_attachment_states_count = 0,
        .color_blend_attachment_states      = NULL,
        .blend_constants[0]                 = 0.0f,
        .blend_constants[1]                 = 0.0f,
        .blend_constants[2]                 = 0.0f,
        .blend_constants[3]                 = 0.0f,
    };
    /** REPEAT: create one or more attachment descriptions by re-specifying and calling the creation function */
    /* specify the attachment description specifications */
    attachment_description_specification = (VK_ATTACHMENT_DESCRIPTION_SPECIFICATION) {
        .flags            = 0,
        .format           = ctx.swapchain_details.format.format,
        .samples          = VK_SAMPLE_COUNT_1_BIT,
        .load_op          = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op         = VK_ATTACHMENT_STORE_OP_STORE,
        .stencil_load_op  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout   = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    /* create and add to the pipeline specifications */
    vk_create_attachment_description
    (
        &pipeline_specification,
        attachment_description_specification
    );
    /** REPEAT: create one or more attachment color blend by re-specifying and calling the creation function */
    /* specify the attachment color blend specifications */
    attachment_color_blend_specification = (VK_ATTACHMENT_COLOR_BLEND_SPECIFICATION) {
        .color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blend_enable = VK_TRUE,
        .src_color_blend_factor = VK_BLEND_FACTOR_ONE,
        .dst_color_blend_factor = VK_BLEND_FACTOR_ZERO,
        .color_blend_op = VK_BLEND_OP_ADD,
        .src_alpha_blend_factor = VK_BLEND_FACTOR_ONE,
        .dst_alpha_blend_factor = VK_BLEND_FACTOR_ZERO,
        .alpha_blend_op = VK_BLEND_OP_ADD,
    };
    /* create and add to pipeline specifications */
    vk_create_attachment_color_blend
    (
        &pipeline_specification,
        attachment_color_blend_specification
    );
    /** REPEAT: create one or more subpasses by re-specifying and calling the creation function */
    /* specify subpass specifications */
    subpass_specification = (VK_SUBPASS_SPECIFICATION) {
        .color_attachments_count = 1,
        .color_attachments       = malloc(sizeof(VkAttachmentReference) * 1)
    };
    /* create all color attachment references */
    subpass_specification.color_attachments[0] = (VkAttachmentReference) {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    /* create and add subpass to pipeline specifications */
    vk_create_subpass
    (
        &pipeline_specification,
        subpass_specification
    );
    /* create the pipeline */
    vk_create_pipeline
    (
        &ctx,
        pipeline_specification,
        shader_files,
        shader_files_count
    );
    vk_create_framebuffers(&ctx);
    /***** application code *****/


    /***** context cleanup *****/
    for (uint32_t i = 0; i < ctx.framebuffers_count; i++)
        vkDestroyFramebuffer(ctx.logical_device, ctx.framebuffers[i], NULL);
    vkDestroyPipeline(ctx.logical_device, ctx.pipeline, NULL);
    vkDestroyRenderPass(ctx.logical_device, ctx.render_pass, NULL);
    vkDestroyPipelineLayout(ctx.logical_device, ctx.pipeline_layout, NULL);
    for (uint32_t i = 0; i < ctx.image_count; i++)
        vkDestroyImageView(ctx.logical_device, ctx.image_views[i], NULL);
    vkDestroySwapchainKHR(ctx.logical_device, ctx.swapchain, NULL);
    vkDestroyDevice(ctx.logical_device, NULL);
    vkDestroySurfaceKHR(ctx.instance, ctx.surface, NULL);
    vkDestroyInstance(ctx.instance, NULL);
    free(required_instance_extensions);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();
}
