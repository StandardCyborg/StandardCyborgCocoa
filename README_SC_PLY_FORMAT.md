# PLY format

Our current header for point clouds stored in PLY files looks like:

```
ply
format ascii 1.0
comment StandardCyborgFusionVersion 1.2.0
comment StandardCyborgFusionMetadata { "color_space": "sRGB" }
element vertex 30135
property float x
property float y
property float z
property float nx
property float ny
property float nz
property uchar red
property uchar green
property uchar blue
property float surfel_radius
element face 0
property list uchar int vertex_indices
element gravity 1
property float gravity_x
property float gravity_y
property float gravity_z
end_header
```

Note that MeshLab, for one, (perhaps unnecessarily?) limits the length of comments so that StandardCyborgFusionMetadata should be tested carefully to ensure files with this field parse correctly.
