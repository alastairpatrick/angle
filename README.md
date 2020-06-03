# JANGLE

JANGLE is a Java library that translates OpenGL ES API calls to one of the hardware-supported APIs available
for that platform. It provides translation from OpenGL ES 2.0, 3.0 and 3.1 to Vulkan,
desktop OpenGL, OpenGL ES, Direct3D 9, and Direct3D 11.

JANGLE is the integration of two main components: [ANGLE](https://chromium.googlesource.com/angle/angle) (hence JANGLE's name) and [Android's Java EGL and OpenGL ES bindings](https://developer.android.com/guide/topics/graphics/opengl).

JANGLE supports 64-bit Windows, Mac OS X and Linux platforms.

JANGLE is intended to be used for offscreen rendering, i.e. to "pbuffer" surfaces, and GPGPU.

### Level of OpenGL ES support via backing renderers

|                |  Direct3D 9   |  Direct3D 11     |   Desktop GL   |    GL ES      |    Vulkan     |    Metal      |
|----------------|:-------------:|:----------------:|:--------------:|:-------------:|:-------------:|:-------------:|
| OpenGL ES 2.0  |    complete   |    complete      |    complete    |    complete   |    complete   |  in progress  |
| OpenGL ES 3.0  |               |    complete      |    complete    |    complete   |    complete   |               |
| OpenGL ES 3.1  |               |   in progress    |    complete    |    complete   |  in progress  |               |
| OpenGL ES 3.2  |               |                  |  in progress   |  in progress  |  in progress  |               |

### Platform support via backing renderers

|             |    Direct3D 9  |   Direct3D 11  |   Desktop GL  |    GL ES    |   Vulkan    |    Metal    |
|------------:|:--------------:|:--------------:|:-------------:|:-----------:|:-----------:|:-----------:|
| Windows     |    complete    |    complete    |   complete    |   complete  |   complete  |             |
| Linux       |                |                |   complete    |             |   complete  |             |
| Mac OS X    |                |                |   complete    |             |             | in progress |

### Surface type support

|             |    Pbuffer     |     Pixmap     |      Window      |
|------------:|:--------------:|:--------------:|:----------------:|
| Windows     | yes            | no             | AWT Component    |
| Linux       | yes            | no             | AWT Component    |
| Mac OS X    | yes            | no             | no               |

### Build instructions

Some clues. The BUILD.gn and other Chromium build infrastructure files are a red herring; those are inherited from the parent ANGLE project and preserved only to ease merging from upstream. The main build file for JANGLE is [CMakeLists.txt](https://github.com/alastairpatrick/jangle/blob/master/CMakeLists.txt).

The closest thing to build instructions at the moment is the [script](https://github.com/alastairpatrick/jangle/blob/master/.github/workflows/main.yml) that the continuous integration servers run.

More to come.
