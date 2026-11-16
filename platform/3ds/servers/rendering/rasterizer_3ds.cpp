#include "rasterizer_3ds.h"
#include <malloc.h>

C3D_RenderTarget *Rasterizer3DS::top = nullptr;
bool Rasterizer3DS::active_frame = false;

static u32 pack_color(const Color &c) { return C2D_Color32f(c.r, c.g, c.b, c.a); }

bool Rasterizer3DS::initialize_gpu() {
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) return false;
    // Bounded batching, much smaller than the 4096-object default.
    if (!C2D_Init(256)) { C3D_Fini(); return false; }
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    if (!top) { C2D_Fini(); C3D_Fini(); return false; }
    C2D_Prepare();
    return true;
}

void Rasterizer3DS::begin_frame(double step) {
    frame++; delta = step; time += step;
    active_frame = C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    if (!active_frame) return;
    textures.collect_retired();
    C2D_SetTintMode(C2D_TintMult);
    C2D_TargetClear(top, C2D_Color32(32, 64, 192, 255));
    C2D_SceneBegin(top);
    C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
}

void Rasterizer3DS::end_frame(bool swap) {
    if (!active_frame) return;
    C2D_Flush();
    // The bottom console remains a CPU framebuffer; Citro3D owns top swaps.
    gfxFlushBuffers();
    C3D_FrameEnd(0);
    active_frame = false;
    // Low-frequency diagnostics; timings cover Citro3D, not all Godot CPU work.
    const uint64_t now = osGetTime();
    if (!stats_start_ms) stats_start_ms = now;
    stats_frames++;
    cpu_ms += C3D_GetProcessingTime();
    gpu_ms += C3D_GetDrawingTime();
    command_peak = MAX(command_peak, C3D_GetCmdBufUsage());
    if (now - stats_start_ms >= 5000) {
        printf("FPS %.1f C3D CPU/GPU %.2f/%.2f ms\n", stats_frames * 1000.0 / (now - stats_start_ms),
            cpu_ms / stats_frames, gpu_ms / stats_frames);
        printf("Cmd peak %.1f%% linear free %lu KiB\n", command_peak * 100, (unsigned long)(linearSpaceFree() / 1024));
        const struct mallinfo heap = mallinfo();
        printf("Newlib heap in use %lu KiB\n", (unsigned long)(heap.uordblks / 1024));
        textures.print_memory_stats();
        stats_start_ms = now; stats_frames = 0; cpu_ms = gpu_ms = 0; command_peak = 0;
    }
}

void Rasterizer3DS::finalize() {
    if (!top) return;
    if (active_frame) end_frame(true);
    C3D_FrameSync();
    textures.release_gpu();
    C3D_RenderTargetDelete(top);
    top = nullptr;
    C2D_Fini();
    C3D_Fini();
}

void TextureStorage3DS::render_target_do_clear_request(RID rid) {
    if (render_target_is_clear_requested(rid) && Rasterizer3DS::is_frame_active()) {
        C2D_Flush();
        C2D_TargetClear(Rasterizer3DS::get_top(), pack_color(render_target_get_clear_request_color(rid)));
        render_target_disable_clear_request(rid);
    }
}

RendererCanvasRender::PolygonID Canvas3DS::request_polygon(const Vector<int> &indices,
    const Vector<Point2> &points, const Vector<Color> &colors, const Vector<Point2> &uvs,
    const Vector<int> &bones, const Vector<float> &weights) {
    // Retain Godot's precomputed geometry; no tessellation/allocation each frame.
    if (!uvs.is_empty() || !bones.is_empty() || !weights.is_empty()) {
        WARN_PRINT_ONCE("3DS: textured/skinned polygons unsupported.");
        return 0;
    }
    ERR_FAIL_COND_V(points.is_empty() || points.size() > 4096 || indices.size() > 12288, 0);
    ERR_FAIL_COND_V(colors.size() != 0 && colors.size() != 1 && colors.size() != points.size(), 0);
    for (const Point2 &point : points) ERR_FAIL_COND_V(!point.is_finite(), 0);
    for (int index : indices) ERR_FAIL_COND_V(index < 0 || index >= points.size(), 0);
    PolygonData data;
    data.indices = indices; data.points = points; data.colors = colors;
    const PolygonID id = next_polygon++;
    polygons.insert(id, data);
    return id;
}

