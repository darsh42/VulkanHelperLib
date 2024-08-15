#include "vkInit.h"
#include "SDL2/SDL_vulkan.h"

void
vk_create_instance
(
    VK_CONTEXT *context,
    const char *application_name,
    const char *engine_name,
    const char *required_extensions[],
    const char *required_layers[],
    const uint32_t required_extensions_count,
    const uint32_t required_layers_count
)
{
    VkApplicationInfo app_info = {};
    app_info = (VkApplicationInfo) {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = application_name,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = engine_name,
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_API_VERSION_1_0
    };

    // ennumerate all supported extensions and layers untill all required have been found
    uint32_t extension_count = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL));
    VkExtensionProperties extension_properties[extension_count];
    VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extension_properties));

    uint32_t supported_extensions_count = 0;
    for (uint32_t ex = 0; supported_extensions_count < required_extensions_count && ex < extension_count; ex++) {
        if (!strncmp(required_extensions[supported_extensions_count], extension_properties[ex].extensionName, strlen(required_extensions[supported_extensions_count])))
            supported_extensions_count++;
    }

    if (supported_extensions_count != required_extensions_count) {
        // error
        VK_LOG(LOG_ERROR, "required instance extensions not supported");
        exit(-1);
    }

    uint32_t layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, NULL));
    VkLayerProperties layer_properties[layer_count];
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, layer_properties));

    uint32_t supported_layer_count = 0;
    for (uint32_t ly = 0; supported_layer_count < required_layers_count && ly < layer_count; ly++) {
        if (!strcmp(required_layers[supported_layer_count], layer_properties[ly].layerName))
            supported_layer_count++;
    }

    if (supported_layer_count != required_layers_count) {
        // error
        VK_LOG(LOG_ERROR, "required instance layers not supported");
        exit(-1);
    }

    VkInstanceCreateInfo create_info = {};
    create_info = (VkInstanceCreateInfo) {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &app_info,
        .enabledExtensionCount   = required_extensions_count,
        .ppEnabledExtensionNames = required_extensions,
        .enabledLayerCount       = required_layers_count,
        .ppEnabledLayerNames     = required_layers
    };

    VK_CHECK(vkCreateInstance(&create_info, NULL, &context->instance));
    VK_LOG(LOG_INFO, "Created Instance");
}

void
vk_create_surface
(
    VK_CONTEXT *context
)
{
    SDL_CHECK(SDL_Vulkan_CreateSurface(context->window, context->instance, &context->surface));
    VK_LOG(LOG_INFO, "Created Surface");
}

