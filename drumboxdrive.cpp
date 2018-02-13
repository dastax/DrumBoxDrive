// injector.cpp --- 
// 
// Filename: injector.cpp
// Description: 
// Author: stax
// Maintainer: 
// Created: ven nov 20 23:05:17 2009 (+0100)
// Version: 
// Last-Updated: ven. déc.  8 01:28:09 2017 (+0100)
//           By: stax
//     Update #: 303
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth
// Floor, Boston, MA 02110-1301, USA.
// 
// 

#include <QtXml>
#include <math.h>
#include "drumboxdrive.h"
#include "PortLatency.h"

DrumBoxDrive::DrumBoxDrive()
{
  _menu=NULL;
  _clientName="DrumBoxDrive";
  _confFile=QDir::homePath()+"/.DrumBoxDrive";
  _presetDirectory="";
  
  setUpGui();


  _workerThread=new QThread();
  _workerThread->start(QThread::HighestPriority);

  _bridge=NULL;
  
  jackCtrlClicked();
  loadConf();
  serialConnectClicked();
}
DrumBoxDrive::~DrumBoxDrive()
{
}
void DrumBoxDrive::closeEvent(QCloseEvent*)
{
  saveConf();
  _jack.closeClient();
  if (_bridge)
    _bridge->deleteLater();
  //qDebug()<<"closing";
}
void DrumBoxDrive::setUpGui()
{
  setWindowTitle(QString("DrumBoxDrive"));
  setWindowIcon(QIcon(QPixmap(":/logo")));

  if (_trayIcon.isSystemTrayAvailable()) {
    _trayIcon.setIcon(QIcon(QPixmap(":/logo")));
    _trayIcon.setToolTip(QString("DrumBoxDrive"));
    _trayIcon.show();
  }


  _jackGB.setTitle(tr("Jack client"));
  QHBoxLayout *jc=new QHBoxLayout;
  jc->addWidget(&_jackClientActivation);
  _jackGB.setLayout(jc);

  _staxaudio.setPixmap(QPixmap(":/staxaudio"));
  QHBoxLayout *menubar=new QHBoxLayout;
  menubar->addWidget(&_staxaudio);
  menubar->addStretch();
  menubar->addWidget(&_jackGB);
  menubar->addWidget(&_midiIn);
  menubar->addWidget(&_midiOut);


  _serialPortName.setText("/dev/arduino");
  _serialConnect.setText(tr("Connect"));
  QHBoxLayout *act=new QHBoxLayout;
  act->addWidget(&_serialPortName);
  act->addWidget(&_serialConnect);
  act->addStretch();

  QHBoxLayout *bal=new QHBoxLayout;
  bal->addWidget(&_saveBoxConfig);
  bal->addWidget(&_loadBoxConfig);
  bal->addWidget(&_saveBoxPatterns);
  bal->addWidget(&_loadBoxPatterns);
  bal->addStretch();
  _boxAction.setLayout(bal);
  _saveBoxConfig.setText("Save Config");
  _loadBoxConfig.setText("Load Config");
  _saveBoxPatterns.setText("Save Patterns");
  _loadBoxPatterns.setText("Load Patterns");
  _boxAction.setTitle("DrumBox Commands");
  
  //_logText.setReadOnly(true);
  _layout.addLayout(menubar);
  _layout.addLayout(act);
  _layout.addWidget(&_boxAction);
  _layout.addWidget(&_logText);
  setLayout(&_layout);

  _boxAction.hide();

  connect(&_jackClientActivation, SIGNAL(clicked()),
	  this, SLOT(jackCtrlClicked()));
  connect(&_serialConnect, SIGNAL(clicked()),
	  this, SLOT(serialConnectClicked()));
  connect(&_saveBoxConfig, SIGNAL(clicked()),
	  this, SLOT(saveBoxConfigClicked()));
  connect(&_loadBoxConfig, SIGNAL(clicked()),
	  this, SLOT(loadBoxConfigClicked()));
  connect(&_saveBoxPatterns, SIGNAL(clicked()),
	  this, SLOT(saveBoxPatternsClicked()));
  connect(&_loadBoxPatterns, SIGNAL(clicked()),
	  this, SLOT(loadBoxPatternsClicked()));
  connect(&_jack, SIGNAL(receivedMidiEvent(struct MidiEvent)),
	  this, SLOT(handleMidiEvent(struct MidiEvent)), Qt::QueuedConnection);
  connect(&_jack, SIGNAL(receivedMidiEvent(struct MidiEvent)),
	  &_midiIn, SLOT(midiIn()), Qt::QueuedConnection);
  connect(&_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	  this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
  connect(&_midiIn, SIGNAL(inputChannelChanged(int)),
	  &_jack, SLOT(setInputChannel(int)));
  
}

void DrumBoxDrive::paintEvent(QPaintEvent *)
{
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DrumBoxDrive::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::Trigger) {
    if (!isVisible()) {
      show();
    }
    else {
      _geoMem=geometry();
      hide();
    }
  }
  if (reason == QSystemTrayIcon::Context) {
    //QPoint pos=mapFromGlobal(QCursor::pos());
    //QMouseEvent ev(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    
    //mousePressEvent(&ev);
    if (!_menu) {
      _menu=new QMenu(this);
      _menu->addAction(QIcon(QPixmap(":/exit")), "Close", this, SLOT(close()));
    }
    _menu->exec(QCursor::pos());//mapToGlobal(ev->pos()));

  }
}

