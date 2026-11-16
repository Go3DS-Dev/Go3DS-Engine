extends Node2D
@export var kind := 0

func _draw() -> void:
    if kind == 0:
        draw_rect(Rect2(-24, -16, 48, 32), Color(0.1, 1, 0.5, 0.5))
        draw_rect(Rect2(-24, -16, 48, 32), Color.WHITE, false, 3.0)
        draw_line(Vector2(-30, -20), Vector2(30, 20), Color.YELLOW, 4.0)
    elif kind == 1:
        draw_circle(Vector2.ZERO, 28.0, Color(1, 0.3, 0, 0.65))
        draw_line(Vector2(-50, -25), Vector2(50, 25), Color.WHITE, 5.0)
    else:
        draw_rect(Rect2(-35, -10, 70, 20), Color(1, 1, 0, 0.55))
        draw_line(Vector2(-40, 0), Vector2(40, 0), Color.WHITE, -1.0)