void
vk_select_physical_device
(
    VK_CONTEXT *context,
    VK_DEVICE_SPECIFICATION requirements,
    const char **required_extensions,
    uint32_t required_extensions_count
)
{
    VkPhysicalDevice selected_device = VK_NULL_HANDLE;

    uint32_t device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &device_count, NULL));
    VkPhysicalDevice physical_devices[device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &device_count, physical_devices));

    for (uint32_t i = 0; i < device_count; i++) {
        bool selected = true;

        VkPhysicalDevice device = physical_devices[i];

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures   device_features;

        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        // if device type specified by the user is supported continue device evaluation
        switch (device_properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:          if (requirements.supported_types[VK_PHYSICAL_DEVICE_TYPE_OTHER])          break; else continue;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: if (requirements.supported_types[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) break; else continue;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   if (requirements.supported_types[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU])   break; else continue;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    if (requirements.supported_types[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU])    break; else continue;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:            if (requirements.supported_types[VK_PHYSICAL_DEVICE_TYPE_CPU])            break; else continue;
            case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:       continue;
        }

        context->device_details.supported_types[device_properties.deviceType] = true;

        // if specified by the user and feature is not supported continue to next device
        if (requirements.device_features.robustBufferAccess                      && !device_features.robustBufferAccess                      ) continue;
        if (requirements.device_features.fullDrawIndexUint32                     && !device_features.fullDrawIndexUint32                     ) continue;
        if (requirements.device_features.imageCubeArray                          && !device_features.imageCubeArray                          ) continue;
        if (requirements.device_features.independentBlend                        && !device_features.independentBlend                        ) continue;
        if (requirements.device_features.geometryShader                          && !device_features.geometryShader                          ) continue;
        if (requirements.device_features.tessellationShader                      && !device_features.tessellationShader                      ) continue;
        if (requirements.device_features.sampleRateShading                       && !device_features.sampleRateShading                       ) continue;
        if (requirements.device_features.dualSrcBlend                            && !device_features.dualSrcBlend                            ) continue;
        if (requirements.device_features.logicOp                                 && !device_features.logicOp                                 ) continue;
        if (requirements.device_features.multiDrawIndirect                       && !device_features.multiDrawIndirect                       ) continue;
        if (requirements.device_features.drawIndirectFirstInstance               && !device_features.drawIndirectFirstInstance               ) continue;
        if (requirements.device_features.depthClamp                              && !device_features.depthClamp                              ) continue;
        if (requirements.device_features.depthBiasClamp                          && !device_features.depthBiasClamp                          ) continue;
        if (requirements.device_features.fillModeNonSolid                        && !device_features.fillModeNonSolid                        ) continue;
        if (requirements.device_features.depthBounds                             && !device_features.depthBounds                             ) continue;
        if (requirements.device_features.wideLines                               && !device_features.wideLines                               ) continue;
        if (requirements.device_features.largePoints                             && !device_features.largePoints                             ) continue;
        if (requirements.device_features.alphaToOne                              && !device_features.alphaToOne                              ) continue;
        if (requirements.device_features.multiViewport                           && !device_features.multiViewport                           ) continue;
        if (requirements.device_features.samplerAnisotropy                       && !device_features.samplerAnisotropy                       ) continue;
        if (requirements.device_features.textureCompressionETC2                  && !device_features.textureCompressionETC2                  ) continue;
        if (requirements.device_features.textureCompressionASTC_LDR              && !device_features.textureCompressionASTC_LDR              ) continue;
        if (requirements.device_features.textureCompressionBC                    && !device_features.textureCompressionBC                    ) continue;
        if (requirements.device_features.occlusionQueryPrecise                   && !device_features.occlusionQueryPrecise                   ) continue;
        if (requirements.device_features.pipelineStatisticsQuery                 && !device_features.pipelineStatisticsQuery                 ) continue;
        if (requirements.device_features.vertexPipelineStoresAndAtomics          && !device_features.vertexPipelineStoresAndAtomics          ) continue;
        if (requirements.device_features.fragmentStoresAndAtomics                && !device_features.fragmentStoresAndAtomics                ) continue;
        if (requirements.device_features.shaderTessellationAndGeometryPointSize  && !device_features.shaderTessellationAndGeometryPointSize  ) continue;
        if (requirements.device_features.shaderImageGatherExtended               && !device_features.shaderImageGatherExtended               ) continue;
        if (requirements.device_features.shaderStorageImageExtendedFormats       && !device_features.shaderStorageImageExtendedFormats       ) continue;
        if (requirements.device_features.shaderStorageImageMultisample           && !device_features.shaderStorageImageMultisample           ) continue;
        if (requirements.device_features.shaderStorageImageReadWithoutFormat     && !device_features.shaderStorageImageReadWithoutFormat     ) continue;
        if (requirements.device_features.shaderStorageImageWriteWithoutFormat    && !device_features.shaderStorageImageWriteWithoutFormat    ) continue;
        if (requirements.device_features.shaderUniformBufferArrayDynamicIndexing && !device_features.shaderUniformBufferArrayDynamicIndexing ) continue;
        if (requirements.device_features.shaderSampledImageArrayDynamicIndexing  && !device_features.shaderSampledImageArrayDynamicIndexing  ) continue;
        if (requirements.device_features.shaderStorageBufferArrayDynamicIndexing && !device_features.shaderStorageBufferArrayDynamicIndexing ) continue;
        if (requirements.device_features.shaderStorageImageArrayDynamicIndexing  && !device_features.shaderStorageImageArrayDynamicIndexing  ) continue;
        if (requirements.device_features.shaderClipDistance                      && !device_features.shaderClipDistance                      ) continue;
        if (requirements.device_features.shaderCullDistance                      && !device_features.shaderCullDistance                      ) continue;
        if (requirements.device_features.shaderFloat64                           && !device_features.shaderFloat64                           ) continue;
        if (requirements.device_features.shaderInt64                             && !device_features.shaderInt64                             ) continue;
        if (requirements.device_features.shaderInt16                             && !device_features.shaderInt16                             ) continue;
        if (requirements.device_features.shaderResourceResidency                 && !device_features.shaderResourceResidency                 ) continue;
        if (requirements.device_features.shaderResourceMinLod                    && !device_features.shaderResourceMinLod                    ) continue;
        if (requirements.device_features.sparseBinding                           && !device_features.sparseBinding                           ) continue;
        if (requirements.device_features.sparseResidencyBuffer                   && !device_features.sparseResidencyBuffer                   ) continue;
        if (requirements.device_features.sparseResidencyImage2D                  && !device_features.sparseResidencyImage2D                  ) continue;
        if (requirements.device_features.sparseResidencyImage3D                  && !device_features.sparseResidencyImage3D                  ) continue;
        if (requirements.device_features.sparseResidency2Samples                 && !device_features.sparseResidency2Samples                 ) continue;
        if (requirements.device_features.sparseResidency4Samples                 && !device_features.sparseResidency4Samples                 ) continue;
        if (requirements.device_features.sparseResidency8Samples                 && !device_features.sparseResidency8Samples                 ) continue;
        if (requirements.device_features.sparseResidency16Samples                && !device_features.sparseResidency16Samples                ) continue;
        if (requirements.device_features.sparseResidencyAliased                  && !device_features.sparseResidencyAliased                  ) continue;
        if (requirements.device_features.variableMultisampleRate                 && !device_features.variableMultisampleRate                 ) continue;
        if (requirements.device_features.inheritedQueries                        && !device_features.inheritedQueries                        ) continue;

        context->device_details.device_features = device_features;

        // check the device extension support
        uint32_t extension_count = 0;
        vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
        VkExtensionProperties extension_properties[extension_count];
        vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extension_properties);

        uint32_t supported_extensions_count = 0;
        for (uint32_t ex = 0; supported_extensions_count < required_extensions_count && ex < extension_count; ex++) {
            if (!strncmp(required_extensions[supported_extensions_count],
                         extension_properties[ex].extensionName,
                         strlen(required_extensions[supported_extensions_count])))
            {
                supported_extensions_count++;
            }
        }

        if (supported_extensions_count != required_extensions_count)
            continue;

        // check the supported queue families of the device
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
        VkQueueFamilyProperties queue_families[queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

        uint32_t max_supported_queue_types = 0;
        for (uint32_t j = 0; j < queue_family_count; j++) {
            VkBool32 present_support;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, j, context->surface, &present_support);

            VkQueueFamilyProperties queue_family = queue_families[j];

            // count number of supported queue types
            uint32_t current_supported_queue_types = present_support;
            for (uint32_t k = 0; k < 4; k++)
                current_supported_queue_types += (queue_family.queueFlags >> k) & 0x1;

            // compare to maximum number of supported queue types for a queue
            if (max_supported_queue_types >= current_supported_queue_types)
                continue;

            // if this one supports more types
            max_supported_queue_types = current_supported_queue_types;

            // update corresponding indicies
            for (uint32_t l = 0; l < 4; l++) {
                bool flag = max_supported_queue_types >> l;

                if (!flag) continue;

                context->queue_families.found[l]    = flag;
                context->queue_families.indicies[l] = j;
            }

            if (!present_support) continue;

            context->queue_families.found[4]    = present_support;
            context->queue_families.indicies[4] = j;
        }

        // check if the minimum queue requirements have been met
        for (uint32_t j = 0; j < 5; j++) {
            if (requirements.specified_queues[j] && !context->queue_families.found[j]) {
                selected = false;
                break;
            }
        }

        // if the device is deemed worthy select it and break;
        if (selected) {
            selected_device = device;
            break;
        }
    }

    // if no device is suitable throw an error
    if (selected_device == VK_NULL_HANDLE) {
        // error
        VK_LOG(LOG_ERROR, "Could not select suitable physical device");
        exit(-1);
    }

    context->physical_device = selected_device;
    VK_LOG(LOG_INFO, "Selected Physical Device");
}

