#ifndef BRIDGE_H
#define BRIDGE_H

/*
 * Bridge is the actual controller that bridges Serial <-> MIDI. Create it by passing the
 * the serial & MIDI ports to use (or NULL if that part is not used.)
 *
 * The Bridge manages its own internal state, and can be deleted when you stop bridging and/or
 * recreated with new settings.
 */

#include <QObject>
#include <QTime>
#include <QThread>
#include <stdint.h>
#include "qextserialport.h"
#include "PortLatency.h"
#include <libqjack/jack_client.h>

const QString TEXT_NOT_CONNECTED = "(Not Connected)";
const QString TEXT_NEW_PORT = "(Create new port)";

#define NAME_MIDI_IN "MIDI->Serial"
#define NAME_MIDI_OUT "Serial->MIDI"

// SYSEX REQUESTS
#define SAVE_BOX_CONFIG		1
#define SAVE_BOX_PATTERNS	2
#define LOAD_BOX_CONFIG		3
#define LOAD_BOX_PATTERNS	4

unsigned getEncodeSysExLength(unsigned inLength);
QByteArray encodeSysEx(QByteArray msg, uint8_t action);
QByteArray encodeSysEx(QByteArray in);

unsigned getDecodeSysExLength(unsigned inLength);
QByteArray decodeSysEx(QByteArray in);


class Bridge : public QObject
{
  Q_OBJECT
public:
  explicit Bridge();
  void attach(QString serialName, JackClient *jack, QThread *workerThread);
  
  // Destroying an existing Bridge will cleanup state & release all ports
  ~Bridge();
	   
public slots:
    void queryConfigSave();
    void queryConfigLoad(QByteArray conf);
  
    void queryPatternSave();
    void queryPatternLoad(QByteArray conf);
  
signals:
  // Signals to push user status messages
  void displayMessage(QString message);
  void debugMessage(QString message);
  void errorMessage(QString message);
  void serialPortOpened();

  void sysExReceived(QByteArray data);
  
  // Signals to trigger UI "blinkenlight" updates
  void midiReceived();
  void midiSent();
  void serialTraffic();
private slots:
  void onMidiIn(double timeStamp, QByteArray message);// => todo
  void onSerialAvailable();
private:
  void sendMidiMessage();
  
  void onDataByte(uint8_t byte);
  void onStatusByte(uint8_t byte);
  
  QString applyTimeStamp(QString message);
  QString describeMIDI(QByteArray &buf);

  
  bool bufferStartsWith(uint8_t byte) { return this->msg_data.length() && (uint8_t)msg_data[0] == byte; }
  
  int running_status;       // if we have a running or current status byte, what is it?
  int data_expected;        // how many bytes of data are we currently waiting on?
  QByteArray msg_data;      // accumulated message data from the serial port
  //QRtMidiIn *midiIn;
  //RtMidiOut *midiOut;
  //int midiInPort;
  //int midiOutPort;
  
  JackClient *_jack;  
  QextSerialPort *serial;
  PortLatency *latency;
  QTime attachTime;
};

#endif // BRIDGE_H
