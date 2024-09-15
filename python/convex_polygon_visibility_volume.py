# Setup python path when running through blender via "blender -P convex_polygon_visibility_volume.py"
import os
import sys

local_module_path = os.path.dirname(os.path.abspath(__file__))
if local_module_path not in sys.path:
    sys.path.append(local_module_path)

import bpy
from geometry import Point3D, Point2D
import time
from threading import Timer
from visibility_volume import VisibilityVolume


class BlenderAutomation:
    def __init__(self):
        # Initialize default configuration parameters
        self.parameters = {
            "blender_gui": False
        }
        print("BlenderAutomation initialized with default configuration.")

    def set_redraw(self, blender_gui):
        print("Enabling redraw events for Blender GUI.\n")
        self.parameters['blender_gui'] = blender_gui
        if bpy.context is not None:
            if hasattr(bpy.context, "screen"):
                if hasattr(bpy.context.screen, "areas"):
                    for area in bpy.context.screen.areas:
                        if area.type == 'VIEW_3D':
                            break
                    for region in area.regions:
                        if region.type == 'WINDOW':
                            break
                    self.area = area
                    self.region = region
                    # self.view_all_fn(area, region)
                    # return

    # Blender control functions
    def set_view_frustum_end(self, distance=10000):
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

    def select_and_frame_objects(self, object_names):
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
        self.recursive()
        # for area in bpy.context.screen.areas:
        #     if area.type == 'VIEW_3D':
        #         for region in area.regions:
        #             if region.type == 'WINDOW':
        #                 # override = {'area': area, 'region': region, 'edit_object': bpy.context.edit_object}
        #                 # bpy.ops.view3d.view_all(override)
        #                 # bpy.ops.view3d.view_selected(use_all_regions=False)
        #                 override = {'area': area, 'region': region}
        #                 # bpy.ops.view3d.view_all(override, center=False)
        #                 # bpy.ops.view3d.view_all(override, 'INVOKE_DEFAULT')
        #                 with bpy.context.temp_override(**override):
        #                     # bpy.ops.view3d.view_all(center=False)
        #                     bpy.ops.view3d.view_all(use_all_regions=False, center=False)
        # print(f"Selected objects {object_names} framed to the active view.")

    def recursive(self, count=0):
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
        if bpy.context is not None:
            if hasattr(bpy.context, "screen"):
                if hasattr(bpy.context.screen, "areas"):
                    for area in bpy.context.screen.areas:
                        if area.type == 'VIEW_3D':
                            break
                    for region in area.regions:
                        if region.type == 'WINDOW':
                            break
                    self.view_all_fn(area, region)
                    return

        if count < 20:
            count += 1
            Timer(0.1, self.recursive, (count,)).start()

    def view_all_fn(self, area, region):
        override = {'area': area, 'region': region}
        # bpy.ops.view3d.view_all(override, center=False)
        # bpy.ops.view3d.view_all(override, 'INVOKE_DEFAULT')
        with bpy.context.temp_override(**override):
            # bpy.ops.view3d.view_all(center=False)
            bpy.ops.view3d.view_all(use_all_regions=False, center=False)
        return

    def redraw(self, object_name_list, sleep_time=0.5):
        if self.parameters["blender_gui"]:
            self.select_and_frame_objects(object_name_list)
            # Update the view layer to apply changes
            # bpy.context.view_layer.update()
            override = {'area': self.area, 'region': self.region}
            with bpy.context.temp_override(**override):
                # bpy.ops.view3d.view_all(center=False)
                bpy.ops.view3d.view_selected(use_all_regions=False)
                # bpy.ops.view3d.view_all(use_all_regions=False, center=False)
                bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP', iterations=1)
                # Pause for visibility
                time.sleep(sleep_time)

    def clear_scene(self):
        """Clears all objects in the current Blender scene."""
        bpy.ops.object.select_all(action='DESELECT')
        bpy.ops.object.select_by_type(type='MESH')
        bpy.ops.object.delete()

    def get_selected_objects(self):
        return bpy.context.selected_objects

    def remove(self, obj):
        # Remove the second object (optional)
        bpy.data.objects.remove(obj)

    def quit(self):
        bpy.ops.wm.quit_blender()

    def import_obj(self, filepath):
        """Imports an OBJ file into Blender."""
        # bpy.ops.import_scene.obj(filepath=filepath)
        bpy.ops.wm.obj_import(filepath=filepath)
        return bpy.context.selected_objects[0]

    # bpy.context.view_layer.objects.active = object_1
    # Join the objects into a single mesh
    # bpy.ops.object.join()

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
        # bpy.ops.export_scene.obj(filepath=filepath)
        bpy.ops.wm.obj_export(filepath=filepath)
        print(f'Intersection saved to {filepath}')

    def print_mesh_statistics(self, obj):
        mesh = obj.data
        V = len(mesh.vertices)
        E = len(mesh.edges)
        F = len(mesh.polygons)  # faces
        print(f"Mesh Genus = F - E + V - 2 = {F} - {E} + {V} - 2 = {F - E + V - 2}")

    def mesh_genus(self, obj):
        mesh = obj.data
        V = len(mesh.vertices)
        E = len(mesh.edges)
        F = len(mesh.polygons)  # faces
        topological_genus = F - E + V - 2
        return topological_genus

    def intersect_objs_and_export(self, obj_file_1, obj_file_2, output_file):
        """Main function to handle the intersection and export."""
        self.clear_scene()
        object_1 = self.import_obj(obj_file_1)
        object_2 = self.import_obj(obj_file_2)

        self.perform_intersection(object_1, object_2)

        # Optionally remove the second object (it has been intersected)
        bpy.data.objects.remove(object_2)

        self.export_obj(output_file)
        print(f'Intersection saved to {output_file}')


