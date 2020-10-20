/*  OnePAD - author: arcum42(@gmail.com)
 *  Copyright (C) 2009
 *
 *  Based on ZeroPAD, author zerofrog@gmail.com
 *  Copyright (C) 2006-2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "joystick.h"
#include "resources.h"
#include "../../../../include/controller.h"
#include <signal.h> // sigaction

//////////////////////////
// Joystick definitions //
//////////////////////////

// opens all joysticks
void JoystickInfo::EnumerateJoysticks(std::vector<std::unique_ptr<GamePad>> &vjoysticks)
{
    uint32_t flag = SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER;

    
    // Tell SDL to catch event even if the window is not focussed
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    if (SDL_Init(flag) < 0)
    {
        printf("SDL_Init(flag): failure\n");
        return;
    }

    struct sigaction action = {};
    action.sa_handler = SIG_DFL;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);

    SDL_JoystickEventState(SDL_QUERY);
    SDL_GameControllerEventState(SDL_QUERY);
    SDL_EventState(SDL_CONTROLLERDEVICEADDED, SDL_ENABLE);
    SDL_EventState(SDL_CONTROLLERDEVICEREMOVED, SDL_ENABLE);

    vjoysticks.clear();
    int slot = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gDesignatedControllers[i].gameCtrl[0] != NULL)
        {
            vjoysticks.push_back(std::unique_ptr<GamePad>(new JoystickInfo(slot)));
            // If something goes wrong in the init, let's drop it
            if (!vjoysticks.back()->IsProperlyInitialized())
            {
                vjoysticks.pop_back();
            }
            slot++;
        }
    }
}

void JoystickInfo::Rumble(unsigned type, unsigned pad)
{
    if (type >= m_effects_id.size())
        return;

    if (!(g_conf.pad_options[pad].forcefeedback))
        return;

    if (m_haptic == nullptr)
        return;

    int id = m_effects_id[type];
    if (SDL_HapticRunEffect(m_haptic, id, 1) != 0) {
        fprintf(stderr, "ERROR: Effect is not working! %s, id is %d\n", SDL_GetError(), id);
    }
}

JoystickInfo::~JoystickInfo()
{
}

JoystickInfo::JoystickInfo(int slot)
    : GamePad()
    , m_controller(nullptr)
    , m_haptic(nullptr)
    , m_unique_id(0)
{
    SDL_Joystick *joy = nullptr;
    m_effects_id.fill(-1);
    // Values are hardcoded currently but it could be later extended to allow remapping of the buttons
    m_pad_to_sdl[PAD_L2] = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
    m_pad_to_sdl[PAD_R2] = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
    m_pad_to_sdl[PAD_L1] = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    m_pad_to_sdl[PAD_R1] = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    m_pad_to_sdl[PAD_TRIANGLE] = SDL_CONTROLLER_BUTTON_Y;
    m_pad_to_sdl[PAD_CIRCLE] = SDL_CONTROLLER_BUTTON_B;
    m_pad_to_sdl[PAD_CROSS] = SDL_CONTROLLER_BUTTON_A;
    m_pad_to_sdl[PAD_SQUARE] = SDL_CONTROLLER_BUTTON_X;
    m_pad_to_sdl[PAD_SELECT] = SDL_CONTROLLER_BUTTON_BACK;
    m_pad_to_sdl[PAD_L3] = SDL_CONTROLLER_BUTTON_LEFTSTICK;
    m_pad_to_sdl[PAD_R3] = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
    m_pad_to_sdl[PAD_START] = SDL_CONTROLLER_BUTTON_START;
    m_pad_to_sdl[PAD_UP] = SDL_CONTROLLER_BUTTON_DPAD_UP;
    m_pad_to_sdl[PAD_RIGHT] = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    m_pad_to_sdl[PAD_DOWN] = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
    m_pad_to_sdl[PAD_LEFT] = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
    m_pad_to_sdl[PAD_L_UP] = SDL_CONTROLLER_AXIS_LEFTY;
    m_pad_to_sdl[PAD_L_RIGHT] = SDL_CONTROLLER_AXIS_LEFTX;
    m_pad_to_sdl[PAD_L_DOWN] = SDL_CONTROLLER_AXIS_LEFTY;
    m_pad_to_sdl[PAD_L_LEFT] = SDL_CONTROLLER_AXIS_LEFTX;
    m_pad_to_sdl[PAD_R_UP] = SDL_CONTROLLER_AXIS_RIGHTY;
    m_pad_to_sdl[PAD_R_RIGHT] = SDL_CONTROLLER_AXIS_RIGHTX;
    m_pad_to_sdl[PAD_R_DOWN] = SDL_CONTROLLER_AXIS_RIGHTY;
    m_pad_to_sdl[PAD_R_LEFT] = SDL_CONTROLLER_AXIS_RIGHTX;
    m_pad_to_sdl[PAD_GUIDE] = SDL_CONTROLLER_BUTTON_GUIDE;

    
    m_controller = gDesignatedControllers[slot].gameCtrl[0];
    joy = SDL_GameControllerGetJoystick(m_controller);


    if (joy == nullptr) {
        fprintf(stderr, "onepad:failed to open joystick %d\n", slot);
        return;
    }

    // Collect Device Information
    char guid[64];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joy), guid, 64);
    const char *devname = SDL_JoystickNameForIndex(slot);

    if (m_controller == nullptr) {
        fprintf(stderr, "onepad: Joystick (%s,GUID:%s) isn't yet supported by the SDL2 game controller API\n"
                        "Fortunately you can use AntiMicro (https://github.com/AntiMicro/antimicro) or Steam to configure your joystick\n"
                        "The mapping can be stored in OnePAD2.ini as 'SDL2 = <...mapping description...>'\n"
                        "Please report it to us (https://github.com/PCSX2/pcsx2/issues) so we can add your joystick to our internal database.",
                devname, guid);

        #if SDL_MINOR_VERSION >= 4 // Version before 2.0.4 are bugged, JoystickClose crashes randomly
            SDL_JoystickClose(joy);
        #endif

        return;
    }

    std::hash<std::string> hash_me;
    m_unique_id = hash_me(std::string(guid));

    // Default haptic effect
    SDL_HapticEffect effects[NB_EFFECT];
    for (int i = 0; i < NB_EFFECT; i++) {
        SDL_HapticEffect effect;
        memset(&effect, 0, sizeof(SDL_HapticEffect)); // 0 is safe default
        SDL_HapticDirection direction;
        direction.type = SDL_HAPTIC_POLAR; // We'll be using polar direction encoding.
        direction.dir[0] = 18000;
        effect.periodic.direction = direction;
        effect.periodic.period = 10;
        effect.periodic.magnitude = (Sint16)(g_conf.get_ff_intensity()); // Effect at maximum instensity
        effect.periodic.offset = 0;
        effect.periodic.phase = 18000;
        effect.periodic.length = 125; // 125ms feels quite near to original
        effect.periodic.delay = 0;
        effect.periodic.attack_length = 0;
        /* Sine and triangle are quite probably the best, don't change that lightly and if you do
         * keep effects ordered by type
         */
        if (i == 0) {
            /* Effect for small motor */
            /* Sine seems to be the only effect making little motor from DS3/4 react
             * Intensity has pretty much no effect either(which is coherent with what is explain in hid_sony driver
             */
            effect.type = SDL_HAPTIC_SINE;
        } else {
            /** Effect for big motor **/
            effect.type = SDL_HAPTIC_TRIANGLE;
        }

        effects[i] = effect;
    }

    if (SDL_JoystickIsHaptic(joy)) {
        m_haptic = SDL_HapticOpenFromJoystick(joy);

        for (auto &eid : m_effects_id) {
            eid = SDL_HapticNewEffect(m_haptic, &effects[0]);
            if (eid < 0) {
                fprintf(stderr, "ERROR: Effect is not uploaded! %s\n", SDL_GetError());
                m_haptic = nullptr;
                break;
            }
        }
    }

    fprintf(stdout, "onepad: controller (%s) detected%s, GUID:%s\n",
            devname, m_haptic ? " with rumble support" : "", guid);

    m_no_error = true;
}

