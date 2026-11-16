// Minimal single-screen 2D backend. Unsupported subsystems retain dummy storage.
#ifndef RASTERIZER_3DS_H
#define RASTERIZER_3DS_H
#include "core/templates/rid_owner.h"
#include "core/templates/self_list.h"
#include "scene/resources/mesh.h"
#include "servers/rendering/dummy/environment/fog.h"
#include "servers/rendering/dummy/environment/gi.h"
#include "servers/rendering/dummy/rasterizer_canvas_dummy.h"
#include "servers/rendering/dummy/rasterizer_scene_dummy.h"
#include "servers/rendering/dummy/storage/light_storage.h"
#include "servers/rendering/dummy/storage/material_storage.h"
#include "servers/rendering/dummy/storage/mesh_storage.h"
#include "servers/rendering/dummy/storage/particles_storage.h"
#include "servers/rendering/dummy/storage/texture_storage.h"
#include "servers/rendering/dummy/storage/utilities.h"
#include "servers/rendering/renderer_compositor.h"
#include "servers/rendering_server.h"


#include <citro2d.h>

class TextureStorage3DS : public RendererDummy::TextureStorage {
    struct Target {
        Size2i size;
        Color clear = Color(0.12549, 0.25098, 0.75294, 1);
        bool clear_requested = false;
    };
    mutable RID_Owner<Target> targets;
    struct GpuTexture { C3D_Tex texture = {}; int width = 0, height = 0; };
    HashMap<RID, GpuTexture *> gpu_textures;
    Vector<GpuTexture *> retired;
    uint32_t gpu_bytes = 0, gpu_count = 0, uploads = 0, frees = 0;
    void invalidate(RID rid);
public:
    bool get_image(RID rid, const Rect2 &source, C2D_Image &image, Tex3DS_SubTexture &sub);
    void collect_retired();
    void release_gpu();
    void print_memory_stats() const;
    void texture_2d_initialize(RID rid, const Ref<Image> &image) override;
    void texture_2d_update(RID rid, const Ref<Image> &image, int layer = 0) override { texture_2d_initialize(rid, image); }
    void texture_free(RID rid) override;
    void texture_replace(RID rid, RID replacement) override;
    Size2 texture_size_with_proxy(RID rid) override {
        if (!owns_texture(rid)) return Size2();
        Ref<Image> image = texture_2d_get(rid);
        return image.is_valid() ? Size2(image->get_width(), image->get_height()) : Size2();
    }

public:
    RID render_target_create() override { return targets.make_rid(Target()); }
    void render_target_free(RID rid) override { targets.free(rid); }
    void render_target_set_size(RID rid, int w, int h, uint32_t views) override {
        Target *t = targets.get_or_null(rid);
        ERR_FAIL_NULL(t);
        t->size = Size2i(w, h);
    }
    Size2i render_target_get_size(RID rid) const override {
        const Target *t = targets.get_or_null(rid);
        return t ? t->size : Size2i();
    }
    void render_target_request_clear(RID rid, const Color &color) override {
        Target *t = targets.get_or_null(rid);
        ERR_FAIL_NULL(t);
        t->clear = color;
        t->clear_requested = true;
    }
    bool render_target_is_clear_requested(RID rid) override {
        const Target *t = targets.get_or_null(rid);
        return t && t->clear_requested;
    }
    Color render_target_get_clear_request_color(RID rid) override {
        const Target *t = targets.get_or_null(rid);
        return t ? t->clear : Color();
    }
    void render_target_disable_clear_request(RID rid) override {
        Target *t = targets.get_or_null(rid);
        if (t) t->clear_requested = false;
    }
    void render_target_do_clear_request(RID rid) override;
};

class Canvas3DS : public RasterizerCanvasDummy {
    struct PolygonData {
        Vector<int> indices;
        Vector<Point2> points;
        Vector<Color> colors;
    };
    HashMap<PolygonID, PolygonData> polygons;
    PolygonID next_polygon = 1;
public:
    PolygonID request_polygon(const Vector<int> &indices, const Vector<Point2> &points,
        const Vector<Color> &colors, const Vector<Point2> &uvs = Vector<Point2>(),
        const Vector<int> &bones = Vector<int>(), const Vector<float> &weights = Vector<float>()) override;
    void free_polygon(PolygonID id) override { polygons.erase(id); }
    void canvas_render_items(RID target, Item *items, const Color &modulate,
        Light *lights, Light *directional, const Transform2D &canvas_transform,
        RS::CanvasItemTextureFilter filter, RS::CanvasItemTextureRepeat repeat,
        bool snap, bool &sdf_used, RenderingMethod::RenderInfo *info = nullptr) override;
};

class Utilities3DS : public RendererDummy::Utilities {
public:
    bool has_os_feature(const String &feature) const override { return false; }
    Size2i get_maximum_viewport_size() const override { return Size2i(400, 240); }
    String get_video_adapter_name() const override { return "Nintendo 3DS / Citro2D"; }
};

class Rasterizer3DS : public RendererCompositor {
    uint64_t frame = 0;
    double delta = 0, time = 0;
    uint64_t stats_start_ms = 0;
    uint32_t stats_frames = 0;
    double cpu_ms = 0, gpu_ms = 0;
    float command_peak = 0;
    Canvas3DS canvas;
    Utilities3DS utilities;
    RendererDummy::LightStorage lights;
    RendererDummy::MaterialStorage materials;
    RendererDummy::MeshStorage meshes;
    RendererDummy::ParticlesStorage particles;
    TextureStorage3DS textures;
    RendererDummy::GI gi;
    RendererDummy::Fog fog;
    RasterizerSceneDummy scene;
    static C3D_RenderTarget *top;
    static bool active_frame;
    static RendererCompositor *create_backend() { return memnew(Rasterizer3DS); }
public:
    static bool initialize_gpu();
    static void make_current() { _create_func = create_backend; low_end = true; }
    static C3D_RenderTarget *get_top() { return top; }
    static bool is_frame_active() { return active_frame; }
    RendererUtilities *get_utilities() override { return &utilities; }
    RendererLightStorage *get_light_storage() override { return &lights; }
    RendererMaterialStorage *get_material_storage() override { return &materials; }
    RendererMeshStorage *get_mesh_storage() override { return &meshes; }
    RendererParticlesStorage *get_particles_storage() override { return &particles; }
    RendererTextureStorage *get_texture_storage() override { return &textures; }
    RendererGI *get_gi() override { return &gi; }
    RendererFog *get_fog() override { return &fog; }
    RendererCanvasRender *get_canvas() override { return &canvas; }
    RendererSceneRender *get_scene() override { return &scene; }
    void set_boot_image(const Ref<Image> &, const Color &, bool, bool = true) override {}
    void initialize() override {}
    void begin_frame(double step) override;
    void blit_render_targets_to_screen(int, const BlitToScreen *, int) override {}
    void end_viewport(bool) override {}
    void end_frame(bool swap) override;
    void finalize() override;
    uint64_t get_frame_number() const override { return frame; }
    double get_frame_delta_time() const override { return delta; }
    double get_total_time() const override { return time; }
};
#endif