void
vk_create_logical_device
(
    VK_CONTEXT *context,
    const char **extensions,
    uint32_t extension_count
)
{
    // Find the unique queue indicies
    uint32_t queue_create_info_count = 0;
    uint32_t queue_create_info_indicies[5];

    for (uint32_t i = 0; i < 5; i++) {
        if (!context->queue_families.found[i])
            continue;

        bool unique = true;
        for (uint32_t j = 0; j < queue_create_info_count; j++) {
            if (context->queue_families.indicies[i] == queue_create_info_indicies[j]) {
                unique = false;
                break;
            }
        }

        if (!unique)
            continue;

        queue_create_info_count++;
        queue_create_info_indicies[queue_create_info_count] = i;
    }

    // define create infos for all the unique queue indicies and create them
    float queue_priorities[queue_create_info_count];
    VkDeviceQueueCreateInfo queue_create_infos[queue_create_info_count];

    for (uint32_t i = 0; i < queue_create_info_count; i++) {
        queue_priorities[i]   = 1.0f;
        queue_create_infos[i] = (VkDeviceQueueCreateInfo) {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pQueuePriorities = &queue_priorities[i],
            .queueFamilyIndex = queue_create_info_indicies[i],
            .queueCount       = 1
        };
    }

    VkDeviceCreateInfo logical_device_create_info = {};
    logical_device_create_info   = (VkDeviceCreateInfo) {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount    = queue_create_info_count,
        .pQueueCreateInfos       = queue_create_infos,
        .pEnabledFeatures        = &context->device_details.device_features,
        .enabledExtensionCount   = extension_count,
        .ppEnabledExtensionNames = extensions,
    };

    VK_CHECK(vkCreateDevice(context->physical_device, &logical_device_create_info, NULL, &context->logical_device));
    VK_LOG(LOG_INFO, "Created Logical Device");
}

