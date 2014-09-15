# caeplugin-DualMesh
An experimental Pointwise CAE plugin that converts a 2D unstructured tri 
surface grid to its polygon dual mesh.

![DualMesh][Logo]


## Limitations

Currently, the polygon dual mesh is exported as a Pointwise Glyph script of 
the form shown below. The `vertex` and `poly` procs are not provided and 
must be implmented by the user. See `importDualMesh.glf` for an example 
implementation.

```Tcl
# Element centroid points 6
vertex Elem 0 {  2.9436843  5.0580130  0.0000000 }
vertex Elem 1 {  7.5634270  3.0293195  0.0000000 }
      ...snip...
vertex Elem 5 {  7.4581337  5.0316663  0.0000000 }
# boundary mid points 6
vertex Bndry 6 {  1.8695128  5.7636914  0.0000000 }
vertex Bndry 7 {  8.5653372  2.3663046  0.0000000 }
      ...snip...
vertex Hard 16 {  9.7102060  4.0963645  0.0000000 }
vertex Hard 17 {  7.3805919  7.0603647  0.0000000 }
poly Bndry { 6 0 3 9 12 }
poly Bndry { 10 4 0 6 13 }
      ...snip...
poly Bndry { 9 3 5 11 17 }
poly Interior { 0 4 2 1 5 3 }
```

## Viewing the Dual Mesh CAE Export in Pointwise

The distro's `glyph` folder contains two Glyph scripts, `exportDualMesh.glf` 
and `importDualMesh.glf` that can be used to visualize the dual mesh results 
in Pointwise.

### The `exportDualMesh.glf` Glyph Script

After loading a grid into Pointwise, run this script to export the grid to the 
file `DualMeshData.out` and then immediately import `DualMeshData.out` back 
into Pointwise using the `importDualMesh.glf` script.

If no domains are present when `exportDualMesh.glf` is run, the file `test1.pw` 
is loaded and processed.

### The `importDualMesh.glf` Glyph Script
 
This script loads the contents of a dual mesh export file named `DualMeshData.out`
and creates database entities representing the dual mesh vertices, polygons, and 
polygon centroid normals. These entities are placed in several layers that can be 
turned on or off as needed. The `DualMeshData.out` file must reside in the same 
folder as the script.


## Building the Plugin

This plugin was created using version 1.0 R7 of the Pointwise CAE Plugin SDK.
However, it should build properly with newer versions of the SDK.

To build the DualMesh plugin you must integrate this source code into your local PluginSDK 
installation by following these steps.

* Download and install the [Pointwise Plugin SDK][SDKdownload].
* Configure and validate the SDK following the [SDK's instructions][SDKdocs].
* Create a DualMesh plugin project using the mkplugin script: `mkplugin -uns -cpp DualMesh`
* Replace the project's generated files with the files from this repository.
* Follow the platform specific instuctions below.
* This plugin uses the Configurable Math Library. You can download it from the [CML website][CMLwebsite].
 * The cml must be installed in the same parent folder as PluginSDK. That is:
  * `/the/path/to/your/PluginSDK/`
  * `/the/path/to/your/cml/`

### Building the Plugin with Microsoft Visual Studio

* Open the Plugin SDK Visual Studio solution file.
 * For VS2008 open `PluginSDK\PluginSDK.sln`
 * For VS2012 open `PluginSDK\PluginSDK_vs2012.sln`
* Add the existing CaeUnsDualMesh project file to the solution *Plugins* folder.
 * For VS2008 add `PluginSDK\src\plugins\CaeUnsDualMesh\CaeUnsDualMesh.vcproj`
 * For VS2012 add `PluginSDK\src\plugins\CaeUnsDualMesh\CaeUnsDualMesh.vcxproj`
* Add the following source files to the *CaeUnsDualMesh* project
 * `FanSorter.cxx`
 * `FanSorter.h`
 * `PluginTypes.h`

### Building the Plugin with Mac OS/X and Linux

* Nothing more needs to be done.


## Disclaimer
Plugins are freely provided. They are not supported products of
Pointwise, Inc. Some plugins have been written and contributed by third
parties outside of Pointwise's control.

TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, POINTWISE DISCLAIMS
ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, WITH REGARD TO THESE SCRIPTS. TO THE MAXIMUM EXTENT PERMITTED
BY APPLICABLE LAW, IN NO EVENT SHALL POINTWISE BE LIABLE TO ANY PARTY
FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS
INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
INABILITY TO USE THESE SCRIPTS EVEN IF POINTWISE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES AND REGARDLESS OF THE FAULT OR NEGLIGENCE OF
POINTWISE.

[Logo]: https://raw.github.com/dbgarlisch/CaeUnsDualMesh/master/logo_dualmesh.png  "DualMesh Logo"
[SDKdocs]: http://www.pointwise.com/plugins
[SDKdownload]: http://www.pointwise.com/plugins/#sdk_downloads
[CMLwebsite]: http://cmldev.net/
