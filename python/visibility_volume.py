from geometry import Point3D, Point2D
import os
import subprocess
import tempfile

class VisibilityVolume:
    def __init__(self):
        # Initialize default configuration parameters
        self.parameters = {
            "world_obj_filename": "",
            "output_obj_filename": "",
            "visibility_volume_index": "1",
            "visibility_volume_vertex": Point3D(10, 10, 10),  # Default dimensions (length, width, height)
            "visibility_volume_mesh_resolution": Point2D(40, 40),
            "center": (0, 0, 0),  # Center of the volume
            "visibility_height_threshold": 0.5,  # Default threshold for visibility
            "visibility_prog_path_root": "",
            "visibility_prog_path_and_filename": "ogl_depthrenderer",
            "max_radius": -1
        }
        print("VisibilityVolume initialized with default configuration.")

    def configure(self, **kwargs):
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
                print(f"Parameter '{key}' set to {value}.")
            else:
                print(f"Warning: Unknown parameter '{key}' ignored.")

    def execute(self, config_file_name_and_path):
        # Example: Run the 'ls' command (Linux/Mac) or 'dir' command (Windows)
        # You can replace 'ls' with any other program or script you want to invoke
        os.chdir(self.parameters['visibility_prog_path_root'])
        try:
            argument_list = [self.parameters['visibility_prog_path_and_filename']]
            # if False and self.parameters['max_radius'] and self.parameters['max_radius'] > 0:
            #     argument_list.extend(['-r', self.parameters['max_radius']])
            argument_list.extend(['-c', config_file_name_and_path])
                                  # '-o', self.parameters['output_obj_filename']])
            print("Command: " + ' '.join(map(str, argument_list)) + "\n")
            result = subprocess.run(argument_list, capture_output=True, text=True, check=True)
            print(f"Output:\n{result.stdout}")
        except subprocess.CalledProcessError as e:
            print(f"An error occurred: {e}")

    def evaluate(self):
        """
        Compute the visibility volume for a given point.

        Parameters:
            point (tuple): A tuple representing the (x, y, z) coordinates of the point to evaluate.
        """
        # Create a temporary directory
        with tempfile.TemporaryDirectory() as temp_dir:
            # Print the path of the temporary directory
            print(f"Temporary directory created at: {temp_dir}")

            # Define the path of the new folder inside the temporary directory
            folder_path = os.path.join(temp_dir, "visibility_volume")

            # Create the new folder
            os.mkdir(folder_path)
            print(f"New folder created at: {folder_path}")

            # Define the path of the new text file inside the new folder
            config_file_path_and_name = os.path.join(folder_path, "visibility_config.yaml")
            self.write_config_file(config_file_path_and_name)
            self.execute(config_file_path_and_name)

        # The temporary directory and its contents are deleted once the block ends

    def write_config_file(self, file_path):
        params = self.parameters
        visibility_yaml_config = f"""
world_coord_sys:
    id: world
    origin: [0.0, 0.0, 0.0]
    up: [0.0, 1.0, 0.0]
    front: [1.0, 0.0, 0.0]

mesh:
    id: Mesh 1
    position: [0.0, 0.0, 0.0]
    scale: 1.0
    orientation axis, angle: [1, 0, 0, 0]
    format: OBJ
    filename: {params["world_obj_filename"]}

visibility_vol:
    id: Volume {params["visibility_volume_index"]}
    width: {params["visibility_volume_mesh_resolution"].x}
    height: {params["visibility_volume_mesh_resolution"].y}
    fov_degrees: 90
    origin: [{params["visibility_volume_vertex"].x}, {params["visibility_volume_vertex"].y}, {params["visibility_volume_vertex"].z}]
    front: [1.0, 0.0, 0.0]
    up: [0.0, 1.0, 0.0]
    # max height of visibility volume above ground plane [meters]
    up_max: 4000.0
    # min height of visibility volume above ground plane [meters]
    up_min: -4000.0 
    # max radius of visibility volume [meters] 
    radius_max: {params["max_radius"]} 
    output_file: {params["output_obj_filename"]}
"""

        # Write some text to the new file
        with open(file_path, "w") as file:
            file.write(visibility_yaml_config)

        # Confirm file writing
        print(f"File written to: {file_path}")

        # Optionally, read and display the content of the file
        with open(file_path, "r") as file:
            content = file.read()
        print("File content:\n", content)


# Example usage of the VisibilityVolume class
if __name__ == "__main__":
    # Create an instance of the VisibilityVolume class
    volume = VisibilityVolume()
    data_path_prefix = "/home/arwillis/CLionProjects/visibility/data/"
    prog_path_prefix = "/home/arwillis/CLionProjects/visibility/"
    input_data_path = data_path_prefix + "inputs/"
    output_data_path = data_path_prefix + "results/"
    visibility_prog_path_root = prog_path_prefix + "OpenGLDepthRenderer/cmake-build-debug/bin/"
    visibility_prog_path_and_filename = visibility_prog_path_root + "ogl_depthrenderer"
    vertex1 = Point3D(-110.9651107788086, 1.0, 1527.08544921875)
    vertex2 = Point3D(-80.03492287079642, 1.0, 1526.6367639369446)
    vertex3 = Point3D(-88.3062336621307, 1.0, 1499.6531723345058)
    vertices = [vertex1, vertex2, vertex3]
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
        "max_radius": 50
    }
    # volume.configure(shape="sphere", dimensions=(20, 20, 20), center=(5, 5, 5))
    vertex_index = 1
    visibility_volume_config["visibility_volume_index"] = f"{vertex_index}"
    visibility_volume_config["visibility_volume_vertex"] = vertices[0]
    visibility_volume_config["output_obj_filename"] = \
         output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj"
    volume.configure(**visibility_volume_config)
    volume.evaluate()

    vertex_index = 2
    visibility_volume_config["visibility_volume_index"] = f"{vertex_index}"
    visibility_volume_config["visibility_volume_vertex"] = vertices[1]
    visibility_volume_config["output_obj_filename"] = \
        output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj"
    volume.configure(**visibility_volume_config)
    volume.evaluate()

    vertex_index = 3
    visibility_volume_config["visibility_volume_index"] = f"{vertex_index}"
    visibility_volume_config["visibility_volume_vertex"] = vertices[2]
    visibility_volume_config["output_obj_filename"] = \
        output_data_path + f"destin_triangle_{segment_index}_visibility_v{vertex_index}.obj"
    volume.configure(**visibility_volume_config)
    volume.evaluate()

