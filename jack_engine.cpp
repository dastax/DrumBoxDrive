// jack_engine.cpp --- 
// 
// Filename: jack_engine.cpp
// Description: 
// Author: stax
// Maintainer: 
// Created: mer jun 17 17:22:01 2009 (+0200)
// Version: 
// Last-Updated: sam. janv.  9 21:53:37 2016 (+0100)
//           By: stax
//     Update #: 281
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
#include <math.h>
#include "jack_engine.h"

JackEngine::JackEngine() : JackClient()
{
}
JackEngine::~JackEngine()
{
}
bool JackEngine::initEngine(QString name)
{
  if (initClient(name)) {
    addMidiOutput("midi_out");
    return true;
  }
  return false;
}
void JackEngine::midi_out_process(jack_nframes_t nframes)
{
  portmap_it_t i=_midiOutputs.constBegin();
  while (i != _midiOutputs.constEnd()) {
    struct MidiEvent ev;
    int read, t;
    unsigned char* buffer ;
    jack_nframes_t last_frame_time=jack_last_frame_time(_client);
    void *port_buf=jack_port_get_buffer(i.value(), nframes);
    jack_ringbuffer_t *rb=_ringBuffers[i.key()];

    jack_midi_clear_buffer(port_buf);
    while (jack_ringbuffer_read_space(rb)) {
      read=jack_ringbuffer_peek(rb, (char *)&ev, sizeof(ev));
      if (read != sizeof(ev)) {
	qWarning()<<" Short read from the ringbuffer, possible note loss.";
	jack_ringbuffer_read_advance(rb, read);
	continue;
      }
      else {
	t=ev.time + nframes - last_frame_time;
	if (t >= (int)nframes)
	  break;
	if (t < 0)
	  t=0;
	jack_ringbuffer_read_advance(rb, sizeof(ev));
	buffer=jack_midi_event_reserve(port_buf, t, ev.size);
	if (buffer != NULL)
	  memcpy(buffer, ev.data, ev.size);
      }
    }
    i++;
  }
}

// 
// jack_engine.cpp ends here
