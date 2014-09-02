# caeplugin-DualMesh
An experimental Pointwise CAE plugin that converts a 2D unstructured tri 
surface grid to its polygon dual mesh.

![DualMesh][Logo]

## Limitations
Currently, the polygon dual mesh is exported as a glyph script. To visualize 
the results, the exported script can be loaded into Pointwise as database 
entities using `load-test1.glf`.

The conversion logic does not properly handle sharp corners (yet). This results 
in a loss of precision.

## Building the Plugin
To build the DualMesh plugin you must integrate this source code into your local PluginSDK 
installation by following these steps.

* Download and install the [Pointwise Plugin SDK][SDKdownload].
* Configure and validate the SDK following the [SDK's instructions][SDKdocs].
* Create a DualMesh plugin project using the mkplugin script: `mkplugin -uns -cpp DualMesh`
* Replace the project's generated files with the files from this repository.

This plugin was created using version 1.0 R7 of the Pointwise CAE Plugin SDK.


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

[Logo]: https://raw.github.com/dbgarlisch/CaeUnsGMesh/master/logo_dualmesh.png  "DualMesh Logo"
[SDKdocs]: http://www.pointwise.com/plugins
[SDKdownload]: http://www.pointwise.com/plugins/#sdk_downloads