void DrumBoxDrive::jackCtrlClicked()
{

  if (_jack.isActive()) {
    _jack.closeClient();
    _jackClientActivation.setText(tr("Start"));
  }
  else {
    if (_jack.initEngine(_clientName)) {
      _jackClientActivation.setText(tr("Stop"));
      //int sr=_jack.getSamplerate();

    }
    else
      QMessageBox::warning(this, tr("Error"), 
			   tr("Jack failed to initialise..."));
  }

}
void DrumBoxDrive::serialConnectClicked()
{
  if(_bridge) {
    deleteBridge();
    return;
  }
  if (_serialPortName.text().isEmpty())
    return;
  _logText.clear();
  _bridge=new Bridge();
  connect(_bridge, SIGNAL(displayMessage(QString)), SLOT(onDisplayMessage(QString)));
  connect(_bridge, SIGNAL(errorMessage(QString)), SLOT(onErrorMessage(QString)));
  //connect(_bridge, SIGNAL(debugMessage(QString)), SLOT(onDisplayMessage(QString)));
  connect(_bridge, SIGNAL(serialPortOpened()), SLOT(serialPortOpened()));
  connect(_bridge, SIGNAL(midiSent()), &_midiOut, SLOT(midiOut()));

  connect(_bridge, SIGNAL(sysExReceived(QByteArray)), SLOT(sysExReceived(QByteArray)));
  
  //connect(_bridge, SIGNAL(midiReceived()), ui->led_midiin, SLOT(blinkOn()));
  //connect(_bridge, SIGNAL(midiSent()), ui->led_midiout, SLOT(blinkOn()));
  //connect(_bridge, SIGNAL(serialTraffic()), ui->led_serial, SLOT(blinkOn()));
  _serialConnect.setText("connecting ...");
  _serialPortName.setEnabled(false);
  _bridge->attach(_serialPortName.text(), &_jack, _workerThread);
}
void DrumBoxDrive::onDisplayMessage(QString msg) {
  _logText.addItem(msg+"\n");
  _logText.scrollToBottom();
}
void DrumBoxDrive::onErrorMessage(QString msg) {
  onDisplayMessage(msg);
  deleteBridge();
}
void DrumBoxDrive::serialPortOpened() {
  _serialConnect.setText(tr("Disconnect"));
  _serialConnect.setEnabled(true);
  onDisplayMessage(tr("Succesfully opened %1").arg(_serialPortName.text()));
  _boxAction.show();
}
void DrumBoxDrive::deleteBridge() {
  if(_bridge) {
    _bridge->deleteLater();
    QThread::yieldCurrentThread(); // Try and get any signals from the bridge sent sooner not later
    QCoreApplication::processEvents();
    _bridge=NULL;
    _serialConnect.setText(tr("Connect"));
    _serialPortName.setEnabled(true);
    _boxAction.hide();
  }
}
bool DrumBoxDrive::gotPresetDirectory() {
  if (!_presetDirectory.isEmpty())
    return true;
  if (QMessageBox::information(this, tr("Choose preset directory"),
			       tr("The preset directory isn't defined yet, please choose one !"))
      != QMessageBox::Ok)
    return false;
  QString dir=QFileDialog::getExistingDirectory(this, tr("Choose Preset Directory"),
				    QDir::homePath());
  if (dir.isEmpty())
    return false;
  _presetDirectory=dir;
  return true;
}
void DrumBoxDrive::saveBoxConfigClicked() {
  if (!_bridge)
    return;
  if (!gotPresetDirectory())
    return;
  QString fileName=QFileDialog::getSaveFileName(this, tr("Open File"),
                                                _presetDirectory,
                                                tr("DrumBox Conf file (*.dbc)"));
  if (fileName.isNull())
    return;
  _lastConfFilename=fileName;

  QByteArray sysex;
  sysex.append(42);
  sysex.append(SAVE_BOX_CONFIG);
  _bridge->sendSysEx(sysex);
}
void DrumBoxDrive::loadBoxConfigClicked() {
  if (!_bridge)
    return;
  if (!gotPresetDirectory())
    return;
  QString fileName=QFileDialog::getOpenFileName(this, tr("Open File"),
                                                _presetDirectory,
                                                tr("DrumBox Conf file (*.dbc)"));
  if (fileName.isNull())
    return;

  _lastConfFilename=fileName;
  loadBoxConfig(fileName);
}
void DrumBoxDrive::loadBoxConfig(QString fileName) {
  QDomDocument conf;
  QFile f(fileName);
  if (!f.open(QIODevice::ReadOnly)) {
    onDisplayMessage("Error opening '"+fileName+"' file can't be read");
    return;
  }
  QString errorStr;
  int errorLine, errorColumn;
  if (!conf.setContent(&f, true, &errorStr, &errorLine, &errorColumn)) {
    onDisplayMessage(QString("Parse error at line %1, column %2:%3")
		     .arg(errorLine).arg(errorColumn).arg(errorStr));
    f.close();
    return;
  }
  f.close();
  QDomElement root=conf.documentElement();
  if (root.tagName() != "DrumBoxConf") {
    onDisplayMessage("The file is not a conf file for this app.");
    return;
  }

  QByteArray sysex;
  sysex.append(42);
  sysex.append(LOAD_BOX_CONFIG);
  
  QDomNodeList trs;
  QDomElement elt;

  trs=conf.elementsByTagName("tracks_conf");
  if (trs.length() == 0) {
    onDisplayMessage("uncomplete file, missing values");
    return;
  }
  elt=trs.item(0).toElement();
  uint track_count=elt.attribute("track_count").toInt();
  sysex.append(track_count);
  
  trs=conf.elementsByTagName("track");
  if (trs.length() != track_count) {
    onDisplayMessage("uncomplete file, missing values");
    return;
  }
  for (uint i=0; i< trs.length(); i++) {
    elt=trs.item(i).toElement();
    sysex.append(elt.attribute("midi_channel").toInt());
    sysex.append(elt.attribute("midi_note").toInt());
  }
  onDisplayMessage(QString("Loading Conf %1 to DrumBox ...").arg(fileName));
  _bridge->sendSysEx(sysex);
}
void DrumBoxDrive::saveBoxPatternsClicked() {
  if (!_bridge)
    return;
  if (!gotPresetDirectory())
    return;
  QString fileName=QFileDialog::getSaveFileName(this, tr("Open File"),
                                                _presetDirectory,
                                                tr("DrumBox Pattern file (*.dbp)"));
  if (fileName.isNull())
    return;
  _lastPatternFilename=fileName;

  QByteArray sysex;
  sysex.append(42);
  sysex.append(SAVE_BOX_PATTERNS);
  _bridge->sendSysEx(sysex);
}
void DrumBoxDrive::loadBoxPatternsClicked() {
  if (!_bridge)
    return;
  if (!gotPresetDirectory())
    return;
  QString fileName=QFileDialog::getOpenFileName(this, tr("Open File"),
                                                _presetDirectory,
                                                tr("DrumBox Pattern file (*.dbp)"));
  if (fileName.isNull()) {
    onDisplayMessage("Pattern load cancel...");
    return;
  }
  _lastPatternFilename=fileName;
  loadBoxPatterns(fileName);
  loadBoxPhrases(fileName);
}
void DrumBoxDrive::loadBoxPatterns(QString fileName) {
  QDomDocument conf;
  QFile f(fileName);
  if (!f.open(QIODevice::ReadOnly)) {
    onDisplayMessage("Error opening '"+fileName+"' file can't be read");
    return;
  }
  QString errorStr;
  int errorLine, errorColumn;
  if (!conf.setContent(&f, true, &errorStr, &errorLine, &errorColumn)) {
    onDisplayMessage(QString("Parse error at line %1, column %2:%3")
		     .arg(errorLine).arg(errorColumn).arg(errorStr));
    f.close();
    return;
  }
  f.close();
  QDomElement root=conf.documentElement();
  if (root.tagName() != "DrumBoxPatterns") {
    onDisplayMessage("The file is not a conf file for this app.");
    return;
  }

  QByteArray sysex;
  sysex.append(42);
  sysex.append(LOAD_BOX_PATTERNS);
  
  QDomNodeList trs;
  QDomElement elt;

  trs=conf.elementsByTagName("tracks_conf");
  if (trs.length() == 0) {
    onDisplayMessage("uncomplete file, missing values");
    return;
  }
  elt=trs.item(0).toElement();
  uint track_count=elt.attribute("track_count").toInt();
  sysex.append(track_count);

  trs=conf.elementsByTagName("patterns_conf");
  if (trs.length() == 0) {
    onDisplayMessage("uncomplete file, missing values");
    return;
  }
  elt=trs.item(0).toElement();
  uint pattern_count=elt.attribute("pattern_count").toInt();
  sysex.append(pattern_count);

  trs=conf.elementsByTagName("pattern");
  if (trs.length() != pattern_count) {
    onDisplayMessage("uncomplete file, missing values");
    return;
  }
  for (uint i=0; i< trs.length(); i++) {
    QDomElement dn=trs.item(i).firstChildElement("track");
    while (!dn.isNull()) {
      uint16_t pat=dn.attribute("pattern").toInt(0, 2);
      sysex.append(pat >> 8);
      sysex.append(pat & 0xFF);
      
      dn=dn.nextSiblingElement("track");
    }
  }
  onDisplayMessage(QString("Loading Patterns %1 to DrumBox ...").arg(fileName));

  _bridge->sendSysEx(sysex);
}
void DrumBoxDrive::loadBoxPhrases(QString fileName) {
  QDomDocument conf;
  QFile f(fileName);
  if (!f.open(QIODevice::ReadOnly)) {
    onDisplayMessage("Error opening '"+fileName+"' file can't be read");
    return;
  }
  QString errorStr;
  int errorLine, errorColumn;
  if (!conf.setContent(&f, true, &errorStr, &errorLine, &errorColumn)) {
    onDisplayMessage(QString("Parse error at line %1, column %2:%3")
		     .arg(errorLine).arg(errorColumn).arg(errorStr));
    f.close();
    return;
  }
  f.close();
  QDomElement root=conf.documentElement();
  if (root.tagName() != "DrumBoxPatterns") {
    onDisplayMessage("The file is not a conf file for this app.");
    return;
  }

  QByteArray sysex;
  sysex.append(42);
  sysex.append(LOAD_BOX_PHRASES);
  
  QDomNodeList trs;
  QDomElement elt;

  trs=conf.elementsByTagName("phrases_conf");
  if (trs.length() == 0) {
    onDisplayMessage("uncomplete file, missing phrases values");
    return;
  }
  elt=trs.item(0).toElement();
  uint phrase_count=elt.attribute("phrase_count").toInt();
  sysex.append(phrase_count);

  trs=conf.elementsByTagName("phrase");
  if (trs.length() != phrase_count) {
    onDisplayMessage("uncomplete file, missing phrase values");
    return;
  }
  for (uint i=0; i< phrase_count; i++) {
    QDomNodeList dn=trs.item(i).toElement().elementsByTagName("song_element");
    sysex.append(dn.length());
    for (uint j=0; j < dn.length(); j++) {
      sysex.append(dn.item(j).toElement().attribute("type").toInt());
      sysex.append(dn.item(j).toElement().attribute("id").toInt());
      sysex.append(dn.item(j).toElement().attribute("repeatCount").toInt());
    }
  }
  onDisplayMessage(QString("Loading Phrases %1 to DrumBox ...").arg(fileName));

  _bridge->sendSysEx(sysex);
}

