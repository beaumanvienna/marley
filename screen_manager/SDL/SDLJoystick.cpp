#include <iostream>
#include <string>
#include "../../include/controller.h"
#include "Common/System/NativeApp.h"
#include "Common/System/System.h"
#include "Common/File/VFS/VFS.h"

#include "Common/File/FileUtil.h"
#include "SDL/SDLJoystick.h"

using namespace std;

static int SCREEN_SDLJoystickEventHandlerWrapper(void* userdata, SDL_Event* event)
{
	static_cast<SCREEN_SDLJoystick *>(userdata)->ProcessInput(*event);
	return 0;
}

SCREEN_SDLJoystick::SCREEN_SDLJoystick(bool init_SDL ) : registeredAsEventHandler(false) {
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    #warning "JC: modified"
	/*if (init_SDL) 
    {
		SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	}

	const char *dbPath = "gamecontrollerdb.txt";
	cout << "loading control pad mappings from " << dbPath << ": ";

	size_t size;
	u8 *mappingData = SCREEN_VFSReadFile(dbPath, &size);
	if (mappingData) {
		SDL_RWops *rw = SDL_RWFromConstMem(mappingData, size);
		// 1 to free the rw after use
		if (SDL_GameControllerAddMappingsFromRW(rw, 1) == -1) {
			cout << "Failed to read mapping data - corrupt?" << endl;
		}
		delete[] mappingData;
	} else {
		cout << "gamecontrollerdb.txt missing" << endl;
	}
	cout << "SUCCESS!" << endl;
    */
	setUpControllers();
}

void SCREEN_SDLJoystick::setUpControllers() {
	/*int numjoys = SDL_NumJoysticks();
	for (int i = 0; i < numjoys; i++) {
		setUpController(i);
	}
	if (controllers.size() > 0) {
		cout << "pad 1 has been assigned to control pad: " << SDL_GameControllerName(controllers.front()) << endl;
	}*/
    int slot = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gDesignatedControllers[i].gameCtrl[0] != NULL)
        {
            setUpController(slot);
            slot++;
        }
    }
}

void SCREEN_SDLJoystick::setUpController(int deviceIndex) {
	/*if (!SDL_IsGameController(deviceIndex)) {
		cout << "Control pad device " << deviceIndex << " not supported by SDL game controller database, attempting to create default mapping..." << endl;
		int cbGUID = 33;
		char pszGUID[cbGUID];
		SDL_Joystick* joystick = SDL_JoystickOpen(deviceIndex);
		if (joystick) {
			SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), pszGUID, cbGUID);
			// create default mapping - this is the PS3 dual shock mapping
			std::string mapping = string(pszGUID) + "," + string(SDL_JoystickName(joystick)) + ",x:b3,a:b0,b:b1,y:b2,back:b8,guide:b10,start:b9,dpleft:b15,dpdown:b14,dpright:b16,dpup:b13,leftshoulder:b4,lefttrigger:a2,rightshoulder:b6,rightshoulder:b5,righttrigger:a5,leftstick:b7,leftstick:b11,rightstick:b12,leftx:a0,lefty:a1,rightx:a3,righty:a4";
			if (SDL_GameControllerAddMapping(mapping.c_str()) == 1){
				cout << "Added default mapping ok" << endl;
			} else {
				cout << "Failed to add default mapping" << endl;
			}
			SDL_JoystickClose(joystick);
		} else {
			cout << "Failed to get joystick identifier. Read-only device? Control pad device " + std::to_string(deviceIndex) << endl;
		}
	}
	SDL_GameController *controller = SDL_GameControllerOpen(deviceIndex);
    */
    int devIndex = gDesignatedControllers[deviceIndex].index[0];
    SDL_GameController *controller = gDesignatedControllers[deviceIndex].gameCtrl[0];
/*	if (controller) {
		if (SDL_GameControllerGetAttached(controller)) {
			controllers.push_back(controller);
			controllerDeviceMap[SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))] = devIndex;
			cout << "found control pad: " << SDL_GameControllerName(controller) << ", loading mapping: ";
			auto mapping = SDL_GameControllerMapping(controller);
			if (mapping == NULL) {
				//cout << "FAILED" << endl;
				cout << "Could not find mapping in SDL2 controller database" << endl;
			} else {
				cout << "SUCCESS, mapping is:" << endl << mapping << endl;
			}
		} else {
			SDL_GameControllerClose(controller);
		}
	}*/
}

