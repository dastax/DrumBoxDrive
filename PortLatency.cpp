/*
 * Linux implementation of fixing latency on a serial port.
 *
 * qextserialport doesn't provide an easy way to get the USB details from the serial port, to find FTDIs.
 *
 * However, Linux has a much better serial flag called ASYNC_LOW_LATENCY that will do the job, anyhow.
 * ... and maybe work on other serial port types!
 */
#include "PortLatency.h"

extern "C" {
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
}

PortLatency::PortLatency(QString portName)
    : _portName(portName),
    _isSwitched(false)
{

}

bool PortLatency::fixLatency() {
  if(swapAsync(true)) {
    emit debugMessage(tr("Switched serial port %1 to low-latency mode").arg(_portName));
    _isSwitched=true;
    return true;
  }
  return false;
}

bool PortLatency::resetLatency() {
  if(swapAsync(false)) {
    emit debugMessage(tr("Set serial port %1 back to normal mode").arg(_portName));
    _isSwitched=false;
    return true;
  }
  return false;
}

bool PortLatency::swapAsync(bool setAsyncMode) {
  if(_isSwitched == setAsyncMode) {
    return true;
  }
  if(!_portName.startsWith("/dev/")) {
    _portName.prepend("/dev/");
  }
  int fd=open(_portName.toAscii().data(), O_NONBLOCK);
  if(fd <= 0) {
    emit errorMessage(tr("Failed to open serial port device %1 to lower latency").arg(_portName));
    return false;
  }
  struct serial_struct ser_info;
  int r=ioctl(fd, TIOCGSERIAL, &ser_info);
  if(r) {
    emit errorMessage(tr("Failed to get serial port info for %1, cannot set low latency mode").arg(_portName));
    close(fd);
    return false;
  }
  if((ser_info.flags & ASYNC_LOW_LATENCY) == setAsyncMode) {
    emit debugMessage(tr("Serial port %1 already%2 in low latency mode")
		      .arg(_portName).arg(setAsyncMode ? "" : " not"));
    close(fd);
    return false;
  }
  if(setAsyncMode) {
    ser_info.flags |= ASYNC_LOW_LATENCY;
  } else {
    ser_info.flags &= ~ASYNC_LOW_LATENCY;
  }
  r=ioctl(fd, TIOCSSERIAL, &ser_info);
  if(r) {
    emit errorMessage(tr("Failed to set low-latency mode on %1").arg(_portName));
    close(fd);
    return false;
  }

  // special part to disable arduino reset
  termios mode;
  ::memset(&mode,0,sizeof( mode));
  tcgetattr(fd,& mode);
  mode.c_cflag &= ~HUPCL;   // disable hang-up-on-close to avoid reset
  tcsetattr(fd,TCSANOW,&mode);
    
  close(fd);
  return true;
}
