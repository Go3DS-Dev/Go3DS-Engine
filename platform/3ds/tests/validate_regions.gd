extends SceneTree

# Desktop fixture validation only; not a test of Citro2D output.
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
    var sprites := get_nodes_in_group("region_sprites")
    assert(sprites.size() == 16)
    var atlas: Texture2D = sprites[0].texture
    assert(atlas.get_size() == Vector2(96, 48))
    for sprite in sprites:
        assert(sprite.texture.get_rid() == atlas.get_rid())
        assert(sprite.region_enabled)
        assert(Rect2(0, 0, 96, 48).encloses(sprite.region_rect))
    var colors := [Color.RED, Color.GREEN, Color.BLUE, Color.YELLOW, Color.CYAN, Color.MAGENTA]
    var image := atlas.get_image()
    for i in range(6):
        var region := Rect2((i % 3) * 32, (i / 3) * 24, 32, 24)
        assert(scene.get_node("Tile%d" % i).region_rect == region)
        var x := int(region.position.x)
        var y := int(region.position.y)
        assert(image.get_pixel(x + 20, y + 8) == colors[i])
        assert(image.get_pixel(x + 3, y + 3) == Color.WHITE)
        assert(image.get_pixel(x + 16, y + 12).a == 0.0)
        assert(is_equal_approx(image.get_pixel(x + 20, y + 20).a, 128.0 / 255.0))
    assert(scene.get_node("Crop").region_rect == Rect2(40, 6, 16, 12))
    assert(scene.get_node("FlipH").flip_h and scene.get_node("FlipV").flip_v)
    assert(scene.get_node("NegativeX").scale == Vector2(-1, 1))
    assert(scene.get_node("NegativeY").scale == Vector2(1, -1))
    assert(scene.get_node("ClipBox").clip_contents)
    assert(scene.get_node("Front").z_index > scene.get_node("Back").z_index)
    assert(scene.get_node("Front").get_index() < scene.get_node("Back").get_index())
    var moving: Sprite2D = scene.get_node("Moving")
    for frame in range(1, 13):
        scene._process(1.0)
        var expected := frame % 6
        assert(moving.region_rect == Rect2((expected % 3) * 32, (expected / 3) * 24, 32, 24))
        assert(moving.texture.get_rid() == atlas.get_rid())
    assert(scene.get_node("Rotating").rotation != 0.0)
    assert(scene.get_node("ClipBox/Clipped").rotation != 0.0)
    print("REGIONS PASS: cache, 400x240, 16 shared sprites, six atlas tiles, crop, transforms/flags, alpha, clip, order, 12 region changes.")
    scene.free()
    quit(0)
