#include "Input.h"


bool Input::keysTriggered[NUM_KEYS]{false};
bool Input::keysHeld[NUM_KEYS]{false};
bool Input::keysRelease[NUM_KEYS]{false};
glm::vec2 Input::mousePos{};
glm::vec2 Input::mouseChange{};

bool Input::mouseButtonTriggered[3];
bool Input::mouseButtonHeld[3];
bool Input::mouseButtonRelease[3]; 

short Input::wheelDelta{0};

void Input::Begin()
{
	mouseChange={};
	wheelDelta = {};

	memset(keysTriggered, false, NUM_KEYS);
	//memset(keysHeld, false, NUM_KEYS);
	memset(keysRelease, false, NUM_KEYS);

	memset(mouseButtonTriggered, false, 3);
	memset(mouseButtonRelease, false, 3);

}

 bool Input::GetKeyTriggered(int32_t key)
{
	return keysTriggered[key];
}

 bool Input::GetKeyHeld(int32_t key)
{
	return keysHeld[key];
}

 bool Input::GetKeyReleased(int32_t key)
{
	return keysRelease[key];
}

 void Input::HandleMouseMove(int32_t x, int32_t y)
 {
	 int32_t dx = (int32_t)mousePos.x - x;
	 int32_t dy = (int32_t)mousePos.y - y;

	 mouseChange = { dx,dy };

	 bool handled = false;

	 //if (settings.overlay) {
	//	 ImGuiIO& io = ImGui::GetIO();
	//	 handled = io.WantCaptureMouse;
	 //}
	 //mouseMoved((float)x, (float)y, handled);

	// if (handled) {
	//	 mousePos = glm::vec2((float)x, (float)y);
	//	 return;
	// }

	 mousePos = glm::vec2((float)x, (float)y);
 }

 glm::vec2 Input::GetMouseDelta()
 {
	 return mouseChange;
 }

 glm::vec2 Input::GetMousePos()
 {
	 return mousePos;
 }

 float Input::GetMouseWheel()
 {
	 return static_cast<float>(wheelDelta);
 }

 bool Input::GetMouseTriggered(int32_t key)
 {
	 switch (key)
	 {
	 case MOUSE_LEFT:
	 return mouseButtonTriggered[MouseButton::left];
	 break;
	 case MOUSE_RIGHT:
	 return mouseButtonTriggered[MouseButton::right];
	 break;
	 case MOUSE_MID:
	 return mouseButtonTriggered[MouseButton::middle];
	 break;
	 }
     return false;
 }

 bool Input::GetMouseHeld(int32_t key)
 {
	 switch (key)
	 {
	 case MOUSE_LEFT:
	 return mouseButtonHeld[MouseButton::left];
	 break;
	 case MOUSE_RIGHT:
	 return mouseButtonHeld[MouseButton::right];
	 break;
	 case MOUSE_MID:
	 return mouseButtonHeld[MouseButton::middle];
	 break;
	 }
	 return false;
 }

 bool Input::GetMouseReleased(int32_t key)
 {
	 switch (key)
	 {
	 case MOUSE_LEFT:
	 return mouseButtonRelease[MouseButton::left];
	 break;
	 case MOUSE_RIGHT:
	 return mouseButtonRelease[MouseButton::right];
	 break;
	 case MOUSE_MID:
	 return mouseButtonRelease[MouseButton::middle];
	 break;
	 }
	 return false;
 }

