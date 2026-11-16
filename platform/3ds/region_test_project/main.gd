extends Node2D

var elapsed := 0.0
var atlas: ImageTexture
var atlas_frame := 0
@onready var moving: Sprite2D = $Moving
@onready var rotating: Sprite2D = $Rotating
@onready var clipped: Sprite2D = $ClipBox/Clipped

func _ready() -> void:
    get_viewport().disable_3d = true
    var image := Image.new()
    var error := image.load_png_from_buffer(FileAccess.get_file_as_bytes("res://atlas.png"))
    if error != OK:
        push_error("Atlas decode failed: %s" % error)
        set_process(false)
        return
    atlas = ImageTexture.create_from_image(image)
    for sprite in get_tree().get_nodes_in_group("region_sprites"):
        sprite.texture = atlas
    queue_redraw()
    print("Milestone A: 16 Sprite2D regions, one 96x48 atlas.")
    print("Top: red green blue yellow cyan magenta.")
    print("White L upper-left; clear center; translucent bottom.")
    print("Moving sprite changes region once/sec, no new uploads.")
    print("Bottom: H/V flips, negative X/Y. Right: clipping.")
    print("Expected GPU tex 1/32 KiB up/free 1/0. START exits.")

func _draw() -> void:
    draw_rect(Rect2(258, 128, 100, 84), Color.WHITE)
    draw_rect(Rect2(260, 130, 96, 80), Color(0.08, 0.08, 0.15))
    draw_rect(Rect2(15, 145, 64, 24), Color.GREEN)
    if atlas != null:
        # Direct CanvasItem source rectangle, independently of Sprite2D nodes.
        draw_texture_rect_region(atlas, Rect2(175, 145, 64, 36), Rect2(64, 24, 32, 24))

func _process(delta: float) -> void:
    elapsed += delta
    moving.position.x = 155.0 + sin(elapsed) * 30.0
    rotating.rotation = elapsed * 0.7
    clipped.position.x = 48.0 + sin(elapsed * 0.9) * 65.0
    clipped.rotation = -elapsed * 0.5
    var next_frame := int(elapsed) % 6
    if next_frame != atlas_frame:
        atlas_frame = next_frame
        moving.region_rect = Rect2((atlas_frame % 3) * 32, (atlas_frame / 3) * 24, 32, 24)
