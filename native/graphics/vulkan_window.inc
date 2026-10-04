static int ds_vk_reset_acquire_semaphores(void) {
    if (!vk_device)
        return 0;
    for (int i = 0; i < DS_VK_FRAMES_IN_FLIGHT; i++) {
        DsVkFrame *f = &vk_frames[i];
        if (f->image_available == VK_NULL_HANDLE)
            continue;
        vkDestroySemaphore(vk_device, f->image_available, NULL);
        f->image_available = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo si = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (!ds_vk_check_result(vkCreateSemaphore(vk_device, &si, NULL, &f->image_available),
                                "vkCreateSemaphore(acquire reset)")) {
            f->image_available = VK_NULL_HANDLE;
            vk_device_lost = 1;
            return 0;
        }
    }
    return 1;
}

static void ds_vk_surface_lost(void) {
    if (!vk_device) {
        vk_window = NULL;
        vk_surface_lost = 0;
        return;
    }
    vkDeviceWaitIdle(vk_device);
    ds_vk_release_frame_rate();
    ds_vk_destroy_swapchain();
    (void)ds_vk_reset_acquire_semaphores();
    for (int i = 0; i < DS_VK_FRAMES_IN_FLIGHT; i++) vk_frames[i].submitted = 0;
    if (vk_surface != VK_NULL_HANDLE && vk_instance != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
    vk_surface = VK_NULL_HANDLE;
    vk_window = NULL;
    vk_begun = 0;
    vk_image_held = 0;
    vk_need_recreate = 0;
    vk_sw_stale = 0;
    vk_sw_check_timer = 0;
    vk_surface_lost = 0;
    vk_pace_next_ns = 0;
}

static int ds_vk_attach_window(ANativeWindow *window) {
    if (!window || !vk_device || !vk_instance || !vk_phys)
        return 0;
    VkAndroidSurfaceCreateInfoKHR sci = {VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
    sci.window = window;
    if (!ds_vk_check_result(vkCreateAndroidSurfaceKHR(vk_instance, &sci, NULL, &vk_surface),
                            "vkCreateAndroidSurfaceKHR(reattach)")) {
        vk_surface = VK_NULL_HANDLE;
        return 0;
    }
    VkBool32 present = VK_FALSE;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(vk_phys, vk_queue_family, vk_surface, &present) != VK_SUCCESS ||
        !present) {
        vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
        vk_surface = VK_NULL_HANDLE;
        return 0;
    }
    vk_window = window;
    ds_vk_request_frame_rate(window);

    vk_need_recreate = 1;
    vk_begun = 0;
    vk_image_held = 0;
    vk_pace_next_ns = 0;
    vk_reattaches++;
    ds_log("vulkan: window #%u attached to the kept device", vk_reattaches);
    return 1;
}

static int ds_vk_recover(void) {
    ANativeWindow *window = vk_window ? vk_window : vk_rebuild_window;
    if (vk_device_lost || !vk_ready) {
        vk_device_lost = 0;
        vk_surface_lost = 0;
        if (!window)
            return 0;

        if (vk_rebuild_window && (vk_rebuild_tries++ % 30) != 0)
            return 0;
        ds_log_err("vulkan: rebuilding the renderer (device lost or earlier failure)");
        ds_vk_shutdown_backend();
        if (!ds_vk_init_backend(window)) {
            vk_rebuild_window = window;
            return 0;
        }
        vk_rebuild_window = NULL;
        vk_rebuild_tries = 0;
        return 1;
    }
    if (vk_surface_lost) {
        ds_vk_surface_lost();
        if (window && ds_vk_attach_window(window))
            return 1;
        vk_device_lost = 1;
        vk_rebuild_window = window;
        return 0;
    }
    return 1;
}
