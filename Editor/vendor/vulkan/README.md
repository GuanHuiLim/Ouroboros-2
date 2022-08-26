
Ensure you run the "updateExamplesDependencies.bat" in the Build folder to get the dependancies.

### Prerequisites
+ [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
+ [cmake](https://cmake.org/download/)
+ [vulkan](https://vulkan.lunarg.com/)

### Dependencies
To see the examples you will need to run a batch located: **Build/updateExamplesDependencies.bat**
it will gather and build the dependencies:
- [GLM](https://https://github.com/g-truc/glm)
- [assimp](https://github.com/assimp/assimp)


### Controls
The demo will show a model in the centre of the scene.
The surrounding skeleton models are rendered using indirect draw commands.
These commands are using bindless textures to render more than 100 textures using a single draw call.

Spacebar -- toggles light follow camera
Click and drag -- to rotate around the focused object