void
vk_create_queues
(
    VK_CONTEXT *context
)
{
    for (uint32_t i = 0; i < 5; i++) {
        if (!context->device_details.specified_queues[i])
            continue;

        vkGetDeviceQueue(context->logical_device, context->queue_families.indicies[i], 0, &context->queues[i]);
    }
    VK_LOG(LOG_INFO, "Retrived Queues");
}

void
vk_create_swapchain
(
    VK_CONTEXT *context,
    VK_SWAPCHAIN_SUPPORT_DETAILS swapchain_specification
)
{

    VkPresentModeKHR   present_mode;
    VkSurfaceFormatKHR format       = {};
    VkExtent2D         extent       = {};

    // check for swap chain support
    uint32_t format_count;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, context->surface, &format_count, NULL));
    VkSurfaceFormatKHR formats[format_count];
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, context->surface, &format_count, formats));

    bool format_found = false;
    for (uint32_t j = 0; j < format_count; j++) {
        if (swapchain_specification.format.format     == formats[j].format &&
            swapchain_specification.format.colorSpace == formats[j].colorSpace)
        {
            format_found = true;
            break;
        }
    }

    if (!format_found && !swapchain_specification.suggestion) {
        VK_LOG(LOG_ERROR, "Cannot find specified format");
        exit(-1);
    }

    if (!format_found) {
        VK_LOG(LOG_WARNING, "Could not find specified format, switching to defaults");
        format = formats[0];
    } else {
        format = swapchain_specification.format;
    }

    context->swapchain_details.format = format;

    uint32_t present_modes_count;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(context->physical_device, context->surface, &present_modes_count, NULL));
    VkPresentModeKHR present_modes[present_modes_count];
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(context->physical_device, context->surface, &present_modes_count, present_modes));

    bool present_mode_found = false;
    for (uint32_t j = 0; j < present_modes_count; j++) {
        if (swapchain_specification.present_mode == present_modes[j]) {
            present_mode_found = true;
            break;
        }
    }

    if (!present_mode_found && !swapchain_specification.suggestion) {
        VK_LOG(LOG_ERROR, "Cannot find specified present mode");
        exit(-1);
    }

    if (!present_mode_found) {
        VK_LOG(LOG_INFO, "Could not find specified present mode, switching to defaults");
        present_mode = VK_PRESENT_MODE_FIFO_KHR;
    } else {
        present_mode = swapchain_specification.present_mode;
    }

    context->swapchain_details.present_mode = present_mode;

    VkSurfaceCapabilitiesKHR capabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physical_device, context->surface, &capabilities));

    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        uint32_t width, height;

        SDL_Vulkan_GetDrawableSize(context->window, (int *) &width, (int *) &height);

        extent.width = VK_CLAMP(width, capabilities.minImageExtent.width);
        extent.height = VK_CLAMP(height, capabilities.minImageExtent.height);
    }

    context->swapchain_details.capabilities = capabilities;
    context->swapchain_details.extent = extent;

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface          = context->surface;
    create_info.minImageCount    = capabilities.minImageCount + 1; // spec recommendation
    create_info.imageExtent      = extent;
    create_info.imageExtent      = extent;
    create_info.imageFormat      = format.format;
    create_info.imageColorSpace  = format.colorSpace;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (context->queue_families.indicies[GRAPHICS] != context->queue_families.indicies[PRESENT]) {
        uint32_t indicies[] = {
            context->queue_families.indicies[GRAPHICS],
            context->queue_families.indicies[PRESENT]
        };

        create_info.imageSharingMode       = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices  = indicies;
    } else {
        create_info.imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices  = NULL;
    }

    create_info.preTransform   = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode    = present_mode;
    create_info.clipped        = VK_TRUE;
    create_info.oldSwapchain   = NULL;

    vkCreateSwapchainKHR(context->logical_device, &create_info, NULL, &context->swapchain);
    VK_LOG(LOG_INFO, "Created Swapchain");

}

