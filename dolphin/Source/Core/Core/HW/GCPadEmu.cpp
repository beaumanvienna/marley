// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Core/HW/GCPadEmu.h"

#include <array>

#include "Common/Common.h"
#include "Common/CommonTypes.h"

#include "InputCommon/ControllerEmu/Control/Input.h"
#include "InputCommon/ControllerEmu/Control/Output.h"
#include "InputCommon/ControllerEmu/ControlGroup/AnalogStick.h"
#include "InputCommon/ControllerEmu/ControlGroup/Buttons.h"
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/ControllerEmu/ControlGroup/MixedTriggers.h"
#include "InputCommon/ControllerEmu/StickGate.h"

#include "InputCommon/GCPadStatus.h"

static const u16 button_bitmasks[] = {
    PAD_BUTTON_A,
    PAD_BUTTON_B,
    PAD_BUTTON_X,
    PAD_BUTTON_Y,
    PAD_TRIGGER_Z,
    PAD_BUTTON_START,
    PAD_BUTTON_GUIDE,
    0  // MIC HAX
};

static const u16 trigger_bitmasks[] = {
    PAD_TRIGGER_L,
    PAD_TRIGGER_R,
};

static const u16 dpad_bitmasks[] = {PAD_BUTTON_UP, PAD_BUTTON_DOWN, PAD_BUTTON_LEFT,
                                    PAD_BUTTON_RIGHT};


static const char* const named_buttons[] = {"A", "B", "X", "Y", "Z", "Start","Guide"};

static const char* const named_triggers[] = {
    // i18n: The left trigger button (labeled L on real controllers)
    _trans("L"),
    // i18n: The right trigger button (labeled R on real controllers)
    _trans("R"),
    // i18n: The left trigger button (labeled L on real controllers) used as an analog input
    _trans("L-Analog"),
    // i18n: The right trigger button (labeled R on real controllers) used as an analog input
    _trans("R-Analog")};

GCPad::GCPad(const unsigned int index) : m_index(index)
{
  // buttons
  groups.emplace_back(m_buttons = new ControllerEmu::Buttons(_trans("Buttons")));
  for (const char* named_button : named_buttons)
  {
    const bool is_start = named_button == std::string("Start");
    const ControllerEmu::Translatability translate =
        is_start ? ControllerEmu::Translate : ControllerEmu::DoNotTranslate;
    // i18n: The START/PAUSE button on GameCube controllers
    std::string ui_name = is_start ? _trans("START") : named_button;
    m_buttons->AddInput(translate, named_button, std::move(ui_name));
  }

  // sticks
  groups.emplace_back(m_main_stick = new ControllerEmu::OctagonAnalogStick(
                          "Main Stick", _trans("Control Stick"), MAIN_STICK_GATE_RADIUS));
  groups.emplace_back(m_c_stick = new ControllerEmu::OctagonAnalogStick(
                          "C-Stick", _trans("C Stick"), C_STICK_GATE_RADIUS));

  // triggers
  groups.emplace_back(m_triggers = new ControllerEmu::MixedTriggers(_trans("Triggers")));
  for (const char* named_trigger : named_triggers)
  {
    m_triggers->AddInput(ControllerEmu::Translate, named_trigger);
  }

  // rumble
  groups.emplace_back(m_rumble = new ControllerEmu::ControlGroup(_trans("Rumble")));
  m_rumble->AddOutput(ControllerEmu::Translate, _trans("Motor"));

  // Microphone
  groups.emplace_back(m_mic = new ControllerEmu::Buttons(_trans("Microphone")));
  m_mic->AddInput(ControllerEmu::Translate, _trans("Button"));

  // dpad
  groups.emplace_back(m_dpad = new ControllerEmu::Buttons(_trans("D-Pad")));
  for (const char* named_direction : named_directions)
  {
    m_dpad->AddInput(ControllerEmu::Translate, named_direction);
  }

  // options
  groups.emplace_back(m_options = new ControllerEmu::ControlGroup(_trans("Options")));
  m_options->AddSetting(&m_always_connected_setting,
                        // i18n: Treat a controller as always being connected regardless of what
                        // devices the user actually has plugged in
                        _trans("Always Connected"), false);
}

std::string GCPad::GetName() const
{
  return std::string("GCPad") + char('1' + m_index);
}

ControllerEmu::ControlGroup* GCPad::GetGroup(PadGroup group)
{
  switch (group)
  {
  case PadGroup::Buttons:
    return m_buttons;
  case PadGroup::MainStick:
    return m_main_stick;
  case PadGroup::CStick:
    return m_c_stick;
  case PadGroup::DPad:
    return m_dpad;
  case PadGroup::Triggers:
    return m_triggers;
  case PadGroup::Rumble:
    return m_rumble;
  case PadGroup::Mic:
    return m_mic;
  case PadGroup::Options:
    return m_options;
  default:
    return nullptr;
  }
}

