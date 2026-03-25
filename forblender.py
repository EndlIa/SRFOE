import bpy
import math
import numpy as np
cam = bpy.context.scene.camera
cam_matrix = np.array(cam.matrix_world)
print(cam_matrix)
v_fov_rad = cam.data.angle_y
v_fov_deg = math.degrees(v_fov_rad)
print(f"竖直FOV: {v_fov_rad:.4f} rad / {v_fov_deg:.2f} deg")