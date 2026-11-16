extends SceneTree

# Run with desktop Godot --headless --main-pack <Game.pck> --script <this file>.
# Checks project semantics, not the 3DS GPU backend.
func _initialize() -> void:
    call_deferred("validate")

func validate() -> void:
    var cache := ConfigFile.new()
    assert(cache.load("res://.godot/global_script_class_cache.cfg") == OK)
    assert(cache.get_value("", "list") == [])
    assert(ProjectSettings.get_setting("display/window/size/viewport_width") == 400)
    assert(ProjectSettings.get_setting("display/window/size/viewport_height") == 240)
    var packed: PackedScene = load("res://main.tscn")
    var scene := packed.instantiate()
    root.add_child(scene)
    scene.set_process(false)
    var sprites := get_nodes_in_group("fixture_sprites")
    assert(sprites.size() == 9)
    var shared: Texture2D = sprites[0].texture
    assert(shared != null and shared.get_size() == Vector2(32, 32))
    for sprite in sprites:
        assert(sprite.texture.get_rid() == shared.get_rid())
    assert(scene.get_node("FlipH").flip_h)
    assert(scene.get_node("FlipV").flip_v)
    assert(scene.get_node("NegativeX").scale == Vector2(-1, 1))
    assert(scene.get_node("NegativeY").scale == Vector2(1, -1))
    assert(scene.get_node("ClipBox").clip_contents)
    assert(scene.get_node("ClipBox").size == Vector2(96, 80))
    var front: Sprite2D = scene.get_node("Front")
    var back: Sprite2D = scene.get_node("Back")
    assert(front.get_index() < back.get_index() and front.z_index > back.z_index)
    assert(is_equal_approx(front.modulate.a, 0.65))
    var image := shared.get_image()
    assert(image.get_pixel(16, 16).a == 0.0)
    assert(is_equal_approx(image.get_pixel(24, 24).a, 128.0 / 255.0))
    var before: Vector2 = scene.get_node("Sprite2D").position
    scene._process(1.0)
    assert(scene.get_node("Sprite2D").position != before)
    assert(is_equal_approx(scene.get_node("Pivot").rotation, 0.7))
    assert(is_equal_approx(scene.get_node("ClipBox/ClippedSprite").rotation, -0.5))
    print("FIXTURE PASS: cache, native size, 9 shared sprites, transforms, flips, alpha, clip setup, Z order and animation.")
    scene.free()
    quit(0)
