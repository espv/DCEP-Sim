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


#include <ns3/simulator.h>
#include "dcep-header.h"
#include "message-types.h"

namespace ns3 {

    DcepHeader::DcepHeader ():
    m_type (0)
    {
      // we must provide a public default constructor, 
      // implicit or explicit, but never private.
        //qt = "";
    }
    DcepHeader::~DcepHeader ()
    {
        
    }

    TypeId
    DcepHeader::GetTypeId (void)
    {
      static TypeId tid = TypeId ("ns3::DcepHeader")
        .SetParent<Header> ()
        .AddConstructor<DcepHeader> ()
      ;
      return tid;
    }
    TypeId
    DcepHeader::GetInstanceTypeId (void) const
    {
      return GetTypeId ();
    }

    void
    DcepHeader::Print (std::ostream &os) const
    {
      // This method is invoked by the packet printing
      // routines to print the content of my header.
      //os << "data=" << m_data << std::endl;
      os << "content type = " << m_type;
    }

    uint32_t
    DcepHeader::GetSerializedNumberBytes() const {
        return sizeof(uint32_t) * 1 + sizeof(uint16_t) * 5;
    }

    uint32_t
    DcepHeader::GetSerializedSize (void) const
    {
        uint32_t hs = GetSerializedNumberBytes();
        for (auto pair : arguments) {
            auto str = pair.first;
            auto str_length = str.length();
            hs += sizeof(uint16_t) * 2 + str_length;
        }
        return hs;
        //return sizeof(DcepHeader);
    }
    void
    DcepHeader::Serialize (Buffer::Iterator start) const {
      // we can serialize two bytes at the start of the buffer.
      // we write them in network byte order.
      //this->header_size = GetSerializedSize();
      start.WriteHtonU16(magic_num);
      start.WriteHtonU16(GetSerializedSize());
      start.WriteHtonU32(size);
      start.WriteHtonU16(m_type);
      start.WriteHtonU16(from_node);
      auto number_arguments = arguments.size();
      start.WriteHtonU16(number_arguments);
      //std::cout << "Serializing DcepHeader with " << number_arguments << " arguments" << std::endl;
      for (auto pair : arguments) {
          auto str = pair.first;
          auto str_length = str.length();
          start.WriteHtonU16(str_length);
          start.Write((uint8_t*) pair.first.c_str(), str_length);
          start.WriteHtonU16(pair.second);
      }
    }

    uint32_t
    DcepHeader::Deserialize (Buffer::Iterator start)
    {
      // we can deserialize two bytes from the start of the buffer.
      // we read them in network byte order and store them
      // in host byte order.
      // We assume that the packet at least has 8 bytes to read the header size
      // This means that we can have two cases of reading the header:
      // 1: we read the header fully and everything is good
      // 2: we fail to read the entire header and have to restart next packet
      // We assume that the entire header will be received in the second go
      magic_num = start.ReadNtohU16();
      header_size = start.ReadNtohU16();
      size = start.ReadNtohU32();
      m_type = start.ReadNtohU16();
      from_node = start.ReadNtohU16();
      uint16_t number_arguments = start.ReadNtohU16();
      for (int i = 0; i < number_arguments; i++) {
          auto str_size = start.ReadNtohU16();
          char str[str_size+1];
          str[str_size] = '\0';
          start.Read((uint8_t*) str, str_size);
          auto value = start.ReadNtohU16();
          this->arguments[str] = value;
      }
      return header_size;
    }

    uint32_t
    DcepHeader::Deserialize (Buffer::Iterator start, Buffer::Iterator end)
    {
        return Deserialize(start);
    }
    
    void 
    DcepHeader::SetContentType (uint16_t data)
    {
      m_type = data;
    }
    uint16_t
    DcepHeader::GetContentType (void) const
    {
      return m_type;
    }
    
    void
    DcepHeader::setContentSize(std::size_t s)
    {
        size = s; //qt.size();
    }

    void
    DcepHeader::setHeaderSize(std::size_t s) {
        this->header_size = s;
    }

    std::size_t
    DcepHeader::GetHeaderSize() {
        return header_size;
    }
    
    std::size_t
    DcepHeader::GetContentSize() const
    {
        return size;
    }
    
    
}