void
vk_create_image_views
(
    VK_CONTEXT *context
)
{
    /** get the images of the swapchain */
    vkGetSwapchainImagesKHR(context->logical_device, context->swapchain, &context->image_count, NULL);
    context->images      = malloc(sizeof(VkImage) * context->image_count);
    context->image_views = malloc(sizeof(VkImageView) * context->image_count);
    vkGetSwapchainImagesKHR(context->logical_device, context->swapchain, &context->image_count, context->images);

    /** TODO: Reasses this structure to be more generic         *
     ** Currently I think that this will be too Example focused */
    for (uint32_t i = 0; i < context->image_count; i++) {
        VkImage image = context->images[i];

        VkImageViewCreateInfo create_info           = {};
        create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image                           = image;
        create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format                          = context->swapchain_details.format.format;

        /** components allows to change (swizzle) color channels around */
        create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;

        /** Describe the purpose of the image and what will be accessed */
        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;

        VK_CHECK(vkCreateImageView(context->logical_device, &create_info, NULL, &context->image_views[i]));
    }
    VK_LOG(LOG_INFO, "Created Images and Image Views");
}

void
vk_create_attachment_description
(
    VK_PIPELINE_SPECIFICATION *pipeline_specification,
    VK_ATTACHMENT_DESCRIPTION_SPECIFICATION attachment_specification
)
{
    pipeline_specification->attachment_descriptions_count++;
    pipeline_specification->attachment_descriptions = realloc(pipeline_specification->attachment_descriptions, sizeof(VkAttachmentDescription) * pipeline_specification->attachment_descriptions_count);
    pipeline_specification->attachment_descriptions[pipeline_specification->attachment_descriptions_count - 1] = (VkAttachmentDescription) {
        .flags          = attachment_specification.flags,
        .format         = attachment_specification.format,
        .samples        = attachment_specification.samples,
        .loadOp         = attachment_specification.load_op,
        .storeOp        = attachment_specification.store_op,
        .stencilLoadOp  = attachment_specification.stencil_load_op,
        .stencilStoreOp = attachment_specification.stencil_store_op,
        .initialLayout  = attachment_specification.initial_layout,
        .finalLayout    = attachment_specification.final_layout
    };
    VK_LOG(LOG_INFO, "Created Attachment");
}

