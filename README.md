# Objective:

- NEON EON is a physics puzzle game:
    - Drag shapes into their target zones to complete each level!
    - Four levels of increasing complexity.
    - If a shape falls off screen, the level resets.
    - Press `ENTER` to advance after completing a level.

# Controls:

- Hold `M1` and drag to grab and move shapes
- Press `ENTER` to advance on any prompt
- Press `` ` `` to toggle debug mode
- Press `Q` to quit

# Overview:

Originally, all I wanted to do for this project was to build a simple physics engine in C++ with raylib for rendering. However, I quickly realized that in the time I had, I could either make a simple physics engine or a decent game, but not both. I was granted permission to use Box2D, which allowed me to actually make this game work.

I was inspired by Lofty Tower and Nitrome's Powerup, two Flash games I played a ton of as a kid. I am also a fan of packing puzzles, which can be either games or toys where the objective is to fit objects into a confined space. Both have a similar mechanic of dragging shapes around a physics world to solve puzzles. When I was discussing textures with Marisa, she suggested glowing neon shapes, and that's how the theme was born; especially since I love retrowave aesthetics.

# Technical Details:

This project was the most technically ambitious thing I've built so far. Physics are handled by Box2D 3.x with a fixed timestep accumulator, and rendering is done in raylib-cpp with two custom GLSL shaders.

I challenged myself to follow modern C++ practices including RAII and OOP principles, and to write clean, modular code.
I am quite proud of not needing to write any `shutdown`-like functions since all resources are managed by destructors. The `raylib-cpp` wrapper library was a huge help in this regard, since it provides RAII wrappers for all raylib resources.

The scene system uses a `Scene` base class with fade-in/fade-out transitions managed by a `SceneManager`. Each level subclasses `Level`, which handles the physics world, mouse joint dragging, hit sounds, off-screen detection, and win/fail logic. Subclasses only need to implement `spawnEntities()`. The `ResourceManager` loads and stores all assets which persist across scenes, and provides convenient access to them. Assets required for only a single scene are loaded by that scene instead.

The target zone system uses `b2OverlapPolygon` sensor queries to check whether each entity's AABB is fully inside its slot. The title screen target is a T-shaped two-slot structure that stacks NEON on top of EON.

The two shaders are:

- **Invert shader** - inverts white/near-white pixels on the held entity while preserving colored glow, giving visual feedback during drag.
- **Spectrum shader** - rotates the hue of vivid pixels over time on level completion, while leaving white outlines and transparent pixels unchanged.

The title screen bakes the NEON and EON text into `RenderTexture`s at startup so they can be used as physics body textures. Bodies use a local AABB offset to align the visual texture with the collider.

I did my best to write helpful, professional comments without over-commenting (for practice and because I think I will come back to this project), but I did leave some easter eggs scattered throughout the codebase, just to keep myself entertained.

# Gen AI Statement:

Generative AI was used to assist with computing geometry and spawn positions for the levels, and as a learning tool to familiarize myself with Box2D's API and modern C++ practices. All code was written by me.

### Additional Notes:

- Credit to Marisa Triola for the art assets.
- Credit to Prof. Romero Cruz for the course.