SCREEN_SDLJoystick::~SCREEN_SDLJoystick() {
	if (registeredAsEventHandler) {
		SDL_DelEventWatch(SCREEN_SDLJoystickEventHandlerWrapper, this);
	}
	/*for (auto & controller : controllers) {
		SDL_GameControllerClose(controller);
	}*/
}

void SCREEN_SDLJoystick::registerEventHandler() {
	SDL_AddEventWatch(SCREEN_SDLJoystickEventHandlerWrapper, this);
	registeredAsEventHandler = true;
}

keycode_t SCREEN_SDLJoystick::getKeycodeForButton(SDL_GameControllerButton button) {
	switch (button) {
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		return NKCODE_DPAD_UP;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		return NKCODE_DPAD_DOWN;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		return NKCODE_DPAD_LEFT;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		return NKCODE_DPAD_RIGHT;
	case SDL_CONTROLLER_BUTTON_A:
		return NKCODE_BUTTON_2;
	case SDL_CONTROLLER_BUTTON_B:
		return NKCODE_BUTTON_3;
	case SDL_CONTROLLER_BUTTON_X:
		return NKCODE_BUTTON_4;
	case SDL_CONTROLLER_BUTTON_Y:
		return NKCODE_BUTTON_1;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		return NKCODE_BUTTON_5;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		return NKCODE_BUTTON_6;
	case SDL_CONTROLLER_BUTTON_START:
		return NKCODE_BUTTON_STRT;
	case SDL_CONTROLLER_BUTTON_BACK:
		return NKCODE_BUTTON_9;
	case SDL_CONTROLLER_BUTTON_GUIDE:
		return NKCODE_BACK;
	case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		return NKCODE_BUTTON_THUMBL;
	case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
		return NKCODE_BUTTON_THUMBR;
	case SDL_CONTROLLER_BUTTON_INVALID:
	default:
		return NKCODE_UNKNOWN;
	}
}

void SCREEN_SDLJoystick::ProcessInput(SDL_Event &event){
	switch (event.type) {
	case SDL_CONTROLLERBUTTONDOWN:
		if (event.cbutton.state == SDL_PRESSED) {
			auto code = getKeycodeForButton((SDL_GameControllerButton)event.cbutton.button);
			if (code != NKCODE_UNKNOWN) {
				SCREEN_KeyInput key;
				key.flags = KEY_DOWN;
				key.keyCode = code;
				key.deviceId = DEVICE_ID_PAD_0 + getDeviceIndex(event.cbutton.which);
				SCREEN_NativeKey(key);
			}
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		if (event.cbutton.state == SDL_RELEASED) {
			auto code = getKeycodeForButton((SDL_GameControllerButton)event.cbutton.button);
			if (code != NKCODE_UNKNOWN) {
				SCREEN_KeyInput key;
				key.flags = KEY_UP;
				key.keyCode = code;
				key.deviceId = DEVICE_ID_PAD_0 + getDeviceIndex(event.cbutton.which);
				SCREEN_NativeKey(key);
			}
		}
		break;
	case SDL_CONTROLLERAXISMOTION:
		SCREEN_AxisInput axis;
		axis.axisId = event.caxis.axis;
		// 1.2 to try to approximate the PSP's clamped rectangular range.
		axis.value = 1.2 * event.caxis.value * 1.0f / 32767.0f;
		if (axis.value > 1.0f) axis.value = 1.0f;
		if (axis.value < -1.0f) axis.value = -1.0f;
		axis.deviceId = DEVICE_ID_PAD_0 + getDeviceIndex(event.caxis.which);
		axis.flags = 0;
		SCREEN_NativeAxis(axis);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		// for removal events, "which" is the instance ID for SDL_CONTROLLERDEVICEREMOVED		
		for (auto it = controllers.begin(); it != controllers.end(); ++it) {
			if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(*it)) == event.cdevice.which) {
				SDL_GameControllerClose(*it);
				controllers.erase(it);
				break;
			}
		}
		break;
	case SDL_CONTROLLERDEVICEADDED:
		// for add events, "which" is the device index!
		int prevNumControllers = controllers.size();
		setUpController(event.cdevice.which);
		if (prevNumControllers == 0 && controllers.size() > 0) {
			cout << "pad 1 has been assigned to control pad: " << SDL_GameControllerName(controllers.front()) << endl;
		}
		break;
	}
}

int SCREEN_SDLJoystick::getDeviceIndex(int instanceId) {
	auto it = controllerDeviceMap.find(instanceId);
	if (it == controllerDeviceMap.end()) {
			// could not find device
			return -1;
	}
	return it->second;
}
