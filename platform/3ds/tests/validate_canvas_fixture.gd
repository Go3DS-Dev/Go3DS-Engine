extends SceneTree

func _initialize() -> void:
    call_deferred("validate")

func validate() -> void:
    var cache := ConfigFile.new()
    assert(cache.load("res://.godot/global_script_class_cache.cfg") == OK)
    assert(cache.get_value("", "list") == [])
    root.size = Vector2i(400, 240)
    var packed: PackedScene = load("res://main.tscn")
    var scene := packed.instantiate()
    root.add_child(scene)
    scene.set_process(false)
    await process_frame
    if scene.has_node("Player/Camera2D"):
        var player: Sprite2D = scene.get_node("Player")
        var camera: Camera2D = scene.get_node("Player/Camera2D")
        assert(camera.is_current() and not camera.ignore_rotation)
        assert(player.texture.get_rid() == scene.get_node("Landmark").texture.get_rid())
        var hud: Node2D = scene.get_node("HUD/Marks")
        var hud_before := hud.get_global_transform_with_canvas()
        for i in range(12):
            scene._process(0.5)
            camera.force_update_scroll()
            var screen := player.get_global_transform_with_canvas().origin
            assert(screen.distance_to(Vector2(200, 120)) < 0.01)
            var canvas := scene.get_viewport().canvas_transform
            assert(is_equal_approx(canvas.x.length(), camera.zoom.x))
            assert(is_equal_approx(canvas.get_rotation(), -camera.rotation))
            assert(hud.get_global_transform_with_canvas().is_equal_approx(hud_before))
        print("CAMERA PASS: follow center 200x120, zoom, rotation, fixed HUD, shared texture across 12 samples.")
    else:
        assert(scene.get_node("Sprite2D").texture.get_size() == Vector2(32, 32))
        assert(scene.get_node("Foreground").z_index > scene.get_node("Sprite2D").z_index)
        assert(scene.get_node("Foreground").get_index() < scene.get_node("Sprite2D").get_index())
        assert(scene.get_node("ClipBox").clip_contents)
        assert(scene.get_node("ClipBox").size == Vector2(80, 90))
        scene._process(1.0)
        assert(is_equal_approx(scene.get_node("Rotor").rotation, 0.6))
        assert(scene.get_node("Rotor").scale == Vector2(1.2, 0.8))
        assert(is_equal_approx(scene.get_node("ClipBox/Shapes").rotation, -0.4))
        print("PRIMITIVES FIXTURE PASS: ready/draw callbacks, texture, order, clipping, animated rotation/scale.")
    scene.free()
    quit(0)
