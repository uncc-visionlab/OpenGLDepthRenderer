import bpy
from geometry import Point3D, Point2D
import time
from visibility_volume import VisibilityVolume


# Blender control functions
def set_view_frustum_end(distance=10000):
    # Set the view frustum end distance for the active camera or 3D view
    for area in bpy.context.screen.areas:
        if area.type == 'VIEW_3D':
            # Loop through all regions in the area to find the region type 'WINDOW'
            for region in area.regions:
                if region.type == 'WINDOW':
                    # Get the 3D view space and set the clip end
                    space = area.spaces.active
                    if space.type == 'VIEW_3D':
                        space.clip_end = distance
                        print(f"View frustum end set to {distance}")
                    break


def select_and_frame_objects(object_names):
    # Deselect all objects first
    bpy.ops.object.select_all(action='DESELECT')

    # Select the specified objects by their names
    for obj_name in object_names:
        obj = bpy.data.objects.get(obj_name)
        if obj:
            obj.select_set(True)
        else:
            print(f"Object '{obj_name}' not found.")

    # Fit the selected objects to the active view
    for area in bpy.context.screen.areas:
        if area.type == 'VIEW_3D':
            for region in area.regions:
                if region.type == 'WINDOW':
                    override = {'area': area, 'region': region, 'edit_object': bpy.context.edit_object}
                    # bpy.ops.view3d.view_all(override)
                    # bpy.ops.view3d.view_selected(use_all_regions=False)
                    override = {'area': area, 'region': region}
                    # bpy.ops.view3d.view_all(override, center=False)
                    # bpy.ops.view3d.view_all(override, 'INVOKE_DEFAULT')
                    with bpy.context.temp_override(**override):
                        # bpy.ops.view3d.view_all(center=False)
                        bpy.ops.view3d.view_all(use_all_regions=False, center=False)
    print(f"Selected objects {object_names} framed to the active view.")


def recursive(count=0):
    # areas = [area for area in bpy.context.screen.areas if area.type == 'VIEW_3D']
    # if areas:
    #     regions = [region for region in areas[0].regions if region.type == 'WINDOW']
    #
    # if regions:
    #     override = {'area': areas[0],
    #                 'region': regions[0]}
    #     view_all_fn(areas[0], regions[0])
    #     # bpy.ops.view3d.view_all(override, center=False)
    #     # bpy.ops.view3d.view_all(use_all_regions=False, center=False)
    if context is not None:
        if hasattr(context, "screen"):
            if hasattr(context.screen, "areas"):
                for area in context.screen.areas:
                    if area.type == 'VIEW_3D':
                        break
                for region in area.regions:
                    if region.type == 'WINDOW':
                        break
                view_all_fn(area, region)
                return

    if count < 20:
        count += 1
        Timer(0.1, recursive, (count,)).start()


def view_all_fn(area, region):
    override = {'area': area, 'region': region}
    # bpy.ops.view3d.view_all(override, center=False)
    # bpy.ops.view3d.view_all(override, 'INVOKE_DEFAULT')
    with context.temp_override(**override):
        # bpy.ops.view3d.view_all(center=False)
        bpy.ops.view3d.view_all(use_all_regions=False, center=False)
    return


class ConvexPolygonVisibilityVolume:
    def __init__(self):
        # Initialize default configuration parameters
        self.parameters = {
        }
        print("ConvexPolygonVisibilityVolume initialized with default configuration.")

    def clear_scene(self):
        """Clears all objects in the current Blender scene."""
        bpy.ops.object.select_all(action='DESELECT')
        bpy.ops.object.select_by_type(type='MESH')
        bpy.ops.object.delete()

    def import_obj(self, filepath):
        """Imports an OBJ file into Blender."""
        bpy.ops.import_scene.obj(filepath=filepath)
        return bpy.context.selected_objects[0]

    def perform_intersection(self, obj1, obj2):
        """Performs boolean intersection between two objects."""
        # Ensure both objects are selected
        obj1.select_set(True)
        obj2.select_set(True)
        bpy.context.view_layer.objects.active = obj1

        # Add a boolean modifier to the first object
        bool_mod = obj1.modifiers.new(name='Boolean', type='BOOLEAN')
        bool_mod.operation = 'INTERSECT'
        bool_mod.use_self = True
        bool_mod.object = obj2

        # Apply the boolean modifier
        bpy.ops.object.modifier_apply(modifier='Boolean')

    def export_obj(self, filepath):
        """Exports the active object to an OBJ file."""
        bpy.ops.export_scene.obj(filepath=filepath)

    def intersect_objs_and_export(self, obj_file_1, obj_file_2, output_file):
        """Main function to handle the intersection and export."""
        clear_scene()
        object_1 = import_obj(obj_file_1)
        object_2 = import_obj(obj_file_2)

        perform_intersection(object_1, object_2)

        # Optionally remove the second object (it has been intersected)
        bpy.data.objects.remove(object_2)

        export_obj(output_file)
        print(f'Intersection saved to {output_file}')

