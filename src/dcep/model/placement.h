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

#ifndef PLACEMENT_H
#define PLACEMENT_H

#include <stdint.h>
#include <ns3/cep-engine.h>
#include "ns3/object-factory.h"
#include "ns3/object.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/traced-callback.h"
#include "ns3/Tuple.h"

namespace ns3 {

/* ... */
    class Ipv4Address;
    class Detector;
    class Forwarder;
    class Dcep;
    class TupleWrapper;

    /**
     * The placement component can be seen as an extension for 
     * a CEP engine which enables distribution.
     * This means that the primary goal of this component is to hide distribution
     * to the CEP engine. To achieve this, the placement component takes care of:
     *      1. operator placement assignment on network nodes, essentially building an
     *          operator network.
     *      2. event forwarding through the operator network
     * The design of the placement component applies the separation of 
     * policies and mechanisms. The aim is to allow different placement assignment policies 
     * which can be loaded at compile or runtime. This should make this component reusable across
     * different system scenarios. 
     * A placement object implements mechanisms for building the operator network and forwarding events
     * through it. And instance of a PlacementPolicy (defined above) object implements a placement 
     * assignment algorithm to determine where an operator should be placed. 
     */
    
class Placement : public Object
    {
    public:
        static TypeId GetTypeId (void);
        
        void configure();
        void SendCepEventToCepEngineNew (Ptr<TupleWrapper> tuple);
        void RcvCepEventNew(Ptr<TupleWrapper> tupleWrapper);

    private:
        
        friend class CentralizedPlacementPolicy;
        friend class Detector;
        friend class Forwarder;
        friend class Dcep;
        friend class ResourceManager;

        uint16_t deploymentModel;
        bool centralized_mode;
        uint16_t operator_counter;
    };
    
}

#endif /* PLACEMENT_H */