void
vk_create_attachment_color_blend
(
    VK_PIPELINE_SPECIFICATION *pipeline_specification,
    VK_ATTACHMENT_COLOR_BLEND_SPECIFICATION attachment_specification
)
{
    pipeline_specification->color_blend_attachment_states_count++;
    pipeline_specification->color_blend_attachment_states = realloc(pipeline_specification->color_blend_attachment_states, sizeof(VkPipelineColorBlendAttachmentState) * pipeline_specification->color_blend_attachment_states_count);
    pipeline_specification->color_blend_attachment_states[pipeline_specification->color_blend_attachment_states_count - 1] = (VkPipelineColorBlendAttachmentState) {
        .blendEnable         = attachment_specification.blend_enable,
        .srcColorBlendFactor = attachment_specification.src_color_blend_factor,
        .dstColorBlendFactor = attachment_specification.dst_color_blend_factor,
        .colorBlendOp        = attachment_specification.color_blend_op,
        .srcAlphaBlendFactor = attachment_specification.src_alpha_blend_factor,
        .dstAlphaBlendFactor = attachment_specification.dst_alpha_blend_factor,
        .alphaBlendOp        = attachment_specification.alpha_blend_op,
        .colorWriteMask      = attachment_specification.color_write_mask
    };
}

void
vk_create_subpass
(
    VK_PIPELINE_SPECIFICATION *pipeline_specification,
    VK_SUBPASS_SPECIFICATION subpass_specification
)
{
    /** create and add a subpass to the pipeline specification */
    pipeline_specification->subpass_descriptions_count++;
    pipeline_specification->subpass_descriptions = realloc(pipeline_specification->subpass_descriptions, sizeof(VkSubpassDescription) * pipeline_specification->subpass_descriptions_count);
    pipeline_specification->subpass_descriptions[pipeline_specification->subpass_descriptions_count - 1] = (VkSubpassDescription) {
        .flags                   = subpass_specification.flags,
        .pipelineBindPoint       = subpass_specification.pipeline_bind_point,
        .colorAttachmentCount    = subpass_specification.color_attachments_count,
        .inputAttachmentCount    = subpass_specification.input_attachments_count,
        .preserveAttachmentCount = subpass_specification.preserve_attachment_count,
        .pInputAttachments       = subpass_specification.input_attachments,
        .pColorAttachments       = subpass_specification.color_attachments,
        .pPreserveAttachments    = subpass_specification.preserve_attachments,
        .pResolveAttachments     = subpass_specification.resolve_attachments,
        .pDepthStencilAttachment = subpass_specification.depth_stencil_attchments
    };
    VK_LOG(LOG_INFO, "Created Subpass");
}