const char *JoystickInfo::GetName()
{
    return SDL_JoystickName(SDL_GameControllerGetJoystick(m_controller));
}

size_t JoystickInfo::GetUniqueIdentifier()
{
    return m_unique_id;
}

bool JoystickInfo::TestForce(float strength = 0.60)
{
    // This code just use standard rumble to check that SDL handles the pad correctly! --3kinox
    if (m_haptic == nullptr)
        return false; // Otherwise, core dump!

    SDL_HapticRumbleInit(m_haptic);

    // Make the haptic pad rumble 60% strength for half a second, shoudld be enough for user to see if it works or not
    if (SDL_HapticRumblePlay(m_haptic, strength, 400) != 0) {
        fprintf(stderr, "ERROR: Rumble is not working! %s\n", SDL_GetError());
        return false;
    }

    return true;
}

int JoystickInfo::GetInput(gamePadValues input)
{
    float k = g_conf.get_sensibility() / 100.0; // convert sensibility to float

    // Handle analog inputs which range from -32k to +32k. Range conversion is handled later in the controller
    if (IsAnalogKey(input)) {
        int value = SDL_GameControllerGetAxis(m_controller, (SDL_GameControllerAxis)m_pad_to_sdl[input]);
        value *= k;
        return (abs(value) > m_deadzone) ? value : 0;
    }

    // Handle triggers which range from 0 to +32k. They must be converted to 0-255 range
    if (input == PAD_L2 || input == PAD_R2) {
        int value = SDL_GameControllerGetAxis(m_controller, (SDL_GameControllerAxis)m_pad_to_sdl[input]);
        return (value > m_deadzone) ? value / 128 : 0;
    }

    // Map buttons
    int value = SDL_GameControllerGetButton(m_controller, (SDL_GameControllerButton)m_pad_to_sdl[input]);
    return value ? 0xFF : 0; // Max pressure
}

void JoystickInfo::UpdateGamePadState()
{
    SDL_GameControllerUpdate();
}
