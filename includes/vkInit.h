#ifndef VKINIT_H_
#define VKINIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#define LOG_INFO    0
#define LOG_WARNING 1
#define LOG_ERROR   2

#define VK_CHECK(expr)      {assert(expr == VK_SUCCESS);}
#define SDL_CHECK(expr)     {assert(expr == SDL_TRUE);}
#define VK_CLAMP(v, bound)  (((v) > (bound)) ? (bound): (v))
/** basic logging macro **/
#define VK_LOG(level, msg)                                                      \
    do {                                                                        \
        switch ((level)) {                                                      \
            case 0:  fprintf(stderr, "[INFO]: %s\n", (msg)); break;             \
            case 1:  fprintf(stderr, "[WARNING]: %s\n", (msg)); break;          \
            case 2:  fprintf(stderr, "[ERROR]: %s\n", (msg)); break;            \
        }                                                                       \
    } while(0)
/** takes filename, extension name and length for each along with result variable **
 ** and compares to find if filetype is the specified type                        **/
#define VK_CHECK_FILETYPE(filename, filetype, namesize, typesize, result)       \
    do {                                                                        \
        char c;                                                                 \
        uint32_t nameindex = 0;                                                 \
                                                                                \
        /** skip to extension */                                                \
        while ((c = (filename)[nameindex]) != '.' && nameindex < (namesize))    \
            nameindex++;                                                        \
                                                                                \
        result = !strncmp(&(filename)[nameindex], filetype, typesize);          \
    } while(0)

enum VK_QUEUE_FAMILIES_ENUM {
    GRAPHICS       = 0x00,
    COMPUTE        = 0x01,
    TRANSFER       = 0x02,
    SPARSE_BINDING = 0x03,
    PRESENT        = 0x04
};

typedef struct VK_SUPPORTED_QUEUE_FAMILIES {
    bool     found[5];
    uint32_t indicies[5];
} VK_SUPPORTED_QUEUE_FAMILIES;

typedef struct VK_SWAPCHAIN_SUPPORT_DETAILS {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR       format;
    VkPresentModeKHR         present_mode;
    VkExtent2D               extent;

    bool suggestion; // if true and specs not found use defaults
} VK_SWAPCHAIN_SUPPORT_DETAILS;

typedef struct VK_DEVICE_SPECIFICATION {
    // VK Device Types
    bool supported_types[5];

    // VK Device Features - if value is true then feature is checked
    VkPhysicalDeviceFeatures device_features;

    // VK Queues required by the user
    bool specified_queues[5];
} VK_DEVICE_SPECIFICATION;

typedef struct VK_ATTACHMENT_DESCRIPTION_SPECIFICATION {
    VkAttachmentDescriptionFlags flags;
    VkFormat                     format;
    VkSampleCountFlagBits        samples;
    VkAttachmentLoadOp           load_op;
    VkAttachmentStoreOp          store_op;
    VkAttachmentLoadOp           stencil_load_op;
    VkAttachmentStoreOp          stencil_store_op;
    VkImageLayout                initial_layout;
    VkImageLayout                final_layout; 
} VK_ATTACHMENT_DESCRIPTION_SPECIFICATION;

typedef struct VK_ATTACHMENT_COLOR_BLEND_SPECIFICATION {
    VkBool32                 blend_enable;
    VkBlendFactor            src_color_blend_factor;
    VkBlendFactor            dst_color_blend_factor;
    VkBlendOp                color_blend_op;
    VkBlendFactor            src_alpha_blend_factor;
    VkBlendFactor            dst_alpha_blend_factor;
    VkBlendOp                alpha_blend_op;
    VkColorComponentFlags    color_write_mask; 
} VK_ATTACHMENT_COLOR_BLEND_SPECIFICATION;

typedef struct VK_SUBPASS_SPECIFICATION {
    VkSubpassDescriptionFlags flags;
    VkPipelineBindPoint       pipeline_bind_point;
    uint32_t                  color_attachments_count;
    uint32_t                  input_attachments_count;
    uint32_t                  preserve_attachment_count;
    VkAttachmentReference*    input_attachments;
    VkAttachmentReference*    color_attachments;
    uint32_t*                 preserve_attachments;
    VkAttachmentReference*    resolve_attachments;
    VkAttachmentReference*    depth_stencil_attchments;
} VK_SUBPASS_SPECIFICATION;