void DrumBoxDrive::sysExReceived(QByteArray sysex) {
  if (sysex.size() < 2)
    return;
  if (sysex.at(0) != 42) {
    qWarning()<<"sysex: no valid signature !";
    return;
  }
  QString msg;
  switch (sysex.at(1)) {
  case BOX_READY:
    msg="DrumBox is ready";
    if (!_lastConfFilename.isEmpty()) {
      msg+="\nLoading last conf '"+_lastConfFilename+"'";
      loadBoxConfig(_lastConfFilename);
    }
    if (!_lastPatternFilename.isEmpty()) {
      msg+="\nLoading last patterns '"+_lastPatternFilename+"'";
      loadBoxPatterns(_lastPatternFilename);
      loadBoxPhrases(_lastPatternFilename);
    }
    onDisplayMessage(msg);
    break;
  case SAVE_BOX_CONFIG:
    saveBoxConfig(_lastConfFilename, sysex);
    break;
  case SAVE_BOX_PATTERNS:
    saveBoxPatterns(_lastPatternFilename, sysex);
    break;
  case SAVE_BOX_PHRASES:
    saveBoxPhrases(_lastPatternFilename, sysex);
    break;
  case LOAD_BOX_CONFIG:
    switch (sysex.at(2)) {
    case 0:
      onDisplayMessage("Conf loaded by DrumBox !");
      break;
    case 1:
      onDisplayMessage("Error: DrumBox didn't get the right message length");
      break;
    case 2:
      onDisplayMessage("Warning: loaded conf as more tracks than DrumBox can handle");
      break;
    }
    break;
  case LOAD_BOX_PATTERNS:
    switch (sysex.at(2)) {
    case 0:
      onDisplayMessage("Patterns loaded by DrumBox !");
      break;
    case 1:
      onDisplayMessage("Error: DrumBox didn't get the right message length");
      break;
    case 2:
      onDisplayMessage("Warning: loaded patterns has more tracks than DrumBox can handle");
      break;
    case 3:
      onDisplayMessage("Warning: not all the patterns has been loaded due to DrumBox MAX_PATTERN limitation");
      break;
    }
    break;
  case LOAD_BOX_PHRASES:
    switch (sysex.at(2)) {
    case 0:
      onDisplayMessage("Phrases loaded by DrumBox !");
      break;
    case 1:
      onDisplayMessage("Error: DrumBox didn't get the right message length");
      break;
    case 2:
      onDisplayMessage("Warning: some pattern id into some phrase, is over DrumBox MAX_PATTERNS");
      break;
    case 3:
      onDisplayMessage("Warning: not all the phrases has been loaded due to DrumBox MAX_PHRASES limitation");
      break;
    }
    break;

  default:
    qWarning()<<"unknown sysex command";
  }
}

