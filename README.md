# Replica Dataset

The Replica Dataset is a dataset of five high quality reconstructions of a
variety of indoor spaces. Each reconstruction has clean dense geometry, high
resolution and high dynamic range textures, glass and mirror surface
information, planar segmentation as well as
semantic class and instance segmentation.

**TODO** add video of flythroughs here

The Replica SDK contained in this repository allows visual inspection of the
datasets via the ReplicaViewer and gives and example of how to render out images
from the scenes headlessly via the ReplicaRenderer. 

For machine learning purposes each dataset also contains an export to the format
employed by [AI Habitat](https://www.aihabitat.org/) and is therefore usable
seamlessly in that framework for AI agent training and other ML tasks.

## Dataset Layout

Each Replica contains the following assets:
```
├── glass.sur
├── habitat
    ├── mesh_semantic.ply 
    ├── mesh_semantic.navmesh
    ├── info_semantic.json
    ├── mesh_preseg_semantic.ply 
    ├── mesh_preseg_semantic.navmesh
    └── info_preseg_semantic.json 
├── mesh.ply
├── preseg.bin
├── preseg.json
├── semantic.bin
├── semantic.json
└── textures
    ├── 0-color-ptex.hdr
    ├── 0-color-ptex.w
    ├── 1-color-ptex.hdr
    ├── 1-color-ptex.w
    ├── ...
    └── parameters.json
```
The different files contain the following:
- `glass.sur`: parameterization of glass and mirror surfaces.
- `mesh.ply`: the quad mesh of the scene with vertex colors.
- `preseg.json` and `preseg.bin`: the presegmentation in terms of planes and non-planes of the scene.
- `semantic.json` and `semantic.bin`: the semantic segmentation of the scene.
- `textures`: the high resolution and high dynamic range textures of the scene.
- `habitat/mesh*semantic.ply`: the quad meshes including semantic or presegmentation information for AI Habitat. 
- `habitat/info*semantic.json`: mapping from instance IDs in the respective `mesh_*.ply` to semantic names.
- `habitat/mesh*semantic.navmesh`: navigation grid for AI Habitat.

## Replica SDK

### Setup
The Replica SDK can be compiled using the build script via
```
git submodule init
./build.sh
```
It requires the development packages for libGLEW, libGL **TODO** to be
installed first.

### ReplicaViewer

ReplicaViewer is a interactive UI to explore the Replica Dataset. 

```
./build/bin/ReplicaViewer mesh.ply /path/to/atlases [mirrorFile]
```

![ReplicaViewer](./assets/ReplicaViewer.png)

The exposure value for rendering from the HDR textures can be adjusted on the
top left. 

### ReplicaRenderer

The ReplicaRenderer shows how to render out images from a Replica for a
programmatically defined trajectory without UI. This executable can be run
headless on a server if so desired. 

```
./build/bin/ReplicaRenderer mesh.ply /path/to/atlases [mirrorFile]
```

## Replica and AI Habitat

To use Replica within AI Habitat checkout the AI Habitat Sim at https://github.com/facebookresearch/habitat-sim
After building the project you can launch the test viewer to verify that everything works:
```
./build/viewer /PATH/TO/REPLICA/apartment_0/habitat/mesh_semantic.ply
```

## Team 

Julian Straub, Thomas Wheelan, Lingni Ma, Steven Chen, Simon Green, Erik Wijman, Brian Budge, Christopher Dotson, Shobhit Verma, Anton Clarkson, Luis Pesqueira, Matthew Banks, Yuyang Zou, Mingfei Yan, Richard Newcombe.

## Contact

[Julian.Straub@oculus.com](Julian.Straub@oculus.com)

## Acknowledgements

We want to acknowledge the hard work of our operators in the field who scanned
apartments and places. 
We also want to acknowledge our collaborators Dhruv Batra and Manolis Savva at FAIR who worked to allow the use
of Replica with AI Habitat. 
Additionally beyond the immediate team, this dataset would not have been
possible without various contributions from the surreal team at large. 