typedef struct VK_PIPELINE_SPECIFICATION {
    /** vertex input create info specs */
    uint32_t                           vertex_binding_descriptions_count;
    uint32_t                           vertex_attribute_descriptions_count;
    VkVertexInputBindingDescription   *vertex_binding_descriptions;
    VkVertexInputAttributeDescription *vertex_attribute_descriptions;

    /** input assembly creat info specs */
    VkPrimitiveTopology topology;
    VkBool32            primitive_restart_enable;

    /** viewport specs */
    float x;
    float y;
    float width;
    float height;
    float min_depth;
    float max_depth;

    /** scissor specs */
    VkRect2D scissor;

    /** rasterizer specs */
    VkBool32        depth_clamp_enable;
    VkBool32        rasterizer_discard_enable;
    VkPolygonMode   polygon_mode;
    VkCullModeFlags cull_mode;
    VkFrontFace     front_face;
    VkBool32        depth_bias_enable;
    float           depth_bias_constant_factor;
    float           depth_bias_clamp;
    float           depth_bias_slope_factor;
    float           line_width;

    VkBool32                 sample_shading_enable;
    VkSampleCountFlagBits    rasterization_samples;
    float                    min_sample_shading;
    const VkSampleMask      *p_sample_mask;
    VkBool32                 alpha_to_coverage_enable;
    VkBool32                 alpha_to_one_enable;

    /** TODO: Depth buffering */

    /** Color blending attachment */
    uint32_t                             color_blend_attachment_states_count;
    VkPipelineColorBlendAttachmentState *color_blend_attachment_states;
    VkBool32                             logic_op_enable;
    VkLogicOp                            logic_op;
    float                                blend_constants[4];
    
    /** render pass */
    uint32_t                 attachment_descriptions_count;
    VkAttachmentDescription *attachment_descriptions;
    uint32_t                 subpass_descriptions_count;
    VkSubpassDescription    *subpass_descriptions;
    
} VK_PIPELINE_SPECIFICATION;

typedef struct VK_CONTEXT {
    /** SDL Objects */
    SDL_Window *window;

    /** Vulkan Objects */
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physical_device;
    VkDevice         logical_device;
    VkSwapchainKHR   swapchain;

    VkQueue  queues[5];

    uint32_t      image_count;
    VkImage           *images;
    VkImageView  *image_views;
    
    VkRenderPass     render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline       pipeline;

    uint32_t       framebuffers_count;
    VkFramebuffer *framebuffers;

    /** Framework Objects */
    VK_DEVICE_SPECIFICATION         device_details;
    VK_SUPPORTED_QUEUE_FAMILIES     queue_families;
    VK_SWAPCHAIN_SUPPORT_DETAILS swapchain_details;
} VK_CONTEXT;

/** Public functions */
extern void vk_create_instance
(
    VK_CONTEXT *context,
    const char *application_name,
    const char *engine_name,
    const char *required_extensions[],
    const char *required_layers[],
    const uint32_t required_extensions_count,
    const uint32_t required_layer_count
);
extern void vk_create_surface (VK_CONTEXT *context);
extern void vk_select_physical_device 
(
    VK_CONTEXT *context, 
    VK_DEVICE_SPECIFICATION requirements, 
    const char **required_extensions, 
    uint32_t required_extension_count
);
extern void vk_create_logical_device 
(
    VK_CONTEXT *context,
    const char **extensions,
    uint32_t extension_count
);
extern void vk_create_queues (VK_CONTEXT *context);
extern void vk_create_swapchain
(
    VK_CONTEXT *context,
    VK_SWAPCHAIN_SUPPORT_DETAILS swapchain_details
);
extern void vk_create_image_views (VK_CONTEXT *context);
extern void vk_create_attachment_description
(
    VK_PIPELINE_SPECIFICATION *pipeline_specification,
    VK_ATTACHMENT_DESCRIPTION_SPECIFICATION attachment_specification
);
extern void vk_create_attachment_color_blend
(
    VK_PIPELINE_SPECIFICATION *pipeline_specification,
    VK_ATTACHMENT_COLOR_BLEND_SPECIFICATION attachment_specification
);
extern void vk_create_subpass
(
    VK_PIPELINE_SPECIFICATION *pipeline_specification,
    VK_SUBPASS_SPECIFICATION subpass_specification 
);
extern void vk_create_pipeline
(
    VK_CONTEXT *context,
    VK_PIPELINE_SPECIFICATION pipeline_specification,
    const char **filenames,
    const uint32_t count
);
extern void vk_create_framebuffers
(
    VK_CONTEXT *context
);
#endif // VKMAIN_H_