void Canvas3DS::canvas_render_items(RID target, Item *items, const Color &modulate,
    Light *lights, Light *directional, const Transform2D &canvas_transform,
    RS::CanvasItemTextureFilter filter, RS::CanvasItemTextureRepeat repeat,
    bool snap, bool &sdf_used, RenderingMethod::RenderInfo *info) {
    sdf_used = false;
    if (!Rasterizer3DS::is_frame_active()) return;
    RSG::texture_storage->render_target_do_clear_request(target);
    Transform2D last_transform;
    bool have_transform = false;
    Rect2i last_clip(0, 0, 400, 240);
    for (Item *item = items; item; item = item->next) {
        Transform2D extra;
        bool ignore_clip = false;
        for (Item::Command *cmd = item->commands; cmd; cmd = cmd->next) {
            if (cmd->type == Item::Command::TYPE_TRANSFORM) {
                extra = static_cast<Item::CommandTransform *>(cmd)->xform;
                continue;
            }
            if (cmd->type == Item::Command::TYPE_CLIP_IGNORE) {
                ignore_clip = static_cast<Item::CommandClipIgnore *>(cmd)->ignore;
                continue;
            }
            if (cmd->type != Item::Command::TYPE_RECT && cmd->type != Item::Command::TYPE_PRIMITIVE && cmd->type != Item::Command::TYPE_POLYGON) {
                WARN_PRINT_ONCE("3DS canvas: unsupported command (only rectangles, untextured primitives/polygons, transforms and clip-ignore supported).");
                continue;
            }
            const Transform2D xform = item->final_transform * extra;
            Rect2i clip(0, 0, 400, 240);
            if (item->final_clip_owner && !ignore_clip) {
                const Rect2 bounds = item->final_clip_owner->final_clip_rect.intersection(Rect2(0, 0, 400, 240));
                const Vector2i start(Math::ceil(bounds.position.x), Math::ceil(bounds.position.y));
                const Vector2i end(Math::floor(bounds.get_end().x), Math::floor(bounds.get_end().y));
                clip = Rect2i(start, end - start);
            }
            if (clip.size.x <= 0 || clip.size.y <= 0) continue;
            if (clip != last_clip) {
                C2D_Flush();
                // OrthoTilt maps landscape (x,y) to portrait (240-y,400-x).
                // Citro3D accepts exclusive right/bottom bounds.
                C3D_SetScissor(GPU_SCISSOR_NORMAL, 240 - clip.get_end().y, 400 - clip.get_end().x,
                    240 - clip.position.y, 400 - clip.position.x);
                last_clip = clip;
            }
            // Preserve Godot's complete affine transform, including parent transforms
            // and reflections. Flush before changing the matrix used by queued vertices.
            if (!have_transform || xform != last_transform) {
                C2D_Flush();
                C3D_Mtx model;
                Mtx_Identity(&model);
                model.r[0].x = xform[0].x; model.r[0].y = xform[1].x; model.r[0].w = xform[2].x;
                model.r[1].x = xform[0].y; model.r[1].y = xform[1].y; model.r[1].w = xform[2].y;
                C2D_ViewRestore(&model);
                last_transform = xform;
                have_transform = true;
            }
            // Reflections reverse winding; canvas geometry must remain two-sided.
            C3D_CullFace(GPU_CULL_NONE);
            if (cmd->type == Item::Command::TYPE_POLYGON) {
                const auto *command = static_cast<Item::CommandPolygon *>(cmd);
                const PolygonData *data = polygons.getptr(command->polygon.polygon_id);
                if (command->texture.is_valid() || !data) {
                    WARN_PRINT_ONCE("3DS: unsupported or missing polygon skipped.");
                    continue;
                }
                const bool triangles = command->primitive == RS::PRIMITIVE_TRIANGLES;
                const bool strip = command->primitive == RS::PRIMITIVE_TRIANGLE_STRIP;
                const bool lines = command->primitive == RS::PRIMITIVE_LINES;
                const bool line_strip = command->primitive == RS::PRIMITIVE_LINE_STRIP;
                if (!triangles && !strip && !lines && !line_strip) {
                    WARN_PRINT_ONCE("3DS: unsupported polygon topology skipped.");
                    continue;
                }
                const int count = data->indices.is_empty() ? data->points.size() : data->indices.size();
                if ((triangles && count % 3) || (lines && count % 2)) {
                    WARN_PRINT_ONCE("3DS: incomplete polygon indices skipped.");
                    continue;
                }
                const Color modulation = item->final_modulate * modulate;
                auto index_at = [&](int i) { return data->indices.is_empty() ? i : data->indices[i]; };
                auto color_at = [&](int i) {
                    const Color color = data->colors.is_empty() ? Color(1, 1, 1, 1) : data->colors[data->colors.size() == 1 ? 0 : i];
                    return pack_color((color * modulation).clamp());
                };
                if (lines || line_strip) {
                    C2D_Flush(); C2D_ViewReset(); have_transform = false;
                }
                const int vertices = (triangles || strip) ? 3 : 2;
                const int step = (strip || line_strip) ? 1 : vertices;
                bool drawn = true;
                for (int i = 0; i + vertices <= count; i += step) {
                    const int ia = index_at(i), ib = index_at(i + 1);
                    Vector2 a = data->points[ia], b = data->points[ib];
                    if (vertices == 3) {
                        const int ic = index_at(i + 2);
                        const Vector2 c = data->points[ic];
                        // Triangle strips include deliberate degenerate join triangles.
                        if ((b - a).cross(c - a) == 0) continue;
                        drawn = C2D_DrawTriangle(a.x, a.y, color_at(ia), b.x, b.y, color_at(ib), c.x, c.y, color_at(ic), 0);
                    } else {
                        a = xform.xform(a); b = xform.xform(b);
                        if (a == b) continue;
                        drawn = C2D_DrawLine(a.x, a.y, color_at(ia), b.x, b.y, color_at(ib), 1, 0);
                    }
                    if (!drawn) break;
                }
                if (!drawn) WARN_PRINT_ONCE("3DS: polygon draw capacity exceeded.");
                continue;
            }
            if (cmd->type == Item::Command::TYPE_PRIMITIVE) {
                const auto *primitive = static_cast<Item::CommandPrimitive *>(cmd);
                if (primitive->texture.is_valid() || primitive->point_count < 2 || primitive->point_count > 4) {
                    WARN_PRINT_ONCE("3DS: only untextured 2/3/4-point primitives supported.");
                    continue;
                }
                bool valid = true;
                u32 colors[4];
                for (uint32_t i = 0; i < primitive->point_count; i++) {
                    valid = valid && primitive->points[i].is_finite();
                    colors[i] = pack_color((primitive->colors[i] * item->final_modulate * modulate).clamp());
                }
                if (!valid) { WARN_PRINT_ONCE("3DS: non-finite primitive skipped."); continue; }
                const Vector2 *p = primitive->points;
                bool drawn;
                if (primitive->point_count == 2) {
                    // Negative-width Godot lines remain one screen pixel under scaling.
                    C2D_Flush(); C2D_ViewReset(); have_transform = false;
                    const Vector2 a = xform.xform(p[0]), b = xform.xform(p[1]);
                    drawn = a == b || C2D_DrawLine(a.x, a.y, colors[0], b.x, b.y, colors[1], 1, 0);
                } else {
                    drawn = C2D_DrawTriangle(p[0].x, p[0].y, colors[0], p[1].x, p[1].y, colors[1], p[2].x, p[2].y, colors[2], 0);
                    if (primitive->point_count == 4) {
                        drawn = C2D_DrawTriangle(p[0].x, p[0].y, colors[0], p[2].x, p[2].y, colors[2], p[3].x, p[3].y, colors[3], 0) && drawn;
                    }
                }
                if (!drawn) WARN_PRINT_ONCE("3DS: primitive draw capacity exceeded.");
                continue;
            }
            const auto *rect = static_cast<Item::CommandRect *>(cmd);
            const uint16_t supported = CANVAS_RECT_REGION | CANVAS_RECT_CLIP_UV | CANVAS_RECT_FLIP_H | CANVAS_RECT_FLIP_V;
            if (rect->flags & ~supported) {
                WARN_PRINT_ONCE("3DS canvas: unsupported rectangle flags; skipping.");
                continue;
            }
            const Vector2 pos = rect->rect.position;
            Vector2 size = rect->rect.size;
            const Color color = (rect->modulate * item->final_modulate * modulate).clamp();
            bool drawn = false;
            if (rect->texture.is_valid()) {
                auto *storage = static_cast<TextureStorage3DS *>(RSG::texture_storage);
                C2D_Image image;
                Tex3DS_SubTexture sub;
                Rect2 source = rect->source;
                if (!(rect->flags & CANVAS_RECT_REGION)) source = Rect2(Vector2(), storage->texture_size_with_proxy(rect->texture));
                if (!storage->get_image(rect->texture, source, image, sub)) continue;
                // Citro2D's negative draw dimensions flip UVs, not the local geometry.
                if (rect->flags & CANVAS_RECT_FLIP_H) size.x = -size.x;
                if (rect->flags & CANVAS_RECT_FLIP_V) size.y = -size.y;
                C2D_ImageTint tint;
                C2D_PlainImageTint(&tint, pack_color(color), 1.0f);
                C2D_DrawParams params = { { pos.x, pos.y, size.x, size.y }, { 0, 0 }, 0, 0 };
                drawn = C2D_DrawImage(image, &params, &tint);
                static bool reported = false;
                if (drawn && !reported) {
                    print_line("3DS canvas: first Godot textured rectangle submitted to Citro2D.");
                    reported = true;
                }
            } else {
                drawn = C2D_DrawRectSolid(pos.x, pos.y, 0, size.x, size.y, pack_color(color));
            }
            if (!drawn) WARN_PRINT_ONCE("3DS canvas: Citro2D draw capacity exceeded.");
        }
    }
    C2D_Flush();
    C2D_ViewReset();
    C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
}

