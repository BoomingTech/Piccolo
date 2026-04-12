# Adapted from https://github.com/pybind/python_example/blob/master/setup.py
import sys

# from pybind11 import get_cmake_dir
# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension  # , build_ext
from setuptools import setup

try:
    # try to read setuptools_scm generated _version.py
    from .python import _version
except:
    __version__ = "2.0.0rc10"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
        "tinyobjloader",
        sorted(["python/bindings.cc", "python/tiny_obj_loader.cc"]),
        # Example: passing in the version to the compiled code
        define_macros=[("VERSION_INFO", __version__)],
        cxx_std=11,
    ),
]

setup(
    name="tinyobjloader",
    packages=["python"],
    # version=__version__,
    author="Syoyo Fujita",
    author_email="syoyo@lighttransport.com",
    url="https://github.com/tinyobjloader/tinyobjloader",
    # project_urls={
    #    "Issue Tracker": "https://github.com/tinyobjloader/tinyobjloader/issues",
    # },
    description="Tiny but powerful Wavefront OBJ loader",
    long_description_content_type="text/markdown",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Manufacturing",
        "Topic :: Artistic Software",
        "Topic :: Multimedia :: Graphics :: 3D Modeling",
        "Topic :: Scientific/Engineering :: Visualization",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
    ],
    ext_modules=ext_modules,
    # extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    # cmdclass={"build_ext": build_ext},
    # zip_safe=False,
    # python_requires=">=3.6",
)
