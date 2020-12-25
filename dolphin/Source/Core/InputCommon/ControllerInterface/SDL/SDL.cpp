// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/SDL/SDL.h"

#include <algorithm>
#include <thread>

#include <SDL_events.h>

#include "Common/Event.h"
#include "Common/Logging/Log.h"
#include "Common/ScopeGuard.h"
#include "Common/StringUtil.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "../../../../../../include/controller.h"
using namespace std;

bool requestShutdownGUIDE;
namespace ciface::SDL

{
static std::string GetJoystickName(int index)
{
    #warning "JC modified"
    std::string s = "0xbaadf00dbeefbabe - ";
    s+=to_string(index);
  return s;
}

static void OpenAndAddDevice(int slot)
{
    #warning "JC modified"
    if (gDesignatedControllers[slot].gameCtrl[0] != NULL)
    {
        SDL_GameController* const dev = gDesignatedControllers[slot].gameCtrl[0];
        
        auto js = std::make_shared<Joystick>(dev, slot);
        
        // only add if it has some inputs/outputs
        if (!js->Inputs().empty() || !js->Outputs().empty())
        {
            g_controller_interface.AddDevice(std::move(js));
        }
    }
}


static Common::Event s_init_event;
static Common::Event s_populated_event;
static std::thread s_hotplug_thread;


void Init()
{
    requestShutdownGUIDE = false;
    #warning "jc: modified"
    if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
    {
      ERROR_LOG(SERIALINTERFACE, "SDL failed to initialize");
      return;
    }
    
    SDL_JoystickEventState(SDL_IGNORE);
    
    int slot = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gDesignatedControllers[i].gameCtrl[0] != NULL)
        {
            OpenAndAddDevice(slot);
            slot++;
        }
    }
    
    s_hotplug_thread = std::thread([] 
    {
        {
          Common::ScopeGuard init_guard([] { s_init_event.Set(); });
        }
    });

  s_init_event.Wait();

}

void DeInit()
{

#warning "jc: modified"

  if (!s_hotplug_thread.joinable())
    return;

  s_hotplug_thread.join();
}

void PopulateDevices()
{
    if (!s_hotplug_thread.joinable())
      return;

    int slot = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gDesignatedControllers[i].gameCtrl[0] != NULL)
        {
            OpenAndAddDevice(slot);
            slot++;
        }
    }
    s_populated_event.Set();
}