bool DrumBoxDrive::saveBoxConfig(QString path, QByteArray sysex) 
{
  QDomDocument conf;
  QDomProcessingInstruction instr;
  instr=conf.createProcessingInstruction("xml", "version=\"1.0\"  encoding=\"UTF-8\"");
  conf.appendChild(instr);
  QDomElement root, node;
  root=conf.createElement("DrumBoxConf");
  root.setAttribute("xmlns", "http://staxaudio.free.fr/mime-info");
  conf.appendChild(root);

  QString msg;
  uint8_t track_count;

  track_count=sysex.at(2);
  node=conf.createElement("tracks_conf");
  node.setAttribute("track_count", track_count);
  root.appendChild(node);

  if (3 + (2 * track_count) != sysex.size()) {
    onDisplayMessage("uncomplete sysex data !");
    return false;
  }

  
  msg=QString("track_count: %1\n").arg(track_count);
  for (uint i=0; i < track_count; i++) {
    node=conf.createElement("track");
    node.setAttribute("midi_channel", (uint)sysex.at(3 + (i * 2)));
    node.setAttribute("midi_note", (uint)sysex.at(4 + (i * 2)));
    root.appendChild(node);

    msg+=QString("* track %1: ch%2 note %3\n")
      .arg(i+1)
      .arg((uint)sysex.at(3 + (i * 2)))
      .arg((uint)sysex.at(4 + (i * 2)));
  }
  
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  QTextStream ts(&f);
  ts << conf.toString();
  f.close();

  msg+=QString("File writed to %1").arg(path);
  
  onDisplayMessage(msg);

  return true;
}
bool DrumBoxDrive::saveBoxPatterns(QString path, QByteArray sysex) 
{
  QDomDocument conf;
  QDomProcessingInstruction instr;
  instr=conf.createProcessingInstruction("xml", "version=\"1.0\"  encoding=\"UTF-8\"");
  conf.appendChild(instr);
  QDomElement root, node, pnode;
  root=conf.createElement("DrumBoxPatterns");
  root.setAttribute("xmlns", "http://staxaudio.free.fr/mime-info");
  conf.appendChild(root);

  QString msg;
  uint8_t track_count, pattern_count;

  track_count=sysex.at(2);
  node=conf.createElement("tracks_conf");
  node.setAttribute("track_count", track_count);
  root.appendChild(node);

  pattern_count=sysex.at(3);
  node=conf.createElement("patterns_conf");
  node.setAttribute("pattern_count", pattern_count);
  root.appendChild(node);


  if (4 + (2 * track_count * pattern_count) != sysex.size()) {
    onDisplayMessage("uncomplete sysex data !");
    return false;
  }

  msg=QString("track_count: %1, pattern_count: %2\n").arg(track_count).arg(pattern_count);
  for (int i=0; i < pattern_count; i++) {
    pnode=conf.createElement("pattern");
    pnode.setAttribute("id", i+1);
    pnode.setAttribute("track_count", track_count);
    root.appendChild(pnode);
    for (int j=0; j < track_count; j++) {
      uint16_t msb=(uint8_t)sysex.at(4 + (i * track_count * 2) + (j * 2));
      uint8_t lsb=(uint8_t)sysex.at(5 + (i * track_count * 2) + (j * 2));

      uint16_t pat=(msb << 8) + lsb;

      node=conf.createElement("track");
      node.setAttribute("pattern", QString::number(pat, 2));
      pnode.appendChild(node);

      msg+=QString("* pattern %1, track %2: %3\n")
	.arg(i+1).arg(j+1).arg(pat, 0, 2);
    }
    onDisplayMessage(msg);
    msg="";
  }

  QFile f(path);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  QTextStream ts(&f);
  ts << conf.toString();
  f.close();

  msg+=QString("File writed to %1").arg(path);
  
  onDisplayMessage(msg);

  return true;
}
bool DrumBoxDrive::saveBoxPhrases(QString path, QByteArray sysex) 
{
  QDomDocument conf;
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    onDisplayMessage("Error opening '"+path+"' file can't be read");
    return false;
  }
  QString errorStr;
  int errorLine, errorColumn;
  if (!conf.setContent(&f, true, &errorStr, &errorLine, &errorColumn)) {
    onDisplayMessage(QString("Parse error at line %1, column %2:%3")
		     .arg(errorLine).arg(errorColumn).arg(errorStr));
    f.close();
    return false;
  }
  f.close();
  
  QDomElement root, node, pnode;

  root=conf.documentElement();
  if (root.tagName() != "DrumBoxPatterns") {
    onDisplayMessage("The file is not a conf file for this app.");
    return false;
  }

  QString msg;
  uint8_t phrase_count;

  phrase_count=sysex.at(2);
  node=conf.createElement("phrases_conf");
  node.setAttribute("phrase_count", phrase_count);
  root.appendChild(node);

  int sysexIndex=3;
  for (int i=0; i < phrase_count; i++) {
    pnode=conf.createElement("phrase");
    pnode.setAttribute("id", i+1);
    root.appendChild(pnode);
    int elementCount=sysex.at(sysexIndex);
    sysexIndex++;
    msg+=QString("-Phrase %1:\n").arg(i+1);
    for (int j=0; j < elementCount; j++) {
      node=conf.createElement("song_element");
      node.setAttribute("type", QString::number(sysex.at(sysexIndex)));
      node.setAttribute("id", QString::number(sysex.at(sysexIndex +1)));
      node.setAttribute("repeatCount", QString::number(sysex.at(sysexIndex+2)));
      pnode.appendChild(node);

      msg+=QString("\t* type: %1, id: %2, repeatCount: %3\n")
	.arg((int)sysex.at(sysexIndex)).arg((int)sysex.at(sysexIndex+1)).arg((int)sysex.at(sysexIndex+2));

      sysexIndex+=3;
    }
    onDisplayMessage(msg);
    msg="";
  }
  

  if (!f.open(QIODevice::WriteOnly))
    return false;
  QTextStream ts(&f);
  ts << conf.toString();
  f.close();

  msg+=QString("File writed to %1").arg(path);
  
  onDisplayMessage(msg);

  return true;
}

