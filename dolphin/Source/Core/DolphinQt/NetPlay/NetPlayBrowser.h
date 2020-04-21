// Copyright 2019 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <map>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <QDialog>

#include "Common/Event.h"
#include "Common/Flag.h"
#include "UICommon/NetPlayIndex.h"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QTableWidget;

class NetPlayBrowser : public QDialog
{
  Q_OBJECT
public:
  explicit NetPlayBrowser(QWidget* parent = nullptr);
  ~NetPlayBrowser();

  void accept() override;
signals:
  void Join();

private:
  void CreateWidgets();
  void ConnectWidgets();

  void Refresh();
  void RefreshLoop();
  void UpdateList();

  void OnSelectionChanged();

  QComboBox* m_region_combo;
  QLabel* m_status_label;
  QPushButton* m_button_refresh;
  QTableWidget* m_table_widget;
  QDialogButtonBox* m_button_box;
  QLineEdit* m_edit_name;
  QLineEdit* m_edit_game_id;
  QCheckBox* m_check_hide_incompatible;
  QCheckBox* m_check_hide_ingame;

  QRadioButton* m_radio_all;
  QRadioButton* m_radio_private;
  QRadioButton* m_radio_public;

  std::vector<NetPlaySession> m_sessions;

  std::thread m_refresh_thread;
  std::optional<std::map<std::string, std::string>> m_refresh_filters;
  std::mutex m_refresh_filters_mutex;
  Common::Flag m_refresh_run;
  Common::Event m_refresh_event;
};
