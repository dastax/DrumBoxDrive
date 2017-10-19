// jack_engine.cpp --- 
// 
// Filename: jack_engine.cpp
// Description: 
// Author: stax
// Maintainer: 
// Created: mer jun 17 17:22:01 2009 (+0200)
// Version: 
// Last-Updated: sam. janv.  9 14:16:27 2016 (+0100)
//           By: stax
//     Update #: 261
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

    createMidiIn("midi_in");
    addMidiOutput("midi_out");
    return true;
  }
  return false;
}

// 
// jack_engine.cpp ends here