# Example usage of the ConvexPolygonVisibilityVolume class
if __name__ == "__main__":
    # Create an instance of the VisibilityVolume class
    convex_polygon_visibility_volume = ConvexPolygonVisibilityVolume()
    data_path_prefix = "/home/arwillis/CLionProjects/visibility/data/"
    prog_path_prefix = "/home/arwillis/CLionProjects/visibility/"
    input_data_path = data_path_prefix + "inputs/"
    output_data_path = data_path_prefix + "results/"
    visibility_prog_path_root = prog_path_prefix + "OpenGLDepthRenderer/cmake-build-debug/bin/"
    visibility_prog_path_and_filename = visibility_prog_path_root + "ogl_depthrenderer"
    vertex1 = Point3D(-110.9651107788086, 1.0, 1527.08544921875)
    vertex2 = Point3D(-80.03492287079642, 1.0, 1526.6367639369446)
    vertex3 = Point3D(-88.3062336621307, 1.0, 1499.6531723345058)
    convex_polygon_vertices = [vertex1, vertex2, vertex3]

    segment_index = 1
    vertex_index = 1
    # Configure the visibility volume with custom parameters
    visibility_volume_config = {
        "world_obj_filename": input_data_path + "destin_and_miramar_beach.obj",
        "output_obj_filename": output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj",
        "visibility_volume_index": f"{vertex_index}",
        "visibility_volume_vertex": Point3D(10, 10, 10),  # Default dimensions (length, width, height)
        "visibility_volume_mesh_resolution": Point2D(40, 40),
        "center": (0, 0, 0),  # Center of the volume
        "visibility_height_threshold": 0.5,  # Default threshold for visibility
        "visibility_prog_path_root": visibility_prog_path_root,
        "visibility_prog_path_and_filename": visibility_prog_path_and_filename,
        "max_radius": 20
    }

    visibility_volume_obj_file_list = []

    vertex_index = 1
    visibility_volume_config["visibility_volume_index"] = f"{vertex_index}"
    visibility_volume_config["visibility_volume_vertex"] = convex_polygon_vertices[0]
    visibility_volume_config["output_obj_filename"] = \
         output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj"
    visibility_volume_obj_file_list.append(visibility_volume_config["output_obj_filename"])
    volume = VisibilityVolume()
    volume.configure(**visibility_volume_config)
    volume.evaluate()

    vertex_index = 2
    visibility_volume_config["visibility_volume_index"] = f"{vertex_index}"
    visibility_volume_config["visibility_volume_vertex"] = convex_polygon_vertices[1]
    visibility_volume_config["output_obj_filename"] = \
        output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj"
    visibility_volume_obj_file_list.append(visibility_volume_config["output_obj_filename"])
    volume.configure(**visibility_volume_config)
    volume.evaluate()

    vertex_index = 3
    visibility_volume_config["visibility_volume_index"] = f"{vertex_index}"
    visibility_volume_config["visibility_volume_vertex"] = convex_polygon_vertices[2]
    visibility_volume_config["output_obj_filename"] = \
        output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj"
    visibility_volume_obj_file_list.append(visibility_volume_config["output_obj_filename"])
    volume.configure(**visibility_volume_config)
    volume.evaluate()

    # Paths to the input OBJ files and the output OBJ file
    data_path_root = '/home/arwillis/CLionProjects/visibility/data/'
    obj_file_1 = visibility_volume_obj_file_list[0]
    obj_file_2 = visibility_volume_obj_file_list[1]
    obj_file_3 = visibility_volume_obj_file_list[2]
    output_file = data_path_root + 'results/intersected_model.obj'

    # START OF BLENDER SCRIPTING CODE

    object_names = []
    # Set the view frustum end to 10000
    set_view_frustum_end(10000)

    # Ensure the default scene is empty
    bpy.ops.object.select_all(action='DESELECT')
    bpy.ops.object.select_by_type(type='MESH')
    bpy.ops.object.delete()

    # Load the first OBJ file
    # bpy.ops.import_scene.obj(filepath=obj_file_1)
    bpy.ops.wm.obj_import(filepath=obj_file_1)
    # Store the imported object
    object_1 = bpy.context.selected_objects[0]
    # Names of the objects to select and frame

    object_names.append(object_1.name)
    # bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP', iterations=1)
    # select_and_frame_objects(object_names)

    bpy.ops.object.select_all(action='DESELECT')
    bpy.ops.object.select_by_type(type='MESH')
    # Load the second OBJ file
    # bpy.ops.import_scene.obj(filepath=obj_file_2)
    bpy.ops.wm.obj_import(filepath=obj_file_2)
    # Store the imported object
    object_2 = bpy.context.selected_objects[0]

    object_names.append(object_2.name)
    # bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP', iterations=1)
    # select_and_frame_objects(object_names)

    bpy.ops.object.select_all(action='DESELECT')
    bpy.ops.object.select_by_type(type='MESH')
    # Load the second OBJ file
    # bpy.ops.import_scene.obj(filepath=obj_file_3)
    bpy.ops.wm.obj_import(filepath=obj_file_3)
    # Store the imported object
    object_3 = bpy.context.selected_objects[0]

    object_names.append(object_3.name)
    # bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP', iterations=1)
    # select_and_frame_objects(object_names)


    # Store the imported object
    mesh_1 = object_1.data
    V_1 = len(mesh_1.vertices)
    E_1 = len(mesh_1.edges)
    F_1 = len(mesh_1.polygons)  # faces
    print(f"Mesh 1 Genus = F - E + V - 2 = {F_1} - {E_1} + {V_1} - 2 = {F_1 - E_1 + V_1 - 2}")

    # Store the imported object
    mesh_2 = object_2.data
    V_2 = len(mesh_2.vertices)
    E_2 = len(mesh_2.edges)
    F_2 = len(mesh_2.polygons)  # faces
    print(f"Mesh 2 Genus = F - E + V - 2 = {F_2} - {E_2} + {V_2} - 2 = {F_2 - E_2 + V_2 - 2}")

    # Store the imported object
    mesh_3 = object_2.data
    V_3 = len(mesh_3.vertices)
    E_3 = len(mesh_3.edges)
    F_3 = len(mesh_3.polygons)  # faces
    print(f"Mesh 3 Genus = F - E + V - 2 = {F_3} - {E_3} + {V_3} - 2 = {F_3 - E_3 + V_3 - 2}")

    # Select and frame the objects
    select_and_frame_objects(object_names)

    # Ensure both objects are selected
    object_1.select_set(True)
    # object_2.select_set(True)
    # bpy.context.view_layer.objects.active = object_1

    # Join the objects into a single mesh
    # bpy.ops.object.join()

    # Switch to edit mode for the intersection operation
    bpy.ops.object.mode_set(mode='EDIT')
    # Select the entire mesh
    bpy.ops.mesh.select_all(action='SELECT')
    # Use the boolean modifier to intersect the meshes
    bpy.ops.object.mode_set(mode='OBJECT')
    bool_mod = object_1.modifiers.new(name='Boolean', type='BOOLEAN')
    bool_mod.operation = 'INTERSECT'
    bool_mod.use_self = True
    # Set the second object as the target for the boolean operation
    bool_mod.object = object_2
    # Apply the boolean modifier
    bpy.ops.object.modifier_apply(modifier='Boolean')
    # Remove the second object (optional)
    bpy.data.objects.remove(object_2)

    bool_mod = object_1.modifiers.new(name='Boolean', type='BOOLEAN')
    bool_mod.operation = 'INTERSECT'
    bool_mod.use_self = True
    bool_mod.object = object_3
    # Apply the boolean modifier
    bpy.ops.object.modifier_apply(modifier='Boolean')
    # Remove the third object (optional)
    bpy.data.objects.remove(object_3)

    # Export the resulting mesh to a new OBJ file
    # bpy.ops.export_scene.obj(filepath=output_file)
    bpy.ops.wm.obj_export(filepath=output_file)

    print(f'Intersection saved to {output_file}')
    bpy.ops.wm.quit_blender()