Joystick::Joystick(SDL_GameController* const joystick, const int sdl_index)
    : m_joystick(joystick), m_name(StripSpaces(GetJoystickName(sdl_index)))
{


  // get buttons
  for (u8 i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
  {
    AddInput(new Button(i, m_joystick));
  }

  // get axes
  for (u8 i = 0; i != SDL_CONTROLLER_AXIS_MAX; ++i)
  {
    // each axis gets a negative and a positive input instance associated with it
    AddAnalogInputs(new Axis(i, m_joystick, -32768), new Axis(i, m_joystick, 32767));
  }

#ifdef USE_SDL_HAPTIC
  m_haptic = SDL_HapticOpenFromJoystick(m_joystick);
  if (!m_haptic)
    return;

  const unsigned int supported_effects = SDL_HapticQuery(m_haptic);

  // Disable autocenter:
  if (supported_effects & SDL_HAPTIC_AUTOCENTER)
    SDL_HapticSetAutocenter(m_haptic, 0);

  // Constant
  if (supported_effects & SDL_HAPTIC_CONSTANT)
    AddOutput(new ConstantEffect(m_haptic));

  // Ramp
  if (supported_effects & SDL_HAPTIC_RAMP)
    AddOutput(new RampEffect(m_haptic));

  // Periodic
  for (auto waveform :
       {SDL_HAPTIC_SINE, SDL_HAPTIC_TRIANGLE, SDL_HAPTIC_SAWTOOTHUP, SDL_HAPTIC_SAWTOOTHDOWN})
  {
    if (supported_effects & waveform)
      AddOutput(new PeriodicEffect(m_haptic, waveform));
  }

  // LeftRight
  if (supported_effects & SDL_HAPTIC_LEFTRIGHT)
  {
    AddOutput(new LeftRightEffect(m_haptic, LeftRightEffect::Motor::Strong));
    AddOutput(new LeftRightEffect(m_haptic, LeftRightEffect::Motor::Weak));
  }
#endif
}

Joystick::~Joystick()
{
#ifdef USE_SDL_HAPTIC
  if (m_haptic)
  {
    // stop/destroy all effects
    SDL_HapticStopAll(m_haptic);
    // close haptic first
    SDL_HapticClose(m_haptic);
  }
#endif

  // close joystick
  #warning "jc: modfied"
  // keep the controller for marley
  //SDL_JoystickClose(m_joystick);
}

#ifdef USE_SDL_HAPTIC
void Joystick::HapticEffect::UpdateEffect()
{
  if (m_effect.type != DISABLED_EFFECT_TYPE)
  {
    if (m_id < 0)
    {
      // Upload and try to play the effect.
      m_id = SDL_HapticNewEffect(m_haptic, &m_effect);

      if (m_id >= 0)
        SDL_HapticRunEffect(m_haptic, m_id, 1);
    }
    else
    {
      // Effect is already playing. Update parameters.
      SDL_HapticUpdateEffect(m_haptic, m_id, &m_effect);
    }
  }
  else if (m_id >= 0)
  {
    // Stop and remove the effect.
    SDL_HapticStopEffect(m_haptic, m_id);
    SDL_HapticDestroyEffect(m_haptic, m_id);
    m_id = -1;
  }
}

Joystick::HapticEffect::HapticEffect(SDL_Haptic* haptic) : m_haptic(haptic)
{
  // FYI: type is set within UpdateParameters.
  m_effect.type = DISABLED_EFFECT_TYPE;
}

Joystick::HapticEffect::~HapticEffect()
{
  m_effect.type = DISABLED_EFFECT_TYPE;
  UpdateEffect();
}

void Joystick::HapticEffect::SetDirection(SDL_HapticDirection* dir)
{
  // Left direction (for wheels)
  dir->type = SDL_HAPTIC_CARTESIAN;
  dir->dir[0] = -1;
}

Joystick::ConstantEffect::ConstantEffect(SDL_Haptic* haptic) : HapticEffect(haptic)
{
  m_effect.constant = {};
  SetDirection(&m_effect.constant.direction);
  m_effect.constant.length = RUMBLE_LENGTH_MS;
}

Joystick::RampEffect::RampEffect(SDL_Haptic* haptic) : HapticEffect(haptic)
{
  m_effect.ramp = {};
  SetDirection(&m_effect.ramp.direction);
  m_effect.ramp.length = RUMBLE_LENGTH_MS;
}

Joystick::PeriodicEffect::PeriodicEffect(SDL_Haptic* haptic, u16 waveform)
    : HapticEffect(haptic), m_waveform(waveform)
{
  m_effect.periodic = {};
  SetDirection(&m_effect.periodic.direction);
  m_effect.periodic.length = RUMBLE_LENGTH_MS;
  m_effect.periodic.period = RUMBLE_PERIOD_MS;
  m_effect.periodic.offset = 0;
  m_effect.periodic.phase = 0;
}

Joystick::LeftRightEffect::LeftRightEffect(SDL_Haptic* haptic, Motor motor)
    : HapticEffect(haptic), m_motor(motor)
{
  m_effect.leftright = {};
  m_effect.leftright.length = RUMBLE_LENGTH_MS;
}

std::string Joystick::ConstantEffect::GetName() const
{
  return "Constant";
}

std::string Joystick::RampEffect::GetName() const
{
  return "Ramp";
}

std::string Joystick::PeriodicEffect::GetName() const
{
  switch (m_waveform)
  {
  case SDL_HAPTIC_SINE:
    return "Sine";
  case SDL_HAPTIC_TRIANGLE:
    return "Triangle";
  case SDL_HAPTIC_SAWTOOTHUP:
    return "Sawtooth Up";
  case SDL_HAPTIC_SAWTOOTHDOWN:
    return "Sawtooth Down";
  default:
    return "Unknown";
  }
}

std::string Joystick::LeftRightEffect::GetName() const
{
  return (Motor::Strong == m_motor) ? "Strong" : "Weak";
}

void Joystick::HapticEffect::SetState(ControlState state)
{
  // Maximum force value for all SDL effects:
  constexpr s16 MAX_FORCE_VALUE = 0x7fff;

  if (UpdateParameters(s16(state * MAX_FORCE_VALUE)))
  {
    UpdateEffect();
  }
}

bool Joystick::ConstantEffect::UpdateParameters(s16 value)
{
  s16& level = m_effect.constant.level;
  const s16 old_level = level;

  level = value;

  m_effect.type = level ? SDL_HAPTIC_CONSTANT : DISABLED_EFFECT_TYPE;
  return level != old_level;
}

bool Joystick::RampEffect::UpdateParameters(s16 value)
{
  s16& level = m_effect.ramp.start;
  const s16 old_level = level;

  level = value;
  // FYI: Setting end to same as start is odd,
  // but so is using Ramp effects for rumble simulation.
  m_effect.ramp.end = level;

  m_effect.type = level ? SDL_HAPTIC_RAMP : DISABLED_EFFECT_TYPE;
  return level != old_level;
}

bool Joystick::PeriodicEffect::UpdateParameters(s16 value)
{
  s16& level = m_effect.periodic.magnitude;
  const s16 old_level = level;

  level = value;

  m_effect.type = level ? m_waveform : DISABLED_EFFECT_TYPE;
  return level != old_level;
}

bool Joystick::LeftRightEffect::UpdateParameters(s16 value)
{
  u16& level = (Motor::Strong == m_motor) ? m_effect.leftright.large_magnitude :
                                            m_effect.leftright.small_magnitude;
  const u16 old_level = level;

  level = value;

  m_effect.type = level ? SDL_HAPTIC_LEFTRIGHT : DISABLED_EFFECT_TYPE;
  return level != old_level;
}
#endif

void Joystick::UpdateInput()
{
  // TODO: Don't call this for every Joystick, only once per ControllerInterface::UpdateInput()
  SDL_GameControllerUpdate();
  
}

std::string Joystick::GetName() const
{
  return m_name;
}

std::string Joystick::GetSource() const
{
  return "SDL";
}

SDL_GameController* Joystick::GetSDLJoystick() const
{
  return m_joystick;
}

std::string Joystick::Button::GetName() const
{
  return "Button " + std::to_string(m_index);
}

std::string Joystick::Axis::GetName() const
{
  return "Axis " + std::to_string(m_index) + (m_range < 0 ? '-' : '+');
}

std::string Joystick::Hat::GetName() const
{
  return "Hat " + std::to_string(m_index) + ' ' + "NESW"[m_direction];
}

ControlState Joystick::Button::GetState() const
{
    Uint8 b = SDL_GameControllerGetButton(m_js, (SDL_GameControllerButton)m_index);
    if (b)
    {
        if (m_index == SDL_CONTROLLER_BUTTON_GUIDE) requestShutdownGUIDE=true;
    }
  return b;
}

ControlState Joystick::Axis::GetState() const
{
  return ControlState(SDL_GameControllerGetAxis(m_js, (SDL_GameControllerAxis)m_index)) / m_range;
}

ControlState Joystick::Hat::GetState() const
{
  // this function won't be called anymore
  //return (SDL_GameControllerGetHat(m_js, (SDL_GameControllerHat)m_index) & (1 << m_direction)) > 0;
  return 0;
}
}  // namespace ciface::SDL