void TextureStorage3DS::invalidate(RID rid) {
    GpuTexture **entry = gpu_textures.getptr(rid);
    if (!entry) return;
    if (*entry) retired.push_back(*entry);
    gpu_textures.erase(rid);
}

void TextureStorage3DS::collect_retired() {
    // Called only after frame synchronization: the GPU no longer reads these.
    for (GpuTexture *texture : retired) {
        gpu_bytes -= texture->texture.size;
        gpu_count--;
        frees++;
        C3D_TexDelete(&texture->texture);
        memdelete(texture);
    }
    retired.clear();
}

void TextureStorage3DS::print_memory_stats() const {
    printf("GPU tex %lu/%lu KiB up/free %lu/%lu\n", (unsigned long)gpu_count,
        (unsigned long)(gpu_bytes / 1024), (unsigned long)uploads, (unsigned long)frees);
    printf("Retired awaiting sync: %d\n", retired.size());
}

void TextureStorage3DS::release_gpu() {
    for (const KeyValue<RID, GpuTexture *> &entry : gpu_textures) {
        if (entry.value) retired.push_back(entry.value);
    }
    gpu_textures.clear();
    collect_retired();
}

void TextureStorage3DS::texture_2d_initialize(RID rid, const Ref<Image> &image) {
    ERR_FAIL_COND(image.is_null() || image->is_empty());
    invalidate(rid);
    RendererDummy::TextureStorage::texture_2d_initialize(rid, image);
}