class ConvexPolygonVisibilityVolume:
    def __init__(self, vertices):
        # Initialize default configuration parameters
        self.parameters = {
            "blender_gui": False,
            "python_folder_path": "",
            "segment_visibility_volume_output_path": "",
            "segment_visibility_volume_output_file": 'segment_visibility_volume.obj',
            "polygon_visibility_volume_output_file": 'polygon_visibility_volume.obj'
        }
        self.vertices = vertices
        self.num_vertices = len(self.vertices)
        self.equivalent_viewpoints = self.num_vertices * [False]
        self.blender = BlenderAutomation()
        self.blender.clear_scene()
        self.VIEW_FRUSTUM_LENGTH = 10000
        self.visibility_volume = VisibilityVolume()
        print("ConvexPolygonVisibilityVolume initialized with default configuration.")

    def configure(self, debug=False, **kwargs):
        """
        Configure the visibility volume with custom parameters.
        Accepts parameters like shape, dimensions, center, and visibility_threshold.

        Parameters:
            **kwargs: Key-value pairs of parameters to configure the volume.
        """
        # Update the configuration parameters with provided values
        for key, value in kwargs.items():
            if key in self.parameters:
                self.parameters[key] = value
                if debug:
                    print(f"Parameter '{key}' set to {value}.")
            else:
                print(f"Warning: Unknown parameter '{key}' ignored.")
            if key == "blender_gui":
                self.blender.set_redraw(value)
                self.blender.set_view_frustum_end(self.VIEW_FRUSTUM_LENGTH)

    def configure_visibility_volume(self, visibility_volume_config):
        self.visibility_volume.configure(**visibility_volume_config)

    def evaluate(self):
        for segment_index in range(0, self.num_vertices):
            segment_has_equivalent_viewpoint_ends = self.compute_segment_visibility(segment_index)
            self.equivalent_viewpoints[segment_index] = segment_has_equivalent_viewpoint_ends
            if segment_has_equivalent_viewpoint_ends == False:
                print("")

        visibility_volume_obj_file_list = []
        # Compute visibility volumes for all of the view equivalent vertices of the polygon
        for vertex_index, vertex in enumerate(self.vertices):
            visibility_volume_config_update = {
                "visibility_volume_index": f"{vertex_index}",
                "visibility_volume_vertex": vertex,
                "output_obj_filename": output_data_path + f"destin_triangle_{vertex_index + 1}_visibility_v{vertex_index + 1}.obj"
            }
            convex_polygon_visibility_volume.configure_visibility_volume(visibility_volume_config_update)
            visibility_volume_obj_file_list.append(visibility_volume_config_update["output_obj_filename"])
            self.visibility_volume.evaluate()

        root_visibility_volume = self.blender.import_obj(visibility_volume_obj_file_list[0])
        del visibility_volume_obj_file_list[0]

        for view_equivalent_vv_file in visibility_volume_obj_file_list:
            visibility_volume = self.blender.import_obj(view_equivalent_vv_file)
            self.blender.redraw([root_visibility_volume.name, visibility_volume.name], 5)
            self.blender.perform_intersection(root_visibility_volume, visibility_volume)
            self.blender.remove(visibility_volume)

        self.blender.redraw([root_visibility_volume.name], 5)
        self.blender.export_obj(self.parameters['polygon_visibility_volume_output_file'])

        # if self.parameters["blender_gui"]:
        #     self.blender.quit()

    def compute_segment_visibility(self, segment_index):
        visibility_volume_obj_file_list = []
        vertexA_index = segment_index % self.num_vertices
        vertexB_index = (segment_index + 1) % self.num_vertices  # wrap to vertex index 0 index for last segment

        # Compute vertex A visibility volume
        visibility_volume_config_update = {
            "visibility_volume_index": f"{vertexA_index}",
            "visibility_volume_vertex": convex_polygon_vertices[vertexA_index],
            "output_obj_filename": output_data_path + f"destin_triangle_{segment_index + 1}_visibility_v{vertexA_index + 1}.obj"
        }
        convex_polygon_visibility_volume.configure_visibility_volume(visibility_volume_config_update)
        visibility_volume_obj_file_list.append(visibility_volume_config_update["output_obj_filename"])
        self.visibility_volume.evaluate()

        # Compute vertex B visibility volume
        visibility_volume_config_update = {
            "visibility_volume_index": f"{vertexB_index}",
            "visibility_volume_vertex": convex_polygon_vertices[vertexB_index],
            "output_obj_filename": output_data_path + f"destin_triangle_{segment_index + 1}_visibility_v{vertexB_index + 1}.obj"
        }
        convex_polygon_visibility_volume.configure_visibility_volume(visibility_volume_config_update)
        visibility_volume_obj_file_list.append(visibility_volume_config_update["output_obj_filename"])
        self.visibility_volume.evaluate()

        visibility_volume_A = self.blender.import_obj(visibility_volume_obj_file_list[0])
        self.blender.redraw([visibility_volume_A.name], 5)
        visibility_volume_B = self.blender.import_obj(visibility_volume_obj_file_list[1])
        self.blender.redraw([visibility_volume_A.name, visibility_volume_B.name], 5)

        for visibility_volume in [visibility_volume_A, visibility_volume_B]:
            self.blender.print_mesh_statistics(visibility_volume)

        self.blender.perform_intersection(visibility_volume_A, visibility_volume_B)
        self.blender.print_mesh_statistics(visibility_volume_A)

        if self.blender.mesh_genus(visibility_volume_A) == 0:
            print(f"Segment {segment_index + 1} has equivalent viewpoint endpoints.")
            segment_has_equivalent_viewpoint_ends = True
        else:
            print(
                f"Segment {segment_index + 1} does not have equivalent viewpoint endpoints, recursively split this segment.")
            segment_has_equivalent_viewpoint_ends = False

        self.blender.remove(visibility_volume_B)
        self.blender.redraw([visibility_volume_A.name], 5)

        segment_vv_output_path = self.parameters['segment_visibility_volume_output_path']
        polygon_visibility_volume_config_update = {
            "segment_visibility_volume_output_file": segment_vv_output_path + f"segment{segment_index + 1}_visibility_volume.obj",
        }
        self.configure(**polygon_visibility_volume_config_update)
        self.blender.export_obj(self.parameters['segment_visibility_volume_output_file'])

        self.blender.remove(visibility_volume_A)
        self.blender.clear_scene()
        return segment_has_equivalent_viewpoint_ends