GCPadStatus GCPad::GetInput() const
{
  const auto lock = GetStateLock();
  GCPadStatus pad = {};

  if (!(m_always_connected_setting.GetValue() || IsDefaultDeviceConnected()))
  {
    pad.isConnected = false;
    return pad;
  }

  // buttons
  m_buttons->GetState(&pad.button, button_bitmasks);

  // set analog A/B analog to full or w/e, prolly not needed
  if (pad.button & PAD_BUTTON_A)
    pad.analogA = 0xFF;
  if (pad.button & PAD_BUTTON_B)
    pad.analogB = 0xFF;

  // dpad
  m_dpad->GetState(&pad.button, dpad_bitmasks);

  // sticks
  const auto main_stick_state = m_main_stick->GetState();
  pad.stickX = MapFloat<u8>(main_stick_state.x, GCPadStatus::MAIN_STICK_CENTER_X);
  pad.stickY = MapFloat<u8>(main_stick_state.y, GCPadStatus::MAIN_STICK_CENTER_Y);

  const auto c_stick_state = m_c_stick->GetState();
  pad.substickX = MapFloat<u8>(c_stick_state.x, GCPadStatus::C_STICK_CENTER_X);
  pad.substickY = MapFloat<u8>(c_stick_state.y, GCPadStatus::C_STICK_CENTER_Y);

  // triggers
  std::array<ControlState, 2> triggers;
  m_triggers->GetState(&pad.button, trigger_bitmasks, triggers.data());
  pad.triggerLeft = MapFloat<u8>(triggers[0], 0);
  pad.triggerRight = MapFloat<u8>(triggers[1], 0);

  return pad;
}

void GCPad::SetOutput(const ControlState strength)
{
  const auto lock = GetStateLock();
  m_rumble->controls[0]->control_ref->State(strength);
}

void GCPad::LoadDefaults(const ControllerInterface& ciface,int n)
{
    #warning "jc: modified" // analog triggers should be added
  EmulatedController::LoadDefaults(ciface,n);

  // Buttons
  m_buttons->SetControlExpression(0, "`Button 0`");  // A
  m_buttons->SetControlExpression(1, "`Button 1`");  // B
  m_buttons->SetControlExpression(2, "`Button 2`");  // X
  m_buttons->SetControlExpression(3, "`Button 3`");  // Y
  m_buttons->SetControlExpression(4, "`Button 10`");  // Z
  m_buttons->SetControlExpression(5, "`Button 6`");  // Start
  m_buttons->SetControlExpression(6, "`Button 5`");  // Guide


  // stick modifiers to 50 %
  m_main_stick->controls[4]->control_ref->range = 0.5f;
  m_c_stick->controls[4]->control_ref->range = 0.5f;

  // D-Pad
  m_dpad->SetControlExpression(0, "`Button 11`");  // Up
  m_dpad->SetControlExpression(1, "`Button 12`");  // Down
  m_dpad->SetControlExpression(2, "`Button 13`");  // Left
  m_dpad->SetControlExpression(3, "`Button 14`");  // Right

  // C Stick
  m_c_stick->SetControlExpression(0, "`Axis 3-`");  // Up
  m_c_stick->SetControlExpression(1, "`Axis 3+`");  // Down
  m_c_stick->SetControlExpression(2, "`Axis 2-`");  // Left
  m_c_stick->SetControlExpression(3, "`Axis 2+`");  // Right

  m_c_stick->SetControlExpression(4, "`Button 8`");  // Modifier

  // Control Stick
  m_main_stick->SetControlExpression(0, "`Axis 1-`");       // Up
  m_main_stick->SetControlExpression(1, "`Axis 1+`");     // Down
  m_main_stick->SetControlExpression(2, "`Axis 0-`");     // Left
  m_main_stick->SetControlExpression(3, "`Axis 0+`");    // Right
  m_main_stick->SetControlExpression(4, "`Button 7`");  // Modifier


  // Because our defaults use keyboard input, set calibration shapes to squares.
  m_c_stick->SetCalibrationFromGate(ControllerEmu::SquareStickGate(1.0));
  m_main_stick->SetCalibrationFromGate(ControllerEmu::SquareStickGate(1.0));

  // Triggers
  m_triggers->SetControlExpression(0, "`Button 9`");  // L
  m_triggers->SetControlExpression(1, "`Button 10`");  // R
}

bool GCPad::GetMicButton() const
{
  const auto lock = GetStateLock();
  return m_mic->controls.back()->GetState<bool>();
}
