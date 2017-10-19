/* jack_engine.h --- 
 * 
 * Filename: jack_engine.h
 * Description: 
 * Author: stax
 * Maintainer: 
 * Created: mer jun 17 17:22:36 2009 (+0200)
 * Version: 
 * Last-Updated: sam. janv.  9 14:47:49 2016 (+0100)
 *           By: stax
 *     Update #: 157
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

#ifndef JACK_ENGINE_H
#define JACK_ENGINE_H

#include <QtGui>
#include <libqjack/libqjack.h>

class JackEngine : public JackClient
{
Q_OBJECT

private:

  
public:
  JackEngine();
  ~JackEngine();

  bool initEngine(QString name);

};



#endif
/* jack_engine.h ends here */
