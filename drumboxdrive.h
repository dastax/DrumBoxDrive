/* injector.h --- 
 * 
 * Filename: injector.h
 * Description: 
 * Author: stax
 * Maintainer: 
 * Created: ven nov 20 23:05:39 2009 (+0100)
 * Version: 
 * Last-Updated: jeu. déc.  7 19:13:09 2017 (+0100)
 *           By: stax
 *     Update #: 58
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 */


#ifndef DRUM_BOX_DRIVE_H
#define DRUM_BOX_DRIVE_H

#include <QtGui>
#include "jack_engine.h"
#include "Bridge.h"

// SYSEX REQUESTS
#define BOX_READY		42
#define SAVE_BOX_CONFIG		1
#define SAVE_BOX_PATTERNS	2
#define SAVE_BOX_PHRASES	3
#define LOAD_BOX_CONFIG		4
#define LOAD_BOX_PATTERNS	5
#define LOAD_BOX_PHRASES	6

class DrumBoxDrive : public QWidget
{
  Q_OBJECT
private:
  JackEngine _jack;
  QString _clientName;
  QString _confFile;
  QString _presetDirectory, _lastConfFilename, _lastPatternFilename;
  QMenu *_menu;
  QSystemTrayIcon _trayIcon;
  QRect _geoMem;

  Bridge *_bridge;
  QThread *_workerThread;
  //QTimer debugListTimer;
  //QStringList debugListMessages;
  
  QVBoxLayout _layout;
  QGroupBox _jackGB;
  QPushButton _jackClientActivation;

  QLabel _staxaudio;
  MidiInputBox _midiIn;
  MidiOutputBox _midiOut;

  QLineEdit _serialPortName;
  QPushButton _serialConnect;
  QListWidget _logText;

  QGroupBox _boxAction;
  QPushButton _saveBoxConfig;
  QPushButton _loadBoxConfig;
  QPushButton _saveBoxPatterns;
  QPushButton _loadBoxPatterns;
    
  void setUpGui();
  void closeEvent(QCloseEvent*);

  void deleteBridge();
  
  bool loadConf();
  bool saveConf();

  void paintEvent(QPaintEvent *);

  bool saveBoxConfig(QString path, QByteArray sysex);
  bool saveBoxPatterns(QString path, QByteArray sysex);
  bool saveBoxPhrases(QString path, QByteArray sysex);

  void loadBoxConfig(QString path);
  void loadBoxPatterns(QString path);
  void loadBoxPhrases(QString path);

  bool gotPresetDirectory();

private slots:
  void jackCtrlClicked();
  void serialConnectClicked();
  
  void saveBoxConfigClicked();
  void loadBoxConfigClicked();
  
  void saveBoxPatternsClicked();
  void loadBoxPatternsClicked();

  void onDisplayMessage(QString message);
  void onErrorMessage(QString message);

  void serialPortOpened();

  void sysExReceived(QByteArray sysex);
  
  void handleMidiEvent(struct MidiEvent);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);


public:
  DrumBoxDrive();
  ~DrumBoxDrive();

};

#endif

/* injector.h ends here */
