extends Node2D

var elapsed := 0.0
@onready var mover: Sprite2D = $Sprite2D
@onready var pivot: Node2D = $Pivot
@onready var clipped: Sprite2D = $ClipBox/ClippedSprite

func _ready() -> void:
    get_viewport().disable_3d = true
    # All sprites share one Godot texture RID: animation must not cause uploads.
    var image := Image.new()
    var error := image.load_png_from_buffer(FileAccess.get_file_as_bytes("res://sprite.png"))
    if error != OK:
        push_error("Sprite PNG decode failed: %s" % error)
        set_process(false)
        return
    var shared_texture := ImageTexture.create_from_image(image)
    for sprite in get_tree().get_nodes_in_group("fixture_sprites"):
        sprite.texture = shared_texture
    print("2D checkpoint: 9 sprites, one shared 32x32 texture.")
    print("Top row: rotate, flip H/V, negative X/Y.")
    print("Lower left: tinted front overlaps back.")
    print("Lower right: rotating sprite clipped to white frame.")
    print("FPS / memory every 5 seconds. START exits.")

func _draw() -> void:
    draw_rect(Rect2(40, 30, 64, 48), Color.GREEN)
    # Four filled borders avoid requiring line/polygon renderer commands.
    draw_rect(Rect2(258, 128, 100, 84), Color.WHITE)
    draw_rect(Rect2(260, 130, 96, 80), Color(0.08, 0.08, 0.15))

func _process(delta: float) -> void:
    elapsed += delta
    mover.position.x = 180.0 + sin(elapsed) * 35.0
    pivot.rotation = elapsed * 0.7
    clipped.position = Vector2(48.0 + sin(elapsed * 0.9) * 65.0, 40.0)
    clipped.rotation = -elapsed * 0.5
