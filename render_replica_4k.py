import bpy
import os
import math
import random
from mathutils import Vector
from datetime import datetime

# ==========================================
# 1. USER CONFIGURATION
# ==========================================
ply_path = r"C:\Users\fkuli\OneDrive\Pulpit\Replica-Dataset\replica-data\room_0\mesh.ply"
output_dir = r"C:\Users\fkuli\OneDrive\Pulpit\Replica-Dataset\replica-output"
num_renders = 10 # How many random viewpoints do you want to generate?

os.makedirs(output_dir, exist_ok=True)

# Clean up default scene
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

# ==========================================
# 2. IMPORT & LIGHTING FIX (Shadeless Material)
# ==========================================
print(f"Importing mesh: {ply_path}...")
try:
    bpy.ops.wm.ply_import(filepath=ply_path)
except AttributeError:
    bpy.ops.import_mesh.ply(filepath=ply_path)

mesh_obj = bpy.context.active_object

print("Applying vertex colors to emission materials...")
mat = bpy.data.materials.new(name="Replica_Baked")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
nodes.clear()

out_node = nodes.new('ShaderNodeOutputMaterial')
emission_node = nodes.new('ShaderNodeEmission')
attr_node = nodes.new('ShaderNodeAttribute')

# FIX: Dynamically find the exact name of the PLY color attribute
if mesh_obj.data.color_attributes:
    color_attr_name = mesh_obj.data.color_attributes[0].name
    print(f"SUCCESS: Found vertex colors named '{color_attr_name}'")
    attr_node.attribute_name = color_attr_name
else:
    print("CRITICAL WARNING: This PLY file has no vertex colors baked into it!")
    # Fallback to white so it at least renders a silhouette instead of black
    emission_node.inputs['Color'].default_value = (1.0, 1.0, 1.0, 1.0)

links.new(attr_node.outputs['Color'], emission_node.inputs['Color'])
links.new(emission_node.outputs['Emission'], out_node.inputs['Surface'])

if len(mesh_obj.data.materials) == 0:
    mesh_obj.data.materials.append(mat)
else:
    mesh_obj.data.materials[0] = mat

# ==========================================
# 3. CONFIGURE ENGINE & 5.1 COMPOSITOR
# ==========================================
scene = bpy.context.scene
scene.render.engine = 'CYCLES'
prefs = bpy.context.preferences.addons['cycles'].preferences
prefs.compute_device_type = 'OPTIX'

for device in prefs.devices:
    if device.type == 'OPTIX':
        device.use = True

scene.cycles.device = 'GPU'
scene.cycles.samples = 128  

# Disable all light bounces to stop the emission shader from illuminating the room
scene.cycles.max_bounces = 0
scene.cycles.diffuse_bounces = 0
scene.cycles.glossy_bounces = 0
scene.cycles.transmission_bounces = 0
scene.cycles.volume_bounces = 0
scene.cycles.transparent_max_bounces = 0

# Set 4K Resolution
scene.render.resolution_x = 8192
scene.render.resolution_y = 4096
scene.render.resolution_percentage = 100

# Setup 5.1 Node Tree for Depth
scene.view_layers["ViewLayer"].use_pass_z = True
scene.render.image_settings.file_format = 'PNG'
scene.render.image_settings.color_depth = '8'

# Prevent Blender from double-tonemapping the pre-baked vertex colors
scene.view_settings.view_transform = 'Standard'
scene.view_settings.look = 'None'

tree = bpy.data.node_groups.new("ReplicaComp", "CompositorNodeTree")
scene.compositing_node_group = tree
links = tree.links
tree.interface.new_socket("Image", in_out='OUTPUT', socket_type='NodeSocketColor')

render_layers = tree.nodes.new(type="CompositorNodeRLayers")
group_output = tree.nodes.new(type="NodeGroupOutput") 
links.new(render_layers.outputs['Image'], group_output.inputs['Image'])

file_output_depth = tree.nodes.new(type="CompositorNodeOutputFile")
file_output_depth.directory = output_dir
depth_item = file_output_depth.file_output_items.new('FLOAT', "Depth")
links.new(render_layers.outputs['Depth'], file_output_depth.inputs[0])

# Setup Camera
cam_data = bpy.data.cameras.new("PanoCam")
cam_data.type = 'PANO'
cam_data.panorama_type = 'EQUIRECTANGULAR'
cam_obj = bpy.data.objects.new("PanoCam", cam_data)
scene.collection.objects.link(cam_obj)
scene.camera = cam_obj

# ==========================================
# 4. THE VIRTUAL SURVEYOR (Vertex Sampling)
# ==========================================
print("Surveying room geometry to find the floor...")

# 1. Get all vertices and transform them to world coordinates
vertices = mesh_obj.data.vertices
matrix = mesh_obj.matrix_world
world_verts = [matrix @ v.co for v in vertices]

# 2. Find the absolute lowest point in the scan (the floor level)
min_z = min([v.z for v in world_verts])
print(f"Detected floor level at Z = {min_z:.2f} meters.")

# 3. Filter all vertices to only keep the ones within 30cm of the floor
floor_points = [v for v in world_verts if v.z <= (min_z + 0.3)]

valid_locations = []
for _ in range(num_renders):
    if not floor_points:
        print("Error: Could not detect floor geometry.")
        break
        
    # 4. Pick a random physical point on the floor
    chosen_vert = random.choice(floor_points)
    
    # 5. Elevate camera 1.5m (human eye level) above that exact floor spot
    cam_loc = Vector((chosen_vert.x, chosen_vert.y, chosen_vert.z + 1.5))
    valid_locations.append(cam_loc)

# ==========================================
# 5. EXECUTE BATCH RENDER
# ==========================================
# ADDED: Generate a single timestamp for the entire batch
session_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

for i, loc in enumerate(valid_locations):
    print(f"\n--- Rendering View {i+1} of {len(valid_locations)} ---")
    print(f"Camera Coordinates: X={loc.x:.2f}, Y={loc.y:.2f}, Z={loc.z:.2f}")
    
    cam_obj.location = loc
    # Randomly rotate the camera horizontally for variety
    cam_obj.rotation_euler = (math.radians(90), 0.0, random.uniform(0, math.pi * 2))
    
    # MODIFIED: Insert timestamp into both filenames
    scene.render.filepath = os.path.join(output_dir, f"rgb_4k_{session_timestamp}_view_{i:03d}")
    file_output_depth.file_name = f"depth_4k_{session_timestamp}_view_{i:03d}_"
    
    bpy.ops.render.render(write_still=True) 

print("\nBatch Rendering Complete!")