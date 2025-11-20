# Demoscene Killer Project

This project contains a progression of fire simulations, culminating in a WebGL demoscene demo.

## Final Result
The final output is **[demoscene-killer-v2.html](https://unamatasanatarai.github.io/antigravity-demoscene-killer/demoscene-killer-v2.html)**. Open it in any modern browser to see the result.

## Reproduction Prompt
To generate the final result from scratch using an LLM, use the following prompt:

> Write a single-file HTML5/WebGL demoscene application called "Demoscene Killer".
> 
> **Requirements:**
> 1.  **Tech Stack**: Pure WebGL and JavaScript contained in a single HTML file. No external libraries (like Three.js or glMatrix).
> 2.  **Scene**:
>     *   **Main Cube**: A large 3D rotating cube in the center.
>     *   **Orbiting Cubes**: 4 smaller cubes orbiting around the main cube.
>     *   **Background**: A full-screen 3D tunnel effect running towards the camera.
> 3.  **Procedural Textures** (Generated and updated every frame in JS):
>     *   **Fire**: Classic Doom-style fire algorithm (bottom-up propagation, cooling, randomness). Apply this to **3 faces** of the main cube.
>     *   **Plasma**: Old-school sine-wave color cycling effect. Apply this to **2 faces** of the main cube.
>     *   **Smoke**: A variation of the fire algorithm using a grey palette and slower decay. Apply this to **1 face** of the main cube.
>     *   **Tunnel**: A procedural pattern (e.g., XOR texture or radial distance lookup) that simulates a tunnel. Apply this to the **4 orbiting cubes** and the **background**.
> 4.  **Animation**:
>     *   The main cube rotates on all axes.
>     *   The orbiting cubes rotate around the main cube and on their own axes.
>     *   The background tunnel texture should animate (shift) to create the illusion of forward movement.
>     *   Target 60 FPS.
> 5.  **Implementation Details**:
>     *   Use `requestAnimationFrame` for the loop.
>     *   Use `Uint8Array` for texture buffers.
>     *   Upload textures to the GPU every frame (`gl.texImage2D`).
>     *   Handle window resizing.
> 
> The final code should be a complete, ready-to-run HTML file.