# Example usage of the ConvexPolygonVisibilityVolume class
if __name__ == "__main__":
    # Check if an argument is passed
    if "blender" in sys.argv[0]:
        # Read the first argument
        first_argument = sys.argv[0]
        print(f"First command line argument: {first_argument}. Running in blender GUI.")
        blender_gui = True
    else:
        print("No arguments were provided. Not running in blender GUI")
        blender_gui = False

    data_path_prefix = "/home/arwillis/CLionProjects/visibility/data/"
    prog_path_prefix = "/home/arwillis/CLionProjects/visibility/"
    input_data_path = data_path_prefix + "inputs/"
    output_data_path = data_path_prefix + "results/"
    visibility_prog_path_root = prog_path_prefix + "OpenGLDepthRenderer/cmake-build-debug/bin/"
    python_prog_path = prog_path_prefix + "OpenGLDepthRenderer/python/"
    visibility_prog_path_and_filename = visibility_prog_path_root + "ogl_depthrenderer"
    environment_data_path_and_filename = input_data_path + "destin_and_miramar_beach.obj"
    output_obj_path_and_filename = output_data_path + f"default.obj"

    # Setup configuration parameters for polygon visibility volume calculations with custom parameters
    polygon_visibility_volume_config = {
        "blender_gui": blender_gui,
        "python_folder_path": python_prog_path,
        "segment_visibility_volume_output_path": output_data_path,
        "segment_visibility_volume_output_file": output_data_path + 'segment_visibility_volume.obj',
        "polygon_visibility_volume_output_file": output_data_path + 'polygon_visibility_volume.obj'
    }

    # Setup configuration parameters for visibility volume calculations with custom parameters
    visibility_volume_config = {
        "blender_gui": blender_gui,
        "world_obj_filename": environment_data_path_and_filename,
        "output_obj_filename": output_obj_path_and_filename,
        "visibility_volume_index": "-1",
        "visibility_volume_vertex": Point3D(10, 10, 10),  # Default dimensions (length, width, height)
        "visibility_volume_mesh_resolution": Point2D(40, 40),
        "center": Point3D(0, 0, 0),  # Center of the volume
        "visibility_height_threshold": 0.5,  # Default threshold for visibility
        "visibility_prog_path_root": visibility_prog_path_root,
        "visibility_prog_path_and_filename": visibility_prog_path_and_filename,
        "max_radius": 100
    }

    vertex1 = Point3D(-110.9651107788086, 1.0, 1527.08544921875)
    vertex2 = Point3D(-80.03492287079642, 1.0, 1526.6367639369446)
    vertex3 = Point3D(-88.3062336621307, 1.0, 1499.6531723345058)
    convex_polygon_vertices = [vertex1, vertex2, vertex3]

    # Create an instance of the ConvexPolygonVisibilityVolume class
    convex_polygon_visibility_volume = ConvexPolygonVisibilityVolume(convex_polygon_vertices)
    convex_polygon_visibility_volume.configure(**polygon_visibility_volume_config)
    convex_polygon_visibility_volume.configure_visibility_volume(visibility_volume_config)

    convex_polygon_visibility_volume.evaluate()
