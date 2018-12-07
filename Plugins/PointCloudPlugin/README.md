Unreal Engine 4 - Point Cloud Plugin

Current Version: 0.4.2 beta

This plugin has been created to help with importing, processing and rendering of point clouds.

Tested Engine Versions: 4.18.3, 4.19.2
Tested Platform: Windows

For current and planned feature lists visit the Roadmap (https://trello.com/b/rQ1SCygB/pointcloudplugin-roadmap)

HOW TO USE:
1. Drag and drop TXT or XYZ to content browser (importer should work with any column-based text file)
2. Select columns to import
3. Drag and drop the imported asset onto the scene

IMPORTANT:
- This is still a beta version, be prepared that some things may go wrong.
- In order to optimize VRAM footprint, the plugin introduces a new Vertex Factory (compiling with half precision support introduces an extra one), therefore during initial Engine start-up the shadres will have to be compiled, this might take a while.
- Rendering clouds as sprites uses ~4.5x more VRAM than points, but due to their adjustable size, less overall density is required

KNOWN ISSUES:
- May not fully support using Sequencer in 4.18