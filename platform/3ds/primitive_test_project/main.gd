extends Node2D
var elapsed := 0.0
@onready var rotor: Node2D = $Rotor
@onready var clipped: Node2D = $ClipBox/Shapes

func _ready() -> void:
    get_viewport().disable_3d = true
    var image := Image.new()
    var error := image.load_png_from_buffer(FileAccess.get_file_as_bytes("res://sprite.png"))
    if error != OK:
        push_error("Sprite decode failed: %s" % error)
        return
    $Sprite2D.texture = ImageTexture.create_from_image(image)
    print("Milestone B: lines, outlines, circles, filled polygon.")
    print("Top: horizontal/vertical/diagonal lines, widths 1/3/6.")
    print("Middle: circle, concave arrow, rotating shapes.")
    print("Bottom left: shape / sprite / translucent shape.")
    print("Right white frame clips moving circle and line.")
    print("One texture upload, GPU 4 KiB. START exits.")

func _draw() -> void:
    draw_line(Vector2(15, 20), Vector2(100, 20), Color.WHITE, -1.0)
    draw_line(Vector2(15, 35), Vector2(100, 35), Color.YELLOW, 3.0)
    draw_line(Vector2(15, 55), Vector2(100, 70), Color.RED, 6.0)
    draw_line(Vector2(110, 20), Vector2(110, 70), Color.CYAN, 2.0)
    draw_rect(Rect2(130, 20, 40, 30), Color.GREEN)
    draw_rect(Rect2(190, 20, 40, 30), Color.WHITE, false, 3.0)
    draw_rect(Rect2(250, 20, 40, 30), Color.YELLOW, false, -1.0)
    draw_circle(Vector2(45, 105), 22.0, Color(1, 0, 1, 0.7))
    draw_colored_polygon(PackedVector2Array([Vector2(90, 85), Vector2(120, 85), Vector2(120, 75), Vector2(150, 105), Vector2(120, 130), Vector2(120, 120), Vector2(90, 120)]), Color(1, 0.6, 0, 0.8))
    draw_rect(Rect2(25, 150, 110, 65), Color(0, 0.7, 0.6, 1))
    draw_rect(Rect2(298, 123, 84, 94), Color.WHITE)
    draw_rect(Rect2(300, 125, 80, 90), Color(0.08, 0.08, 0.15))

func _process(delta: float) -> void:
    elapsed += delta
    rotor.rotation = elapsed * 0.6
    clipped.position.x = 40.0 + sin(elapsed) * 40.0
    clipped.rotation = elapsed * -0.4
