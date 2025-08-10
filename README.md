Here’s the English translation of your README while keeping the tone clean but not over-the-top:

# Aether-GameEngine

A lightweight and modular C++ game engine utilizing low-level rendering via Vulkan.

## Project Goals

- **Minimalistic yet powerful** engine with a flexible architecture.
- **Cross-platform** — Windows, Linux (planned).
- **Modular design**, allowing easy integration of rendering, input, physics, AI, etc.
- **High performance** through optimized Vulkan API usage.
- **Educational value** — a great foundation for learning game engine development.

## Status

- Currently no functionality, but this repository is a mirror of future achievements.

## Roadmap

- Vulkan initialization, window creation (via GLFW / SDL2).
- Resource manager — shaders, textures, models.
- Render pipeline — 2D and 3D rendering.
- Scene with basic objects, cameras, materials.
- OpenGL shaders → Vulkan SPIR-V.
- Tools: linters, CI, builds for different configurations (Debug/Release).

## Build Instructions

```bash
git clone https://github.com/A-str0/Aether-GameEngine.git
cd Aether-GameEngine
mkdir build && cd build
cmake ..
cmake --build .
```

## Contributing

* Report ideas and bugs via **Issues**.
* Discuss architecture and module proposals.
* Pull requests are welcome — code formatting, tests, and examples are appreciated!

## License

This project is licensed under the **MIT License**. See the `LICENSE` file for details.