void DrumBoxDrive::handleMidiEvent(struct MidiEvent ev)
{
}

bool DrumBoxDrive::saveConf() 
{
  QDomDocument conf;
  QDomProcessingInstruction instr;
  instr=conf.createProcessingInstruction("xml", "version=\"1.0\"  encoding=\"UTF-8\"");
  conf.appendChild(instr);
  QDomElement root, midi, params, mixer, mixers;
  root=conf.createElement("DrumBoxDriveConf");
  root.setAttribute("xmlns", "http://staxaudio.free.fr/mime-info");
  conf.appendChild(root);

  bool vis=isVisible();
  QRect dims=geometry();
  if (!vis) {
    dims=_geoMem;
  }
  else
    dims=geometry();
  params=conf.createElement("geometry");
  params.setAttribute("visible", vis);
  params.setAttribute("x", dims.x());
  params.setAttribute("y", dims.y());
  params.setAttribute("width", dims.width());
  params.setAttribute("height", dims.height());
  root.appendChild(params);

  if (!_presetDirectory.isEmpty()) {
    params=conf.createElement("preset_directory");
    params.setAttribute("path", _presetDirectory);
    root.appendChild(params);
  }
  if (!_lastConfFilename.isEmpty()) {
    params=conf.createElement("last_box_config");
    params.setAttribute("path", _lastConfFilename);
    root.appendChild(params);
  }
  if (!_lastPatternFilename.isEmpty()) {
    params=conf.createElement("last_box_patterns");
    params.setAttribute("path", _lastPatternFilename);
    root.appendChild(params);
  }
  
  
  params=conf.createElement("serial_port");
  params.setAttribute("name", _serialPortName.text());
  root.appendChild(params);

  root.appendChild(_jack.connectionsToXML());

  root.appendChild(_midiIn.toXML(conf));

  QFile f(_confFile);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  QTextStream ts(&f);
  ts << conf.toString();
  f.close();


  return true;
}
bool DrumBoxDrive::loadConf() 
{
  QDomDocument conf;
  QFile f(_confFile);
  if (!f.open(QIODevice::ReadOnly))
    return false;
  QString errorStr;
  int errorLine, errorColumn;
  if (!conf.setContent(&f, true, &errorStr, &errorLine, &errorColumn)) {
    qDebug()<<"Parse error at line "<<errorLine<< ", column "<<errorColumn<<":"<<errorStr;
    f.close();
    return false;
  }
  f.close();
  QDomElement root=conf.documentElement();
  if (root.tagName() != "DrumBoxDriveConf") {
    qDebug()<<"The file is not a conf file for this app.";
    return false;
  }

  QDomNodeList geo=conf.elementsByTagName("geometry");
  QDomElement elt;
  
  if (geo.length()) {
    elt=geo.item(0).toElement();
    _geoMem.setRect(elt.attribute("x").toInt(), elt.attribute("y").toInt(),
		    elt.attribute("width").toInt(), elt.attribute("height").toInt());
    setGeometry(_geoMem);
    if (elt.attribute("visible").toInt())
      show();
    else
      hide();
  }

  QDomNodeList cd=conf.elementsByTagName("preset_directory");
  if (cd.length())
    _presetDirectory=cd.item(0).toElement().attribute("path");
  cd=conf.elementsByTagName("last_box_config");
  if (cd.length())
    _lastConfFilename=cd.item(0).toElement().attribute("path");
  cd=conf.elementsByTagName("last_box_patterns");
  if (cd.length())
    _lastPatternFilename=cd.item(0).toElement().attribute("path");
  
  QDomNodeList sp=conf.elementsByTagName("serial_port");
  
  if (sp.length()) {
    elt=sp.item(0).toElement();
    _serialPortName.setText(elt.attribute("name"));
  }

  _jack.connectionsFromXML(root);
  _midiIn.fromXML(root);
  return true;
}


// 
// injector.cpp ends here
