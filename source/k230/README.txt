yolo.draw_result(res,pl.osd_img)
这句话是yolo画框，会遮盖原本画布，所以要想在原本画布上作图必须把它放在推理之后立即画框。

res返回的坐标是yolo坐标系的，要映射回原来的坐标系