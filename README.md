# GoCoPP

This is an implementation of the paper [Finding Good Configurations of Planar Primitives in Unorganized Point Clouds](https://hal.inria.fr/hal-03621896) by [Mulin Yu](http://www-sop.inria.fr/members/Mulin.Yu/) and [Florent Lafarge](http://www-sop.inria.fr/members/Florent.Lafarge/).

This repository is a fork of [Ylannl/GoCoPP](https://github.com/Ylannl/GoCoPP) aiming at improving reproducibility using pixi.

## Citation

> Mulin Yu, Florent Lafarge. Finding Good Configurations of Planar Primitives in Unorganized Point Clouds. CVPR 2022 - IEEE Conference on Computer Vision and Pattern Recognition, Jun 2022, La Nouvelle-Orléans, United States. ⟨hal-03621896⟩

```bibtex
@inproceedings{yu:hal-03621896,
  TITLE = {{Finding Good Configurations of Planar Primitives in Unorganized Point Clouds}},
  AUTHOR = {Yu, Mulin and Lafarge, Florent},
  URL = {https://inria.hal.science/hal-03621896},
  BOOKTITLE = {{CVPR 2022 - IEEE Conference on Computer Vision and Pattern Recognition}},
  ADDRESS = {La Nouvelle-Orl{\'e}ans, United States},
  YEAR = {2022},
  MONTH = Jun,
  PDF = {https://inria.hal.science/hal-03621896v1/file/CVPR22_Mulin.pdf},
  HAL_ID = {hal-03621896},
  HAL_VERSION = {v1},
}
```

## Input

The input data is a [PLY](https://en.wikipedia.org/wiki/PLY_(file_format)) file. It should contain the coordinates of input points (x y z). The normals of input points (nx ny nz) are also encouraged to be included. If not included, they will be estimated by [PCA](https://doc.cgal.org/5.6.3/Point_set_processing_3/index.html) just after loading the file.

## Installation and usage

To install GoCoPP, the first step is to clone the repository.
Then, you can use [`pixi`](https://pixi.prefix.dev/latest/installation/) to install the dependencies and build the project:

```bash
# Move to the project directory
cd GoCoPP
# Build (not necessary since `pixi run run` will build the project if needed)
pixi run build
# Run
pixi run run <input_file> --output <output_file> <..options>
```

## Parameters

- The parameter `--output` is the output file path. If not specified, it will be created automatically based on the input name and the parameters, and located in the running folder.
- The parameter `--epsilon` is the fitting tolerance that specifies the maximal distance of an inlier to its supporting plane (default `0.4% * bounding box diagonal`).
- The parameter `--sigma` is the minimal primitive size that allows primitives with a too low number of inliers to be discarded (default `50`).
- The parameter `--nn` is the k of the k-nearest neighbour graph and also used to estimate the points' normals if they are not provided (default `20`).
- The parameter `--normal_deviation` is (i) the minimum cosine of the angle between an inlier and its supporting plane, which is used for region growing, and (ii) the minimum cosine of the angle between two primitives that can be merged (default `0.85`).
- The parameter `--s` and `--c` are the weight for simplicity and completeness, note that `0 < s + c < 3` (default `1` and `1`).
- The parameter `--norm` decide the fidelity metric, which can be `normal`, `L2` or `hybrid` (default `hybrid`). `L2` represents the Euclidean distance between the inlier points and the associated supporting planes, `normal` presents the deviation between inliers' normals and the associated supporting planes' normals and `hybrid` mode uses `L2` in the priority queue and `normal` in the transfer operator.
- The parameter `--max_steps` is the maximum iterations of our exploration mechanism (default `7`).
- The parameter `--constraint` is an option to make sure that the final configuration does not degrade the fidelity, simplicity and completeness of the initial configuration (default `False`).
- The parameter `--vg` is an option to output the results in 'vg' form([Vertex Group](https://github.com/LiangliangNan/PolyFit/blob/main/ReadMe.md#data)) (default `False`).
- The parameter `--alpha` is an option to output the primitives that are represented as alpha shapes (default `True`).
- The parameter `--hull` is an option to output the primitives that are represented as convex hulls (default `False`).