void TextureStorage3DS::texture_free(RID rid) {
    invalidate(rid);
    RendererDummy::TextureStorage::texture_free(rid);
}

void TextureStorage3DS::texture_replace(RID rid, RID replacement) {
    if (rid == replacement) return;
    Ref<Image> image = texture_2d_get(replacement);
    texture_2d_initialize(rid, image);
    texture_free(replacement);
}

bool TextureStorage3DS::get_image(RID rid, const Rect2 &source, C2D_Image &out, Tex3DS_SubTexture &sub) {
    // Reject malformed regions before allocating/uploading. NaN bypasses ordinary
    // bounds comparisons and must never reach the GPU's texture coordinates.
    if (!source.position.is_finite() || !source.size.is_finite()) {
        WARN_PRINT_ONCE("3DS: non-finite texture region; skipping.");
        return false;
    }
    if (source.size.x == 0 || source.size.y == 0) return false;
    if (!owns_texture(rid)) { WARN_PRINT_ONCE("3DS: canvas textures/proxies unsupported."); return false; }
    if (!gpu_textures.has(rid)) {
        // Cache failure too, to avoid repeated allocations/errors each frame.
        gpu_textures.insert(rid, nullptr);
        Ref<Image> original = texture_2d_get(rid);
        ERR_FAIL_COND_V(original.is_null() || original->is_empty(), false);
        ERR_FAIL_COND_V_MSG(original->is_compressed(), false, "3DS: compressed GPU textures unsupported.");
        const int w = original->get_width(), h = original->get_height();
        ERR_FAIL_COND_V_MSG(w > 1024 || h > 1024, false, "3DS texture exceeds 1024 pixels.");
        Ref<Image> image = original->duplicate();
        image->convert(Image::FORMAT_RGBA8);
        int tw = 8, th = 8;
        while (tw < w) tw *= 2;
        while (th < h) th *= 2;
        GpuTexture *gpu = memnew(GpuTexture);
        if (!C3D_TexInit(&gpu->texture, tw, th, GPU_RGBA8)) {
            memdelete(gpu);
            ERR_FAIL_V_MSG(false, "3DS: insufficient linear memory for texture.");
        }
        gpu->width = w; gpu->height = h;
        memset(gpu->texture.data, 0, gpu->texture.size);
        const Vector<uint8_t> bytes = image->get_data();
        const uint8_t *src = bytes.ptr();
        uint8_t *dst = static_cast<uint8_t *>(gpu->texture.data);
        // PICA textures: 8x8 Morton tiles and ABGR bytes, matching tex3ds raw output.
        for (int y = 0; y < h; y++) {
            const unsigned ty = y;
            for (int x = 0; x < w; x++) {
                unsigned morton = 0;
                for (unsigned bit = 0; bit < 3; bit++) {
                    morton |= ((x >> bit) & 1) << (2 * bit);
                    morton |= ((ty >> bit) & 1) << (2 * bit + 1);
                }
                const unsigned offset = (((ty / 8) * (tw / 8) + x / 8) * 64 + morton) * 4;
                const unsigned from = (y * w + x) * 4;
                dst[offset] = src[from + 3];
                dst[offset + 1] = src[from + 2];
                dst[offset + 2] = src[from + 1];
                dst[offset + 3] = src[from];
            }
        }
        C3D_TexSetFilter(&gpu->texture, GPU_NEAREST, GPU_NEAREST);
        C3D_TexSetWrap(&gpu->texture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        C3D_TexFlush(&gpu->texture);
        gpu_textures[rid] = gpu;
        gpu_bytes += gpu->texture.size;
        gpu_count++;
        uploads++;
        print_line(vformat("3DS: uploaded Godot texture %dx%d (GPU %dx%d).", w, h, tw, th));
    }
    GpuTexture *gpu = gpu_textures[rid];
    if (!gpu) return false;
    if (source.position.x < 0 || source.position.y < 0 || source.size.x <= 0 || source.size.y <= 0 ||
        source.get_end().x > gpu->width || source.get_end().y > gpu->height) {
        WARN_PRINT_ONCE("3DS: texture region outside image; skipping.");
        return false;
    }
    // Region changes select UVs on the SAME cached texture. UV normalization uses
    // padded GPU dimensions, not image dimensions (important for NPOT atlases).
    // DrawParams supplies the actual destination size; round metadata up so a
    // fractional region cannot produce a zero-sized Tex3DS descriptor.
    sub = { (u16)Math::ceil(source.size.x), (u16)Math::ceil(source.size.y),
        source.position.x / gpu->texture.width, 1.0f - source.position.y / gpu->texture.height,
        source.get_end().x / gpu->texture.width, 1.0f - source.get_end().y / gpu->texture.height };
    out = { &gpu->texture, &sub };
    return true;
}
