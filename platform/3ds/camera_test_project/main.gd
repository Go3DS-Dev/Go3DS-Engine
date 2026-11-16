extends Node2D
var elapsed := 0.0
@onready var player: Sprite2D = $Player
@onready var camera: Camera2D = $Player/Camera2D

func _ready() -> void:
    get_viewport().disable_3d = true
    var image := Image.new()
    var error := image.load_png_from_buffer(FileAccess.get_file_as_bytes("res://sprite.png"))
    if error != OK:
        push_error("Sprite decode failed: %s" % error)
        return
    var shared := ImageTexture.create_from_image(image)
    player.texture = shared
    $Landmark.texture = shared
    camera.make_current()
    camera.force_update_scroll()
    print("Milestone C: actual Camera2D follows moving sprite.")
    print("Player stays at center; world scrolls, zooms and rotates.")
    print("White center ticks and green corner HUD stay fixed.")
    print("One texture upload, GPU 4 KiB. START exits.")

func _draw() -> void:
    for x in range(-200, 801, 80):
        draw_line(Vector2(x, -200), Vector2(x, 500), Color(0.35, 0.45, 0.8), 1.0)
    for y in range(-200, 501, 80):
        draw_line(Vector2(-200, y), Vector2(800, y), Color(0.35, 0.45, 0.8), 1.0)
    draw_rect(Rect2(40, 40, 70, 45), Color.GREEN)
    draw_rect(Rect2(310, 170, 85, 55), Color.YELLOW, false, 4.0)
    draw_circle(Vector2(350, 40), 25.0, Color(1, 0.1, 0.3, 0.7))

func _process(delta: float) -> void:
    elapsed += delta
    player.position = Vector2(200 + sin(elapsed * 0.5) * 140, 120 + sin(elapsed * 0.8) * 65)
    var amount := 1.0 + sin(elapsed * 0.35) * 0.25
    camera.zoom = Vector2(amount, amount)
    camera.rotation = sin(elapsed * 0.4) * 0.3
    camera.force_update_scroll()
