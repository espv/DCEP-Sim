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


#include <string.h>
#include "placement.h"
#include "ns3/Tuple.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "dcep.h"
#include "ns3/ipv4.h"
#include "ns3/string.h"
#include "ns3/object.h"
#include "communication.h"
#include "cep-engine.h"
#include "common.h"
#include "message-types.h"
#include "dcep-header.h"
#include "ns3/abort.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(Placement);
    NS_LOG_COMPONENT_DEFINE("Placement");

        /* ... */
    TypeId
    Placement::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::Placement")
                .SetParent<Object> ()
                
                ;
        return tid;
    }

    void
    Placement::configure() {

    }


    void
    Placement::SendCepEventToCepEngineNew (Ptr<TupleWrapper> tupleWrapper)
    {
        GetObject<CEPEngine>()->ProcessCepEvent(tupleWrapper);
    }


    void
    Placement::RcvCepEventNew(Ptr<TupleWrapper> tupleWrapper)
    {
      SendCepEventToCepEngineNew(tupleWrapper);
    }
}
