/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

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


#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <deque>
#include <ns3/queue.h>
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/event-id.h"
#include "ns3/Operator.h"
#include "ns3/Tuple.h"
#include "ns3/ptr.h"
#include "ns3/dcep-header.h"
#include "ns3/data-rate.h"

namespace ns3 {

/* ... */
    class CepEvent;
    class Packet;
    class Query;
    class Operator;
    class TupleWrapper;
    class MigrationPlan;


    class SendStateCallback {
    public:
        int old_host;
        int new_host;
        std::size_t size;
        std::size_t left;
        DcepHeader dcepHeader;
        int type;

        SendStateCallback(int old_host, int new_host, std::size_t size, std::size_t prev_state, DcepHeader dcepHeader, int type) {
            this->old_host = old_host;
            this->new_host = new_host;
            this->size = size;
            this->dcepHeader = dcepHeader;
            this->left = prev_state;
            this->type = type;
        }
    };

class DcepPackage : public Object {
public:
    Ptr<Socket> socket;
    uint8_t *payload;
    std::size_t size;
    std::size_t copied_bytes;
    DcepHeader dcep_header;
    bool node_failed = false;
    bool active = false;
    bool loaded_header = false;
    Ptr<Packet> cur_buffered_packet;

    static TypeId GetTypeId (void);

    DcepPackage(const Ptr<Socket>& socket);

    void reset();
    void init(Ptr<Packet>);
    bool readHeader(int header_size);
    void setSize(int payload_size);
};

//template<typename Item> class DropTailQueue2;
class Communication : public Object
    {
    public:
        Communication();
        ~Communication();
        
        static TypeId GetTypeId (void);
        /*create udp server and client applications to use for communication
         Might need to customize them a bit.*/
        void Configure (void);
        
        void setNode(Ptr<Node> node);
        void setPort(uint16_t);
        
      /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
        void HandleReadState (Ptr<Socket> socket);
        void HandleReadTuples (Ptr<Socket> socket);
        void HandleReadFetch (Ptr<Socket> socket);
        void AfterSending ();

        DataRate GetDataRate(int to_node);
        Ptr<NetDevice> GetNetDevice();
        void HandleReadFakeTransmit(int node_id, Ptr<Packet> packet);
        void ConnectionSucceeded (Ptr<Socket> socket);
        void ConnectionFailed (Ptr<Socket> socket);
        void HandleAccept (Ptr<Socket> s, const Address& from);
        void HandleAcceptState (Ptr<Socket> s, const Address& from);
        void HandlePeerError (Ptr<Socket> s);
        void HandlePeerClose (Ptr<Socket> s);
        void doSendRequest(int old_host, int new_host, std::size_t size, DcepHeader &dcepHeader, int type);
        void SendState(int old_host, int new_host, std::size_t size, DcepHeader &dcepHeader);
        void SendFetchRequest(int old_host, int new_host, std::size_t size, DcepHeader &dcepHeader);
        void SetupConnections();
        void NotSendingAnymore();
        void SendNextState();
        void ShutDown();
        void SocketClosedNormal (Ptr<Socket> socket);
        void SocketClosedError (Ptr<Socket> socket);
       // void ScheduleSend(Ipv4Address peerAddress, const uint8_t *, uint32_t size, uint16_t msg_type);
        void ScheduleSend(Ptr<Packet> p, InetSocketAddress addr);
        Ipv4Address GetLocalAddress();
        Ipv4Address GetSinkAddress();
        void HandleStateSent (Ptr<Socket> s, uint32_t state_sent);
        void HandleTupleSent (Ptr<Socket> s, uint32_t state_sent);
        void HandleFetchSent (Ptr<Socket> s, uint32_t state_sent);
        void TxBufferAvailable(Ptr<Socket> s, uint32_t bytes_available);
        inline static std::map<int, int> ipToNodeId;
        inline static std::map<int, int> ipToTuplesNodeId;
        inline static std::map<int, int> ipToMigrationNodeId;
        inline static std::map<int, int> ipToFetchNodeId;
        inline static std::map<int, bool> nodeSetup;

	    std::size_t nodeIdToIpAndPortIterator = 0;
        static inline int total_sent = 0;
        bool currently_migrating = false;

        // These three attributes are used for the overall understanding of which node is migrating to which node
        bool in_migration = false;
        int migration_node = -1;
        int migrating_query_id = -1;


        std::deque<SendStateCallback> sendStateQueue;

        void send(void);

    private:
        int node_id;
        std::deque<Ptr<Packet>> m_stateSendQueue;
        std::deque<Ptr<Packet>> m_tupleSendQueue;
        std::deque<Ptr<Socket>> m_tupleSocketQueue;
        std::deque<Ptr<Socket>> m_stateSocketQueue;
        EventId m_packetSendCepEvent;
        uint32_t backoffTime;
        uint16_t m_port;
        uint32_t numRetransmissions;
        Ipv4Address m_sinkAddress;
        Ipv4Address host_address;
        std::map<int, Ptr<Socket>> nodeIdToFetchSocket;
        std::map<int, Ptr<Socket>> nodeIdToStateSocket;
        std::map<int, Ptr<Socket>> nodeIdToTupleSocket;
        std::map<Ptr<Socket>, int> socketToNodeIdFetch;
        std::map<Ptr<Socket>, int> socketToNodeIdState;
        std::map<Ptr<Socket>, int> socketToNodeIdTuples;
        std::map<Ptr<Socket>, Ptr<DcepPackage>> socketToDcepPackage;
        std::map<int, Ptr<DcepPackage>> nodeIdToDcepPackageState;
        std::map<int, Ptr<DcepPackage>> nodeIdToDcepPackageTuples;
        std::map<int, Ptr<DcepPackage>> nodeIdToDcepPackageFetch;
        Ptr<Node> disnode;
        uint32_t m_sent;
        bool m_connected = false;
        bool sending = false;
        bool running = true;
        std::size_t bytes_left_of_package = 0;
        std::size_t current_package_size = 0;
        std::size_t current_full_package_size = 0;
};
   
    
}

#endif /* COMMUNICATION_H */