void
vk_create_pipeline
(
    VK_CONTEXT *context,
    VK_PIPELINE_SPECIFICATION pipeline_specification,
    const char **filenames,
    const uint32_t count
)
{
    /** shader stage create infos */
    uint32_t shader_stage_create_info_count = 0; /** will contain number of actually created shader stage infos */

    VkShaderModule                  shader_modules[count];
    VkPipelineShaderStageCreateInfo shader_stage_create_info[count];

    for (uint32_t i = 0; i < count; i++)
    {
        size_t  size;
        char *buffer;
        bool fragtype = false;
        bool verttype = false;

        /** read and load shaders from files*/
        FILE *f = fopen(filenames[i], "rb");

        VK_CHECK_FILETYPE(filenames[i], ".frag", strlen(filenames[i]), 5, fragtype);
        VK_CHECK_FILETYPE(filenames[i], ".vert", strlen(filenames[i]), 5, verttype);

        if (!verttype && !fragtype)
        {
            VK_LOG(LOG_WARNING, "Unsupported file type, skipping");
            continue;
        }

        /** if not found continue */
        if (f == NULL)
        {
            VK_LOG(LOG_WARNING, "file not found, skipping");
            continue;
        }

        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);

        /** fix alignment for uint32_t cast when passed to create info */
        size = (size % 4 == 0) ? size: size + 4 - (size % 4);

        buffer = malloc(size * sizeof(buffer));

        /** if file exists but cant read exit */
        if (fread(buffer, 1, size, f) != size)
        {
            VK_LOG(LOG_ERROR, "Could not read file");
            exit(-1);
        }
        fclose(f);

        VkShaderModuleCreateInfo shader_create_info = {};
        shader_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_create_info.codeSize = size;
        shader_create_info.pCode    = (uint32_t *) buffer;

        /** create shader modules */
        VK_CHECK(vkCreateShaderModule(context->logical_device, &shader_create_info, NULL, &shader_modules[shader_stage_create_info_count]));
        VK_LOG(LOG_INFO, "Created Shader Module");

        VkPipelineShaderStageCreateInfo stage_create_info = {};
        stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info.stage  = (fragtype) ? VK_SHADER_STAGE_FRAGMENT_BIT: stage_create_info.stage;
        stage_create_info.stage  = (verttype) ?   VK_SHADER_STAGE_VERTEX_BIT: stage_create_info.stage;
        stage_create_info.module = shader_modules[shader_stage_create_info_count];
        stage_create_info.pName  = "main";

        shader_stage_create_info[shader_stage_create_info_count] = stage_create_info;
        shader_stage_create_info_count++;

        // vkDestroyShaderModule(context->logical_device, shader_module, NULL);
        free(buffer);
    }

    /** vertex input create infos */
    VkPipelineVertexInputStateCreateInfo vertex_create_info = {};
    vertex_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_create_info.vertexBindingDescriptionCount   = pipeline_specification.vertex_binding_descriptions_count;
    vertex_create_info.pVertexBindingDescriptions      = pipeline_specification.vertex_binding_descriptions;
    vertex_create_info.vertexAttributeDescriptionCount = pipeline_specification.vertex_attribute_descriptions_count;
    vertex_create_info.pVertexAttributeDescriptions    = pipeline_specification.vertex_attribute_descriptions;

    /** input assembly create infos */
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
    input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology               = pipeline_specification.topology;
    input_assembly_create_info.primitiveRestartEnable = pipeline_specification.primitive_restart_enable;

    /** Viewport definition */
    VkViewport viewport = {};
    viewport.x      = pipeline_specification.x;
    viewport.y      = pipeline_specification.y;
    viewport.width  = pipeline_specification.width;
    viewport.height = pipeline_specification.height;

    /** scissor */
    VkRect2D scissor = pipeline_specification.scissor;

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    /** rasterizer create info */
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {};
    rasterizer_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.depthClampEnable        = pipeline_specification.depth_clamp_enable;
    rasterizer_create_info.rasterizerDiscardEnable = pipeline_specification.rasterizer_discard_enable;
    rasterizer_create_info.polygonMode             = pipeline_specification.polygon_mode;
    rasterizer_create_info.cullMode                = pipeline_specification.cull_mode;
    rasterizer_create_info.frontFace               = pipeline_specification.front_face;
    rasterizer_create_info.depthBiasEnable         = pipeline_specification.depth_bias_enable;
    rasterizer_create_info.depthBiasConstantFactor = pipeline_specification.depth_bias_constant_factor;
    rasterizer_create_info.depthBiasClamp          = pipeline_specification.depth_bias_clamp;
    rasterizer_create_info.depthBiasSlopeFactor    = pipeline_specification.depth_bias_slope_factor;
    rasterizer_create_info.lineWidth               = pipeline_specification.line_width;

    /** TODO: multisampler create info */
    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
    multisample_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.sampleShadingEnable   = pipeline_specification.sample_shading_enable;
    multisample_create_info.rasterizationSamples  = pipeline_specification.rasterization_samples;
    multisample_create_info.minSampleShading      = pipeline_specification.min_sample_shading;
    multisample_create_info.pSampleMask           = pipeline_specification.p_sample_mask;
    multisample_create_info.alphaToCoverageEnable = pipeline_specification.alpha_to_coverage_enable;
    multisample_create_info.alphaToOneEnable      = pipeline_specification.alpha_to_one_enable;

    /** TODO: Depth buffering */

    /** TODO: Color blending */
    VkPipelineColorBlendStateCreateInfo color_blending_create_info = {};
    color_blending_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending_create_info.logicOpEnable     = pipeline_specification.logic_op_enable;
    color_blending_create_info.logicOp           = pipeline_specification.logic_op;
    color_blending_create_info.attachmentCount   = pipeline_specification.color_blend_attachment_states_count;
    color_blending_create_info.pAttachments      = pipeline_specification.color_blend_attachment_states;
    color_blending_create_info.blendConstants[0] = pipeline_specification.blend_constants[0];
    color_blending_create_info.blendConstants[1] = pipeline_specification.blend_constants[1];
    color_blending_create_info.blendConstants[2] = pipeline_specification.blend_constants[2];
    color_blending_create_info.blendConstants[3] = pipeline_specification.blend_constants[3];

    /** VkPipeline create info */
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VK_CHECK(vkCreatePipelineLayout(context->logical_device, &layout_create_info, NULL, &context->pipeline_layout));
    VK_LOG(LOG_INFO, "Created Pipeline Layout");

    /** Create VkRenderPass */
    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = pipeline_specification.attachment_descriptions_count;
    render_pass_create_info.pAttachments    = pipeline_specification.attachment_descriptions;
    render_pass_create_info.subpassCount    = pipeline_specification.subpass_descriptions_count;
    render_pass_create_info.pSubpasses      = pipeline_specification.subpass_descriptions;

    VK_CHECK(vkCreateRenderPass(context->logical_device, &render_pass_create_info, NULL, &context->render_pass));
    VK_LOG(LOG_INFO, "Created Render Pass");

    /** Create Graphics Pipeline */
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.stageCount = shader_stage_create_info_count;
    graphics_pipeline_create_info.pStages    = shader_stage_create_info;
    graphics_pipeline_create_info.pVertexInputState = &vertex_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_create_info;
    graphics_pipeline_create_info.pDepthStencilState = NULL;
    graphics_pipeline_create_info.pColorBlendState = &color_blending_create_info;
    graphics_pipeline_create_info.pDynamicState = NULL;
    graphics_pipeline_create_info.layout = context->pipeline_layout;
    graphics_pipeline_create_info.renderPass = context->render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(context->logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &context->pipeline));
    VK_LOG(LOG_INFO, "Created Graphics Pipeline");

    for (uint32_t i = 0; i < shader_stage_create_info_count; i++)
    {
        vkDestroyShaderModule(context->logical_device, shader_modules[i], NULL);
    }
}

void
vk_create_framebuffers
(
    VK_CONTEXT *context
)
{
    context->framebuffers_count = context->image_count;
    context->framebuffers = malloc(sizeof(VkFramebuffer) * context->framebuffers_count);

    for (uint32_t i = 0; i < context->image_count; i++) 
    {
        VkImageView attachments[] = {
            context->image_views[i]
        };

        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = context->render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = attachments;
        create_info.width = context->swapchain_details.extent.width;
        create_info.height = context->swapchain_details.extent.height;
        create_info.layers = 1;

        VK_CHECK(vkCreateFramebuffer(context->logical_device, &create_info, NULL, &context->framebuffers[i]));
        VK_LOG(LOG_INFO, "Created Framebuffer");
    }
}
