# Using distutils is probably the easiest way to distribute a python C module.
# See https://docs.python.org/3/extending/building.html#building-c-and-c-extensions-with-distutils
# for more information about how this works, though the simplest example should
# be something like this:

from distutils.core import setup, Extension

liblitmus_dir = "../liblitmus/"

liblitmus_helper = Extension(
    "liblitmus_helper",
    sources=["liblitmus_wrapper.c"],
    include_dirs=[
        liblitmus_dir + "include",
        liblitmus_dir + "arch/x86/include"
    ],
    library_dirs=[liblitmus_dir],
    libraries=["litmus"]
)

description = "A python interface to liblitmus, for LITMUS^RT."
setup(name="Liblitmus Helper", version="1.0", description=description,
    ext_modules=[liblitmus_helper])

