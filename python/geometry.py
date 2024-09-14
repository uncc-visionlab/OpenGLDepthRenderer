class Point2D:
    def __init__(self, x=0, y=0):
        """
        Initialize a Point2D object.

        Parameters:
        - x (float): The x-coordinate of the point.
        - y (float): The y-coordinate of the point.
        """
        self.x = x
        self.y = y

    def distance_to(self, other):
        """
        Calculate the Euclidean distance between this point and another Point2D.

        Parameters:
        - other (Point2D): The other point to calculate the distance to.

        Returns:
        - float: The distance between the two points.
        """
        if not isinstance(other, Point2D):
            raise TypeError("Argument must be an instance of Point2D.")
        return ((self.x - other.x) ** 2 + (self.y - other.y) ** 2) ** 0.5

    def __repr__(self):
        """
        Return a string representation of the point.
        """
        return f"Point2D(x={self.x}, y={self.y})"

class Point3D:
    def __init__(self, x=0, y=0, z=0):
        """
        Initialize a Point3D object.

        Parameters:
        - x (float): The x-coordinate of the point.
        - y (float): The y-coordinate of the point.
        - z (float): The z-coordinate of the point.
        """
        self.x = x
        self.y = y
        self.z = z

    def distance_to(self, other):
        """
        Calculate the Euclidean distance between this point and another Point3D.

        Parameters:
        - other (Point3D): The other point to calculate the distance to.

        Returns:
        - float: The distance between the two points.
        """
        if not isinstance(other, Point3D):
            raise TypeError("Argument must be an instance of Point3D.")
        return ((self.x - other.x) ** 2 + (self.y - other.y) ** 2 + (self.z - other.z) ** 2) ** 0.5

    def __repr__(self):
        """
        Return a string representation of the point.
        """
        return f"Point3D(x={self.x}, y={self.y}, z={self.z})"

class Point3D:
    def __init__(self, x=0, y=0, z=0):
        """
        Initialize a Point3D object.

        Parameters:
        - x (float): The x-coordinate of the point.
        - y (float): The y-coordinate of the point.
        - z (float): The z-coordinate of the point.
        """
        self.x = x
        self.y = y
        self.z = z

    def distance_to(self, other):
        """
        Calculate the Euclidean distance between this point and another Point3D.

        Parameters:
        - other (Point3D): The other point to calculate the distance to.

        Returns:
        - float: The distance between the two points.
        """
        if not isinstance(other, Point3D):
            raise TypeError("Argument must be an instance of Point3D.")
        return ((self.x - other.x) ** 2 + (self.y - other.y) ** 2 + (self.z - other.z) ** 2) ** 0.5

    def __repr__(self):
        """
        Return a string representation of the point.
        """
        return f"Point3D(x={self.x}, y={self.y}, z={self.z})"

