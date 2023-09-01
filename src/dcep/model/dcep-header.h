/*
 * Copyright (C) 2018, Fabrice S. Bigirimana
 * Copyright (c) 2018, University of Oslo
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#ifndef DCEPHEADER_H
#define DCEPHEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>
#include <map>

namespace ns3 {

/* A sample Header implementation
 */
class DcepHeader : public Header 
{
public:

  DcepHeader ();
  virtual ~DcepHeader ();

  void SetContentType (uint16_t data);
  uint16_t GetContentType (void) const;
  std::size_t GetContentSize(void) const;
  void setContentSize(std::size_t s);
  void setHeaderSize(std::size_t s);
  std::size_t GetHeaderSize();
  void SetFromNode(int from_node) {this->from_node = from_node;}
  uint32_t GetFromNode() {return from_node;}

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t Deserialize (Buffer::Iterator start, Buffer::Iterator end);
  virtual uint32_t GetSerializedSize (void) const;
  uint32_t GetSerializedNumberBytes (void) const;
  //uint32_t DeserializeRest (Buffer::Iterator start);

  std::map<std::string, uint16_t> arguments;

  uint16_t magic_num = 0xDCEB;
  uint16_t m_type;
  std::size_t size = 0;
  uint16_t header_size = 0;
  uint16_t from_node;
  // The below variables are never sent, but are used to find out if the entire header has been received
  bool partially_read = false;
  Ptr<Packet> packet_buffer;
};

}

#endif /* DCEPHEADER_H */

