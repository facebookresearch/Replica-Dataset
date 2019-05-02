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

## Team 

**TODO**

## Citing the Replica Dataset

We describe the dataset in the following whitepaper that can be found on arxiv 

```
@article{replica2019arxiv,
  title =   {Replica},
  author =  {}'
  journal = {arXiv preprint arXiv:},
  year =    {2019}
}
```

## Contact

[Julian.Straub@oculus.com](Julian.Straub@oculus.com)

## Acknowledgements

**TODO**

