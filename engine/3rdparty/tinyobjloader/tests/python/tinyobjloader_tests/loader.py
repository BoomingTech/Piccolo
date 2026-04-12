from tinyobjloader import ObjReader, ObjReaderConfig


class LoadException(Exception):
    pass


class Loader:
    """
    A light wrapper around ObjReader to provide a convenient interface for testing.
    """

    def __init__(self, triangulate):
        self.reader = ObjReader()
        config = ObjReaderConfig()
        config.triangulate = triangulate
        self.config = config

    def load(self, mesh_path):
        if not self.reader.ParseFromFile(mesh_path, self.config):
            raise LoadException(self.reader.Error() or self.reader.Warning())

    def loads(self, mesh_string):
        if not self.reader.ParseFromString(mesh_string, "", self.config):
            raise LoadException(self.reader.Error() or self.reader.Warning())

    @property
    def shapes(self):
        return self.reader.GetShapes()

    @property
    def attrib(self):
        return self.reader.GetAttrib()
