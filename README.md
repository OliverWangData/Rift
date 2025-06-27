# Rift

![NoiseGraph Demo](Media/NoiseGraphShowcase.gif)

Rift is an *incomplete* technical prototype showcasing a high-performance voxel world built with SIMD-accelerated, determinate fixed-point procedural noise. The world is generated using Surface Nets for smooth mesh extraction and organized into voxelized chunks, designed for deterministic simulation and potential multiplayer use.

To maximize memory bandwidth and runtime performance, voxel data is bitpacked and efficiently decoded in a custom Unreal Engine vertex factory, enabling highly optimized rendering pipelines.

Key features include:
- SIMD-optimized fixed-point noise functions
- Bitpacked voxel data with custom decoding in the vertex factory
- Surface Nets mesh generation for smooth voxel terrain
- Chunked voxel world structure for efficient updates
- Deterministic algorithms suitable for multiplayer synchronization

This project demonstrates advanced C++ and algorithmic expertise in real-time procedural generation, voxel management, and rendering optimization.


Long demo showcase of the workflow for the noise graph:
https://www.youtube.com/watch?v=kAbr6sMtY1I
