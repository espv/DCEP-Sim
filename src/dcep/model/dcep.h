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

#ifndef DCEP_H
#define DCEP_H

#include <stdint.h>
#include <ns3/cep-engine.h>
#include <ns3/communication.h>
#include "ns3/type-id.h"
#include "ns3/nstime.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/event-id.h"
#include "ns3/application.h"
#include "ns3/traced-callback.h"

namespace ns3 {

    class Query;
    class CepEvent;
    class DataSource;
    class Sink;
    class Communication;
    class Placement;
    class DcepPackage;
/* ... */
class Dcep : public Application
    {
public:
    Dcep();
    static TypeId GetTypeId (void);

    uint16_t getCepEventCode();
    void SendPacket (Ptr<Packet> p, InetSocketAddress addr);
    void rcvRemoteMsg(Ptr<DcepPackage> dcep_package);
    static Ptr<Dcep> GetDcepFromNode(Ptr<Node> node);
    void HandleMigration(Ptr<DcepPackage> dcep_package);
    void HandleInterruptedMigration();
    void HandleFetchRequest(Ptr<DcepPackage> dcep_package);
    void HandleStandardMovingMigration(Ptr<DcepPackage> dcep_package);
    void HandleWindowRecreationMigration(Ptr<DcepPackage> dcep_package);
    void HandleStateRecreationMigration(Ptr<DcepPackage> dcep_package);
    void HandleStatelessMigration(Ptr<DcepPackage> dcep_package);

    TracedCallback<> ClearQueries;
    Ptr<Communication> c_communication;
protected:
    
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    bool datasource_node;
    bool sink_node;
    bool distributed_execution;
    Ipv4Address m_sinkAddress;
    uint16_t m_cepPort;
    uint16_t event_code;
    uint32_t events_load;
    uint32_t query_load;
    uint32_t event_interval;
    uint16_t operators_load;
    std::string placementPolicy;
    std::string routing_protocol;
    std::string trace_fn;
    std::string experiment_metadata_fn;
};

}

#endif /* DCEP_H */

