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

#include <deque>
#include <algorithm>
#include <ns3/tcp-header.h>
#include <ns3/config.h>
#include "ns3/tcp-socket-base.h"
#include "communication.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/type-id.h"
#include "dcep.h"
#include "dcep-header.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/log.h"
#include "cep-engine.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "common.h"
#include "message-types.h"
#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/error-model.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(Communication);
NS_LOG_COMPONENT_DEFINE("Communication");


/* ... */
    TypeId
    Communication::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Communication")
        .SetParent<Object> ()
        .AddConstructor<Communication> ()
        .AddAttribute ("SinkAddress",
                       "The destination Address of the outbound packets",
                       Ipv4AddressValue (),
                       MakeIpv4AddressAccessor (&Communication::m_sinkAddress),
                       MakeIpv4AddressChecker ())
        
        ;
        
        return tid;
    }

    TypeId
    DcepPackage::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::DcepPackage")
                .SetParent<Object> ()
        ;

        return tid;
    }
 
    Communication::Communication()
   // : m_lossCounter (0)
    {
        numRetransmissions = 0;
        m_sent=0;
    }
    
    
    Communication::~Communication()
    {
    }
    
    void
    Communication::setNode(Ptr<Node> node)
    {
        this->disnode = node;
    }
    
    void
    Communication::setPort(uint16_t cepPort)
    {
        this->m_port = cepPort;
    }
    
    void
    Communication::Configure()
    {
        Ptr<Dcep> dcep = Dcep::GetDcepFromNode(disnode);
        node_id = std::stoi(StreamQuery::ns3NodeIdToExposeNodeId[dcep->GetNode()->GetId()]);
        Config::SetDefault ("ns3::TcpSocket::TcpNoDelay", BooleanValue (true));

        nodeSetup[node_id] = true;
    }

    DcepPackage::DcepPackage(const Ptr<Socket>& socket) {
        this->socket = socket;
        this->size = 0;
        this->copied_bytes = 0;
    }

    void DcepPackage::reset() {
        this->copied_bytes = 0;
        if (this->size > 0) {
            delete[] this->payload;
        }
        this->size = 0;
        this->active = false;
        this->cur_buffered_packet = nullptr;
        this->loaded_header = false;
        this->dcep_header = DcepHeader();
    }

    void DcepPackage::init(Ptr<Packet> p) {
        reset();
        this->cur_buffered_packet = p;
        this->active = true;
    }

    bool DcepPackage::readHeader(int header_size) {
        if ((int) cur_buffered_packet->GetSize() < header_size) {
            return false;
        }

        loaded_header = true;
        cur_buffered_packet->RemoveHeader(this->dcep_header);
        setSize(this->dcep_header.GetContentSize());
        return true;
    }

    void DcepPackage::setSize(int payload_size) {
        if (this->size > 0) {
            delete[] this->payload;
        }
        this->size = payload_size;
        NS_ASSERT(payload_size != 0);
        this->payload = new uint8_t[size];
    }

    Ptr<NetDevice> Communication::GetNetDevice() {
        return disnode->GetDevice(0);
    }

    void
    Communication::SetupConnections() {
        if (nodeIdToIpAndPortIterator >= ExposeCoordinator::nodeIdToTuplesIpAndPort.size() &&
            nodeIdToIpAndPortIterator >= ExposeCoordinator::nodeIdToMigrationIpAndPort.size() &&
            nodeIdToIpAndPortIterator >= ExposeCoordinator::nodeIdToFetchIpAndPort.size()) {
            return;
        }
        auto it1 = ExposeCoordinator::nodeIdToTuplesIpAndPort.begin();
        std::advance(it1, nodeIdToIpAndPortIterator);
        auto nid = it1->first;
        auto ipAndPortTuples = it1->second;

        auto it2 = ExposeCoordinator::nodeIdToMigrationIpAndPort.begin();

        std::advance(it2, nodeIdToIpAndPortIterator);
        //auto nid2 = it2->first;
        auto ipAndPortMigration = it2->second;

        auto it3 = ExposeCoordinator::nodeIdToFetchIpAndPort.begin();
        std::advance(it3, nodeIdToIpAndPortIterator);
        auto ipAndPortFetch = it3->second;

        //for (auto &[nid, ipAndPort] : ExposeCoordinator::nodeIdToIpAndPort) {
        ++nodeIdToIpAndPortIterator;
        if (this->node_id == nid || !nodeSetup[nid] || nodeIdToStateSocket[nid] || nodeIdToFetchSocket[nid] || nodeIdToTupleSocket[nid]) {
            NS_LOG_INFO("Node " << this->node_id << " skips Node " << nid);
            Simulator::Schedule(MilliSeconds(200), &Communication::SetupConnections, this);
            return;
        }
        TypeId tid_tcp = TypeId::LookupByName ("ns3::TcpSocketFactory");  // If using TCP
        TypeId tid_udp = TypeId::LookupByName ("ns3::UdpSocketFactory");  // If using UDP
        Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (1000000000));
        Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (1000000000));
        //Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
        //Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue (1000000000));
        //Config::SetDefault("ns3::TcpSocket::TcpNoDelay", BooleanValue (true));
        //Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue (0));
        //Config::SetDefault("ns3::TcpSocket::DataRetries", UintegerValue (100));

        nodeIdToStateSocket[nid] = Socket::CreateSocket (disnode, tid_tcp);
        nodeIdToFetchSocket[nid] = Socket::CreateSocket (disnode, tid_tcp);
        auto sock = nodeIdToStateSocket[nid];
        nodeIdToTupleSocket[nid] = Socket::CreateSocket (disnode, tid_tcp);
        socketToNodeIdState[nodeIdToStateSocket[nid]] = nid;
        socketToNodeIdTuples[nodeIdToTupleSocket[nid]] = nid;
        socketToNodeIdFetch[nodeIdToFetchSocket[nid]] = nid;

        nodeIdToTupleSocket[nid]->Listen();
        nodeIdToTupleSocket[nid]->BindToNetDevice(GetNetDevice());
        nodeIdToStateSocket[nid]->Listen();
        nodeIdToStateSocket[nid]->BindToNetDevice(GetNetDevice());
        nodeIdToFetchSocket[nid]->Listen();
        nodeIdToFetchSocket[nid]->BindToNetDevice(GetNetDevice());
        InetSocketAddress local_tcp = InetSocketAddress (Ipv4Address::GetAny (), m_port);
        InetSocketAddress local_udp = InetSocketAddress (Ipv4Address::GetAny (), m_port+1);
        InetSocketAddress local_fetch = InetSocketAddress (Ipv4Address::GetAny (), m_port+2);
        nodeIdToStateSocket[nid]->Bind (local_tcp);
        nodeIdToFetchSocket[nid]->Bind (local_fetch);
        nodeIdToTupleSocket[nid]->Bind (local_udp);
        nodeIdToStateSocket[nid]->SetCloseCallbacks(MakeCallback (&Communication::SocketClosedNormal, this), MakeCallback (&Communication::SocketClosedError, this));
        nodeIdToFetchSocket[nid]->SetCloseCallbacks(MakeCallback (&Communication::SocketClosedNormal, this), MakeCallback (&Communication::SocketClosedError, this));
        nodeIdToTupleSocket[nid]->SetCloseCallbacks(MakeCallback (&Communication::SocketClosedNormal, this), MakeCallback (&Communication::SocketClosedError, this));
        nodeIdToStateSocket[nid]->SetAllowBroadcast (true);
        nodeIdToFetchSocket[nid]->SetAllowBroadcast (true);
        nodeIdToTupleSocket[nid]->SetAllowBroadcast (true);
        nodeIdToStateSocket[nid]->SetConnectCallback (
                MakeCallback (&Communication::ConnectionSucceeded, this),
                MakeCallback (&Communication::ConnectionFailed, this));
        nodeIdToTupleSocket[nid]->SetConnectCallback (
                MakeCallback (&Communication::ConnectionSucceeded, this),
                MakeCallback (&Communication::ConnectionFailed, this));
        nodeIdToFetchSocket[nid]->SetConnectCallback (
                MakeCallback (&Communication::ConnectionSucceeded, this),
                MakeCallback (&Communication::ConnectionFailed, this));

        auto address_tcp = InetSocketAddress(ipAndPortMigration.first, ipAndPortMigration.second);
        auto address_udp = InetSocketAddress(ipAndPortTuples.first, ipAndPortTuples.second+1);
        auto address_fetch = InetSocketAddress(ipAndPortFetch.first, ipAndPortFetch.second+2);
        std::cout << "Address for doing migration with node " << nid << " is " << address_tcp.GetIpv4() << std::endl;
        std::cout << "Address for doing fetching with node " << nid << " is " << address_fetch.GetIpv4() << std::endl;
        std::cout << "Address for doing query processing with node " << nid << " is " << address_udp.GetIpv4() << std::endl;
        std::cout << this->nodeIdToFetchSocket[nid]->Connect (address_fetch) << std::endl;
        std::cout << this->nodeIdToStateSocket[nid]->Connect (address_tcp) << std::endl;
        std::cout << this->nodeIdToTupleSocket[nid]->Connect (address_udp) << std::endl;
        ipToMigrationNodeId[address_tcp.GetIpv4().Get()] = nid;
        ipToTuplesNodeId[address_udp.GetIpv4().Get()] = nid;
        ipToFetchNodeId[address_fetch.GetIpv4().Get()] = nid;
        nodeIdToStateSocket[nid]->SetRecvCallback (MakeCallback (&Communication::HandleReadState, this));
        nodeIdToTupleSocket[nid]->SetRecvCallback (MakeCallback (&Communication::HandleReadTuples, this));
        nodeIdToFetchSocket[nid]->SetRecvCallback (MakeCallback (&Communication::HandleReadFetch, this));
        nodeIdToStateSocket[nid]->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&Communication::HandleAccept, this));
        nodeIdToFetchSocket[nid]->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&Communication::HandleAccept, this));
        nodeIdToTupleSocket[nid]->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&Communication::HandleAccept, this));
        //nodeIdToStateSocket[nid]->SetCloseCallbacks (
        //        MakeCallback (&Communication::HandlePeerClose, this),
        //        MakeCallback (&Communication::HandlePeerError, this));
        nodeIdToStateSocket[nid]->SetSendCallback(MakeCallback (&Communication::TxBufferAvailable, this));
        nodeIdToFetchSocket[nid]->SetSendCallback(MakeCallback (&Communication::TxBufferAvailable, this));
        nodeIdToTupleSocket[nid]->SetSendCallback(MakeCallback (&Communication::TxBufferAvailable, this));
        nodeIdToStateSocket[nid]->SetDataSentCallback(MakeCallback (&Communication::HandleStateSent, this));
        nodeIdToFetchSocket[nid]->SetDataSentCallback(MakeCallback (&Communication::HandleFetchSent, this));
        nodeIdToTupleSocket[nid]->SetDataSentCallback(MakeCallback (&Communication::HandleTupleSent, this));
        //uint8_t state[1400];
        //nodeIdToTupleSocket[nid]->Send(Create<Packet>(state, 1400));
        NS_LOG_INFO(Simulator::Now() << " Node " << this->node_id << " connecting to Node " << nid);
        Simulator::Schedule(MilliSeconds(200), &Communication::SetupConnections, this);
    }

    void
    Communication::HandleReadFakeTransmit(int nid, Ptr<Packet> packet)
    {
        auto socket = this->nodeIdToStateSocket[nid];
        auto dcep = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[this->node_id]);
        if (packet->GetSize())
        {
            //SeqTsHeader seqTs;

//            Time time_now = Simulator::Now();
            if (socketToDcepPackage[socket]) {
                auto dcep_package = socketToDcepPackage[socket];
                auto packet_size = packet->GetSize();
                packet->CopyData(&dcep_package->payload[dcep_package->copied_bytes], packet_size);
                dcep_package->copied_bytes += packet_size;
                if (dcep_package->node_failed) {
                    NS_LOG_WARN(Simulator::Now() << " Failed package for migration");
                }
                //NS_LOG_INFO(Simulator::Now () << ": Received packet with size " << packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                if (dcep_package->copied_bytes >= dcep_package->size) {
                    if (dcep_package->copied_bytes > dcep_package->size) {
                        NS_LOG_INFO("Error: Tried to copy " << (dcep_package->copied_bytes-dcep_package->size) << " too many bytes from dcep package; maybe it was copied wrongly?");
                        exit(187);
                    }
                    if (dcep_package->node_failed) {
                        NS_LOG_WARN(Simulator::Now() << " Received rest of migration package, but not the full package");
                        return;
                    }
                    dcep->rcvRemoteMsg(dcep_package);
                    //delete dcep_package->payload;
                    socketToDcepPackage[socket] = nullptr;
                }
            } else {
                DcepHeader dcepHeader;
                packet->RemoveHeader(dcepHeader);
                std::size_t payload_size = dcepHeader.GetContentSize();
                std::size_t packet_size = packet->GetSize();
                if (payload_size <= packet_size) {
                    // DCEP payload cannot be smaller than packet size
                    NS_LOG_INFO(Simulator::Now() << " Received packet with size " << packet_size << " byte without header and " << payload_size << " bytes of payload in the header");
                    if (payload_size == packet_size) {
                        // Assume something went wrong with the node
                        // This now happens when the node disconnects and the package has already been dealt with when closing the socket
                        NS_ASSERT(payload_size == packet_size);
                        auto dcep_package = CreateObject<DcepPackage>(socket);
                        //dcep_package->setSize(payload_size);
                        dcep_package->dcep_header = dcepHeader;
                        packet->CopyData(dcep_package->payload, dcepHeader.GetContentSize());
                        dcep->rcvRemoteMsg(dcep_package);
                        dcep_package->reset();
                    }
                } else {
                    // Create new DcepPackage
                    auto new_dcep_package = CreateObject<DcepPackage>(socket);
                    //new_dcep_package->setSize(payload_size);
                    new_dcep_package->dcep_header = dcepHeader;
                    socketToDcepPackage[socket] = new_dcep_package;
                    new_dcep_package->copied_bytes += packet_size;
                    //NS_LOG_INFO(Simulator::Now () << ": Received new packet with size " << packet->GetSize() << " from " << from << ", received " << new_dcep_package->copied_bytes << "/" << new_dcep_package->size << " bytes");
                    packet->CopyData(new_dcep_package->payload, packet_size);
                }
            }
        }
    }

    DataRate Communication::GetDataRate(int to_node) {
        DataRateValue av;
        GetNetDevice()->GetAttribute("DataRate", av);
        return av.Get();
    }

    void
    Communication::HandleReadTuples(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        Ptr<Packet> packet, pcopy;
        Ptr<Dcep> dcep = Dcep::GetDcepFromNode(disnode);

        Address from;
        if (!running) {
            NS_LOG_INFO("Dropping packet because not running anymore");
            // TODO: Should we delete a possible socketToDcepPacket here?
        }

        auto nid = socketToNodeIdTuples[socket];
        if (nodeIdToDcepPackageTuples[nid] == nullptr) {
            nodeIdToDcepPackageTuples[nid] = CreateObject<DcepPackage>(socket);
            nodeIdToDcepPackageTuples[nid]->reset();
        }
        auto &dcep_package = nodeIdToDcepPackageTuples[nid];

        while ((packet = socket->RecvFrom (from)))
        {
            Ptr<Packet> original_packet = packet;
            (void)original_packet;
            // Each packet can contain multiple merged Dcep packages
            // This is the case where the packet contains the continuation of an existing Dcep package
            if (dcep_package->active) {
                // This means that we don't look for a DcepHeader
                dcep_package->cur_buffered_packet->AddAtEnd(packet);
                packet = dcep_package->cur_buffered_packet;
                if (!dcep_package->loaded_header) {
                    if (!dcep_package->readHeader(14)) {
                        continue;
                    }
                }
                auto packet_size = packet->GetSize();
                int copied_bytes = packet_size;
                //auto &dcepHeader = dcep_package->dcep_header;
                if (dcep_package->copied_bytes + copied_bytes > dcep_package->size) {
                    copied_bytes -= dcep_package->copied_bytes + packet_size - dcep_package->size;
                }
                packet->CopyData(&dcep_package->payload[dcep_package->copied_bytes], copied_bytes);
                dcep_package->copied_bytes += copied_bytes;
                NS_LOG_INFO(Simulator::Now () << ": Received packet with size " << dcep_package->cur_buffered_packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                // This is the case where the end of the Dcep package has been received
                if (dcep_package->copied_bytes >= dcep_package->size) {
                    NS_ASSERT_MSG(dcep_package->copied_bytes == dcep_package->size, "Error: Tried to copy " << (dcep_package->copied_bytes-dcep_package->size) << " too many bytes from dcep package; maybe it was copied wrongly?");
                    NS_ASSERT(InetSocketAddress::IsMatchingType (from));
                    if (dcep_package->node_failed) {
                        NS_LOG_WARN(Simulator::Now() << " Received rest of migration package, but not the full package");
                        return;
                    }
                    dcep->rcvRemoteMsg(dcep_package);
                    dcep_package->reset();
                }
                packet->RemoveAtStart(copied_bytes);
            }

            // This loop will be iterated through until all the Dcep packages have been received
            // Whereas the above block is executed if an existing Dcep package is being received,
            // this loop might be iterated multiple times if smaller packets have been merged together.
            // We use packet and not cur_buffered_packet here, because we haven't got a dcep package here
            while (packet->GetSize() > 0) {
                // Here, we do look for headers
                dcep_package->init(packet);
                auto succeeded = dcep_package->readHeader(14);
                if (!succeeded) {
                    // Wait until enough data arrives
                    break;
                }

                DcepHeader &dcepHeader = dcep_package->dcep_header;
                NS_ASSERT(dcepHeader.GetContentType() == EVENT);
                std::size_t payload_size = dcepHeader.GetContentSize();
                if (dcepHeader.GetContentSize() > packet->GetSize()) {
                    NS_ASSERT(payload_size > 0);
                    dcep_package->setSize(payload_size);
                    dcep_package->copied_bytes += packet->GetSize();
                    NS_LOG_INFO(Simulator::Now () << ": Received new packet with size " << packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                    packet->CopyData(dcep_package->payload, packet->GetSize());
                    packet->RemoveAtStart(packet->GetSize());
                } else {
                    NS_ASSERT(payload_size > 0);
                    NS_LOG_INFO(Simulator::Now() << " Received packet with size " << packet->GetSize() << " byte without header and " << payload_size << " bytes of payload in the header");
                    if (InetSocketAddress::IsMatchingType (from)) {
                        dcep_package->setSize(payload_size);
                        packet->CopyData(dcep_package->payload, dcepHeader.GetContentSize());
                        dcep->rcvRemoteMsg(dcep_package);
                    }
                    packet->RemoveAtStart(payload_size);
                    dcep_package->reset();
                }
            }
            NS_ASSERT_MSG(packet->GetSize() >= 0, "Invalid reading of packet!");
        }

        return;
    }

    void
    Communication::HandleReadFetch(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        Ptr<Packet> packet, pcopy;
        Ptr<Dcep> dcep = Dcep::GetDcepFromNode(disnode);

        Address from;
        if (!running) {
            NS_LOG_INFO("Dropping packet because not running anymore");
            // TODO: Should we delete a possible socketToDcepPacket here?
        }

        auto nid = socketToNodeIdFetch[socket];
        if (nodeIdToDcepPackageFetch[nid] == nullptr) {
            nodeIdToDcepPackageFetch[nid] = CreateObject<DcepPackage>(socket);
            nodeIdToDcepPackageFetch[nid]->reset();
        }
        auto &dcep_package = nodeIdToDcepPackageFetch[nid];

        while ((packet = socket->RecvFrom (from)))
        {
            Ptr<Packet> original_packet = packet;
            (void)original_packet;
            // Each packet can contain multiple merged Dcep packages
            // This is the case where the packet contains the continuation of an existing Dcep package
            if (dcep_package->active) {
                // This means that we don't look for a DcepHeader
                dcep_package->cur_buffered_packet->AddAtEnd(packet);
                packet = dcep_package->cur_buffered_packet;
                if (!dcep_package->loaded_header) {
                    if (!dcep_package->readHeader(83)) {
                        continue;
                    }
                }
                auto packet_size = packet->GetSize();
                int copied_bytes = packet_size;
                if (dcep_package->copied_bytes + copied_bytes > dcep_package->size) {
                    copied_bytes -= dcep_package->copied_bytes + packet_size - dcep_package->size;
                }
                packet->CopyData(&dcep_package->payload[dcep_package->copied_bytes], copied_bytes);
                dcep_package->copied_bytes += copied_bytes;
                NS_LOG_INFO(Simulator::Now () << ": Received packet with size " << dcep_package->cur_buffered_packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                // This is the case where the end of the Dcep package has been received
                if (dcep_package->copied_bytes >= dcep_package->size) {
                    NS_ASSERT_MSG(dcep_package->copied_bytes == dcep_package->size, "Error: Tried to copy " << (dcep_package->copied_bytes-dcep_package->size) << " too many bytes from dcep package; maybe it was copied wrongly?");
                    if (InetSocketAddress::IsMatchingType (from)) {
                        if (dcep_package->node_failed) {
                            NS_LOG_WARN(Simulator::Now() << " Received rest of migration package, but not the full package");
                            return;
                        }
                        dcep->rcvRemoteMsg(dcep_package);
                    }
                    //delete dcep_package->payload;
                    dcep_package->reset();
                    //nodeIdToDcepPackageState.erase(nid);
                    //std::cout << Simulator::Now() << " Deleting key for socketToDcepPacket " << socket << std::endl;
                }
                packet->RemoveAtStart(copied_bytes);
            }

            // This loop will be iterated through until all the Dcep packages have been received
            // Whereas the above block is executed if an existing Dcep package is being received,
            // this loop might be iterated multiple times if smaller packets have been merged together.
            // We use packet and not cur_buffered_packet here, because we haven't got a dcep package here
            while (packet->GetSize() > 0) {
                // Here, we do look for headers
                dcep_package->init(packet);
                auto succeeded = dcep_package->readHeader(83);
                if (!succeeded) {
                    // Wait until enough data arrives
                    break;
                }

                DcepHeader &dcepHeader = dcep_package->dcep_header;
                NS_ASSERT(dcepHeader.GetContentType() == MIGRATION_FETCH);
                std::size_t payload_size = dcepHeader.GetContentSize();
                if (dcepHeader.GetContentSize() > packet->GetSize()) {
                    NS_ASSERT(payload_size > 0);
                    dcep_package->setSize(payload_size);
                    dcep_package->copied_bytes += packet->GetSize();
                    NS_LOG_INFO(Simulator::Now () << ": Received new packet with size " << packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                    packet->CopyData(dcep_package->payload, packet->GetSize());
                    packet->RemoveAtStart(packet->GetSize());
                } else {
                    NS_ASSERT(payload_size > 0);
                    NS_LOG_INFO(Simulator::Now() << " Received packet with size " << packet->GetSize() << " byte without header and " << payload_size << " bytes of payload in the header");
                    if (InetSocketAddress::IsMatchingType (from)) {
                        dcep_package->setSize(payload_size);
                        dcep_package->dcep_header = dcepHeader;
                        packet->CopyData(dcep_package->payload, dcepHeader.GetContentSize());
                        dcep->rcvRemoteMsg(dcep_package);
                    }
                    packet->RemoveAtStart(payload_size);
                    dcep_package->reset();
                }
            }
            NS_ASSERT_MSG(packet->GetSize() >= 0, "Invalid reading of packet!");
        }

        return;
    }

    void
    Communication::HandleReadState(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        Ptr<Packet> packet, pcopy;
        Ptr<Dcep> dcep = Dcep::GetDcepFromNode(disnode);

        Address from;
        if (!running) {
            NS_LOG_INFO("Dropping packet because not running anymore");
            // TODO: Should we delete a possible socketToDcepPacket here?
        }

        auto nid = socketToNodeIdState[socket];
        if (nodeIdToDcepPackageState[nid] == nullptr) {
            nodeIdToDcepPackageState[nid] = CreateObject<DcepPackage>(socket);
            nodeIdToDcepPackageState[nid]->reset();
        }
        auto &dcep_package = nodeIdToDcepPackageState[nid];

        while ((packet = socket->RecvFrom (from)))
          {
            Ptr<Packet> original_packet = packet;
              (void)original_packet;
            // Each packet can contain multiple merged Dcep packages
            // This is the case where the packet contains the continuation of an existing Dcep package
            if (dcep_package->active) {
                //NS_ASSERT(dcep_package->size > 0);
                // This means that we don't look for a DcepHeader
                dcep_package->cur_buffered_packet->AddAtEnd(packet);
                packet = dcep_package->cur_buffered_packet;
                if (!dcep_package->loaded_header) {
                    if (!dcep_package->readHeader(149)) {
                        continue;
                    }
                }
                auto packet_size = packet->GetSize();
                int copied_bytes = packet_size;
                if (dcep_package->copied_bytes + copied_bytes > dcep_package->size) {
                    copied_bytes -= dcep_package->copied_bytes + packet_size - dcep_package->size;
                }
                packet->CopyData(&dcep_package->payload[dcep_package->copied_bytes], copied_bytes);
                dcep_package->copied_bytes += copied_bytes;
                NS_LOG_INFO(Simulator::Now () << ": Received packet with size " << dcep_package->cur_buffered_packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                // This is the case where the end of the Dcep package has been received
                if (dcep_package->copied_bytes >= dcep_package->size) {
                    NS_ASSERT_MSG(dcep_package->copied_bytes == dcep_package->size, "Error: Tried to copy " << (dcep_package->copied_bytes-dcep_package->size) << " too many bytes from dcep package; maybe it was copied wrongly?");
                    if (InetSocketAddress::IsMatchingType (from)) {
                        if (dcep_package->node_failed) {
                            NS_LOG_WARN(Simulator::Now() << " Received rest of migration package, but not the full package");
                            return;
                        }
                        dcep->rcvRemoteMsg(dcep_package);
                    }
                    dcep_package->reset();
                }
                packet->RemoveAtStart(copied_bytes);
            }

            // This loop will be iterated through until all the Dcep packages have been received
            // Whereas the above block is executed if an existing Dcep package is being received,
            // this loop might be iterated multiple times if smaller packets have been merged together.
            // We use packet and not cur_buffered_packet here, because we haven't got a dcep package here
            while (packet->GetSize() > 0) {
                // Here, we do look for headers
                dcep_package->init(packet);
                auto succeeded = dcep_package->readHeader(149);
                if (!succeeded) {
                    // Wait until enough data arrives
                    break;
                }

                DcepHeader &dcepHeader = dcep_package->dcep_header;
                NS_ASSERT(dcepHeader.GetContentType() == MIGRATION);
                std::size_t payload_size = dcepHeader.GetContentSize();
                if (dcepHeader.GetContentSize() > packet->GetSize()) {
                    NS_ASSERT(payload_size > 0);
                    dcep_package->setSize(payload_size);
                    dcep_package->copied_bytes += packet->GetSize();
                    NS_LOG_INFO(Simulator::Now () << ": Received new packet with size " << packet->GetSize() << " from " << from << ", received " << dcep_package->copied_bytes << "/" << dcep_package->size << " bytes");
                    packet->CopyData(dcep_package->payload, packet->GetSize());
                    packet->RemoveAtStart(packet->GetSize());
                } else {
                    NS_ASSERT(payload_size > 0);
                    NS_LOG_INFO(Simulator::Now() << " Received packet with size " << packet->GetSize() << " byte without header and " << payload_size << " bytes of payload in the header");
                    if (InetSocketAddress::IsMatchingType (from)) {
                        dcep_package->setSize(payload_size);
                        packet->CopyData(dcep_package->payload, dcepHeader.GetContentSize());
                        dcep->rcvRemoteMsg(dcep_package);
                    }
                    packet->RemoveAtStart(payload_size);
                    dcep_package->reset();
                }
            }
            NS_ASSERT_MSG(packet->GetSize() >= 0, "Invalid reading of packet!");
           }

        return;
    }

    void Communication::SendNextState() {
        auto &mp = ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]->mp;
        if (!sendStateQueue.empty()) {
            SendStateCallback ssc = sendStateQueue[0];
            if (ssc.left != 0) {
                return;
            }
            sendStateQueue.pop_front();
            doSendRequest(ssc.old_host, ssc.new_host, ssc.size, ssc.dcepHeader, ssc.type);
        } else if (mp != nullptr && !mp->finished && (mp->old_host == this->node_id || mp->old_host == 0)) {
            Simulator::Schedule(MilliSeconds(0), &ExposeWrapper::MoveNextState, ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]);
            //ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]->MoveNextState();
        } else if (!ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]->ps_fetch_queue.empty()) {
            if (ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]->ps_fetch_queue.size() >= ExposeCoordinator::BATCH_SIZE) {
                Simulator::ScheduleNow(&ExposeWrapper::FetchNextPartialState,
                                    ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]);
            } else {
                Simulator::Schedule(MilliSeconds(10), &ExposeWrapper::FetchNextPartialState,
                                    ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]);
            }
            //ExposeCoordinator::nodeIdsToWrappers[std::to_string(node_id)]->MoveNextState();
        } else {
            send();
        }
    }

    void
    Communication::doSendRequest(int old_host, int new_host, std::size_t bytes_left, DcepHeader &dcepHeader, int type) {
        if (bytes_left == 0) {
            currently_migrating = false;
            Simulator::Schedule(MilliSeconds(0), &Communication::SendNextState, this);
            return;
        }
        current_full_package_size = dcepHeader.GetContentSize();
        Ptr<Socket> socket;
        if (type == MIGRATION) {
            socket = nodeIdToStateSocket[new_host];
        } else if (type == MIGRATION_FETCH) {
            socket = nodeIdToFetchSocket[new_host];
        } else {
            NS_FATAL_ERROR("Invalid type");
        }

        std::size_t max_packet_size = socket->GetTxAvailable();
        std::size_t cur_pkt_size = bytes_left;
        DcepHeader dcepHeaderToSend = dcepHeader;
        if (bytes_left > max_packet_size) {
            cur_pkt_size = max_packet_size;
        }
        bool add_header = false;
        if (bytes_left == current_full_package_size) {
            add_header = true;
            bytes_left_of_package = current_full_package_size;
            // We only send the header as the first packet!
            //cur_pkt_size = 1;
        } else {
            NS_LOG_INFO("Not sending header");
        }
        std::size_t next_pkt_size = bytes_left - cur_pkt_size;

        //uint8_t state[cur_pkt_size];
        Ptr<Packet> newPacket = Create<Packet>(cur_pkt_size);
        if (add_header) {
            newPacket->AddHeader(dcepHeaderToSend);
        }
        // Here, we place it at front because we prioritize the CURRENT state to send
        sendStateQueue.emplace_front(old_host, new_host, next_pkt_size, newPacket->GetSize(), dcepHeader, type);
        auto cur_node = StreamQuery::nodeIdToNode[old_host];
        auto next_node = StreamQuery::nodeIdToNode[new_host];
        auto dcep = Dcep::GetDcepFromNode(cur_node);
        auto streamIdToNodes = dcep->GetObject<CEPEngine>()->GetStreamIdToNodes();
        std::pair<Ipv4Address, int> ipAndPort;
        int port = -1;
        if (type == MIGRATION_FETCH) {
            ipAndPort = ExposeCoordinator::nodeIdToFetchIpAndPort[new_host];
            port = ipAndPort.second+2;
        } else {
            ipAndPort = ExposeCoordinator::nodeIdToMigrationIpAndPort[new_host];
            port = ipAndPort.second;
        }

        InetSocketAddress address = InetSocketAddress(ipAndPort.first, port);

        m_stateSendQueue.push_back(newPacket);
        //auto ipv4address = address.GetIpv4().Get();
        //auto to_node_id = ipToNodeId[ipv4address];
        NS_LOG_INFO(Simulator::Now() << " Sending packet to " << address);

        m_stateSocketQueue.push_back(socket);
        currently_migrating = true;
        send();
    }

    void
    Communication::SendState(int old_host, int new_host, std::size_t bytes_left, DcepHeader &dcepHeader) {
        doSendRequest(old_host, new_host, bytes_left, dcepHeader, MIGRATION);
    }

    void
    Communication::SendFetchRequest(int old_host, int new_host, std::size_t bytes_left, DcepHeader &dcepHeader) {
        doSendRequest(old_host, new_host, bytes_left, dcepHeader, MIGRATION_FETCH);
    }

    void Communication::ScheduleSend(Ptr<Packet> p, InetSocketAddress addr)
    {
        DcepHeader dcepHeader;
        p->PeekHeader(dcepHeader);
        auto ipv4address = addr.GetIpv4().Get();
        auto to_node_id = ipToTuplesNodeId[ipv4address];
        NS_LOG_INFO(Simulator::Now() << " Sending packet to " << addr);

        auto contentType = dcepHeader.GetContentType();
        if (contentType != EVENT && contentType != QUERY && contentType != MIGRATION) {
            NS_FATAL_ERROR("Wrong content type of packet being sent: " << contentType);
        }

        m_tupleSendQueue.emplace_back(p);
        m_tupleSocketQueue.emplace_back(nodeIdToTupleSocket[to_node_id]);
        send();
        //auto to_comm = Dcep::GetDcepFromNode(ExposeCoordinator::nodeIdsToWrappers[std::to_string(to_node_id)]->node)->c_communication;
        //Simulator::Schedule(Seconds((p->GetSize()+54)*8.0/10000000.0), &Communication::HandleReadFakeTransmit, to_comm, this->node_id, p);
    }

    void Communication::SocketClosedNormal (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        std::cout << Simulator::Now().GetMilliSeconds() << " Node " << this->node_id << " Socket closed normally" << std::endl;
        NS_LOG_INFO(Simulator::Now() << " Socket closed normally");
        auto dcep = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[this->node_id]);
        if (in_migration && socketToNodeIdState[socket] == migration_node) {
            dcep->HandleInterruptedMigration();
        }

        /*if (socketToDcepPackage[socket]) {
            // We have to finish
            auto dcep_package = socketToDcepPackage[socket];
            auto dcep_header = dcep_package->dcep_header;
            auto dcep = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[this->node_id]);
//            auto time_now = Simulator::Now().GetMilliSeconds();
            switch (dcep_header.GetContentType()) {
                case EVENT: {
                    break;
                } case MIGRATION: {
                    // Here we finish the migration, despite it not being finished
                    dcep_package->dcep_header.arguments["op_final_batch"] = true;
                    dcep_package->dcep_header.arguments["last-batch"] = true;
                    dcep_package->dcep_header.arguments["active_op"] = true;
                    std::cout << "INTERRUPTED MIGRATION" << std::endl;
                    dcep->rcvRemoteMsg(dcep_package);
                    //delete dcep_package->payload;
                    socketToDcepPackage[socket]->node_failed = true;
                    break;
                } case QUERY: {
                    break;
                } default: {
                    NS_FATAL_ERROR("Invalid dcep header!");
                }
            }
        }*/
    }

    void Communication::SocketClosedError (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        NS_LOG_INFO(Simulator::Now() << " Socket closed with error");
        //std::cout << Simulator::Now() << " Node " << this->node_id << " Socket closed with error" << std::endl;
    }

    void Communication::ConnectionSucceeded (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        m_connected = true;
        int from_node = this->node_id;
        int to_node = -1;
        for (auto &[nid, s] : this->nodeIdToStateSocket) {
            if (s == socket) {
                to_node = nid;
                break;
            }
        }
        for (auto &[nid, s] : this->nodeIdToFetchSocket) {
            if (s == socket) {
                to_node = nid;
                std::cout << "Node " << this->node_id << " connected to fetch socket of Node " << nid << std::endl;
                break;
            }
        }
        for (auto &[nid, s] : this->nodeIdToTupleSocket) {
            if (s == socket) {
                to_node = nid;
                break;
            }
        }
        //auto new_packet = Create<Packet>();
        //socket->Send(new_packet);
        //NS_LOG_INFO(Simulator::Now() << " Node " << this->node_id << " Sending packet to " << to_node);
        std::cout << "Node " << from_node << " succeeded to connect to Node " << to_node << std::endl;
    }

    void Communication::ConnectionFailed (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION (this << socket);
        int from_node = this->node_id;
        int to_node = -1;
        for (auto &[nid, s] : this->nodeIdToStateSocket) {
            if (s == socket) {
                to_node = nid;
                break;
            }
        }
        std::cout << "Node " << from_node << " failed to connect to Node " << to_node << std::endl;
    }

    void
    Communication::ShutDown() {
        NS_LOG_INFO(Simulator::Now() << " Shutting down node " << this->node_id);
        this->running = false;
        for (auto &[nid, socket] : this->nodeIdToStateSocket) {
            NS_LOG_INFO(Simulator::Now() << " Closing state socket to Node " << nid);
            //socket->Close();
            auto comm = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[nid])->c_communication;
            comm->SocketClosedNormal(comm->nodeIdToStateSocket[this->node_id]);
        }
        for (auto &[nid, socket] : this->nodeIdToTupleSocket) {
            NS_LOG_INFO(Simulator::Now() << " Closing tuple socket to Node " << nid);
            //socket->Close();
            auto comm = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[nid])->c_communication;
            comm->SocketClosedNormal(comm->nodeIdToTupleSocket[this->node_id]);
        }
        for (auto &[nid, socket] : this->nodeIdToFetchSocket) {
            NS_LOG_INFO(Simulator::Now() << " Closing fetch socket to Node " << nid);
            //socket->Close();
            auto comm = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[nid])->c_communication;
            comm->SocketClosedNormal(comm->nodeIdToFetchSocket[this->node_id]);
        }
        /*for (auto &[node_id, socket] : this->nodeIdToStateSocket) {
            //this->nodeIdToIpAndPort[this->node_id]-
            //DynamicCast<TcpSocket>(socket)->
            socket->ShutdownSend();
            socket->ShutdownRecv();
        }*/

        //for (int dev = 0; dev < disnode->GetNDevices(); dev++) {
            /*std::string param = "/NodeList/[i]/DeviceList/[j]/$ns3::PointToPointNetDevice/ReceiveErrorModel/$ns3::RateErrorModel";
            Utility::replace(param, std::string("[i]"), std::to_string(disnode->GetId()));
            Utility::replace(param, std::string("[j]"), std::to_string(0));
            Config::Set(param, DoubleValue(1));*/
        Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
        em->SetAttribute ("ErrorRate", DoubleValue (1));
        em->SetAttribute ("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));
        GetNetDevice()->SetAttribute ("ReceiveErrorModel", PointerValue (em));
        // Device number "disnode->GetId()" points toward the PointToPointNetDevice of the router_node
        ExposeCoordinator::router_container.Get(0)->GetDevice(1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
    }

    int cnt3 = 0;
    void
    Communication::send()
    {
        while ((!m_stateSendQueue.empty() || !m_tupleSendQueue.empty()) && running)
        {
            Ptr<Packet> p;
            Ptr<Socket> s;
            if (!m_stateSendQueue.empty()) {
                p = m_stateSendQueue.front();
                s = m_stateSocketQueue.front();
            } else if (!m_tupleSendQueue.empty()) {
                // TODO: Why did we have the below stuff?
                /*if (currently_migrating) {
                    // We don't send regular tuples while we're migrating
                    return;
                }*/
                p = m_tupleSendQueue.front();
                s = m_tupleSocketQueue.front();
            } else {
                NS_FATAL_ERROR("Send queues are empty");
            }
            int ret;
            auto p_size = p->GetSize();
            if (s->GetTxAvailable() >= p_size) {
                ret = s->Send(p);
                if (ret == (int) p_size) {
                    ++total_sent;
                    ++m_sent;
                    NS_LOG_INFO (Simulator::Now() << " SUCCESSFUL TX from : " << host_address
                                                  //<< " to : " << ipv4.GetDestination()
                                                  << " packet size "
                                                  << p->GetSize()
                                                  << std::endl);
                    if (!m_stateSendQueue.empty()) {
                        m_stateSendQueue.pop_front();
                        m_stateSocketQueue.pop_front();
                    } else if (!m_tupleSendQueue.empty()) {
                        m_tupleSendQueue.pop_front();
                        m_tupleSocketQueue.pop_front();
                        // Only consider sending state as much as possible
                        break;
                    } else {
                        NS_FATAL_ERROR("Send queues are empty");
                    }
                } else {
                    NS_FATAL_ERROR("FAILED PACKET");
                    NS_LOG_INFO (Simulator::Now() << " Failed TX from : " << host_address
                                                  //<< " to : " << ipv4.GetDestination()
                                                  << " packet size "
                                                  << p->GetSize()
                                                  << std::endl);
                    NS_LOG_INFO (Simulator::Now() << " Error " << s->GetErrno());
                }
            } else {
                break;
            }
        }
    }

    Ipv4Address
    Communication::GetSinkAddress()
    {
        return m_sinkAddress;
    }

    Ipv4Address
    Communication::GetLocalAddress() {
        return host_address;
    }

    void Communication::HandleAccept (Ptr<Socket> s, const Address& from) {
        NS_LOG_FUNCTION (this << s);
        std::cout << "Node " << this->node_id << " accepted connection from Address " << from << std::endl;
    }

    void Communication::HandleAcceptState (Ptr<Socket> s, const Address& from) {
        NS_LOG_FUNCTION (this << s);
        NS_LOG_INFO("Node " << this->node_id << " accepted connection from Address " << from);
    }

    void Communication::NotSendingAnymore() {
        //waitingOnNotSendingAnymore = false;
        /*NS_LOG_INFO(Simulator::Now() << " Node " << this->node_id << " not sending anymore");
        if (!sendStateQueue.empty()) {
            // Send rest of state
            //Simulator::Schedule(MilliSeconds(100), &Communication::SendNextState, this);
            //SendNextState();
        } else {
            //send();
        }*/
    }

    void Communication::TxBufferAvailable(Ptr<Socket> s, uint32_t bytes_available) {
        if (bytes_available > 0) {
            if (!m_stateSendQueue.empty() || sendStateQueue.empty()) {
                Simulator::Schedule(MilliSeconds(0), &Communication::send, this);
            } else if (currently_migrating && !sendStateQueue.empty()) {
                // Send rest of state
                Simulator::Schedule(MilliSeconds(0), &Communication::SendNextState, this);
                //SendNextState();
                //SendNextState(bytes_available);
            }
        }
    }

    void Communication::HandleTupleSent (Ptr<Socket> s, uint32_t state_sent) {
        //Simulator::Schedule(MilliSeconds(0), &Communication::SendNextState, this);
    }

    void Communication::HandleFetchSent(Ptr<Socket> s, uint32_t state_sent) {

    }

    void Communication::HandleStateSent (Ptr<Socket> s, uint32_t state_sent) {
        NS_LOG_INFO(Simulator::Now() << ": HandleStateSent, Node " << this->node_id << " sent " << state_sent
                                         << " bytes, " << bytes_left_of_package << "/" << this->current_package_size << " bytes left of this TCP segment, and " << current_full_package_size << " bytes in the total package.");
        for (int i = 0; i < (int) sendStateQueue.size(); i++) {
            if (state_sent >= sendStateQueue[i].left) {
                state_sent -= sendStateQueue[i].left;
                sendStateQueue[i].left = 0;
                if (i == 0) {
                    Simulator::Schedule(MilliSeconds(0), &Communication::SendNextState, this);
                }
                continue;
            } else {
                sendStateQueue[i].left -= state_sent;
                break;
            }
        }

        /*if (state_sent >= sendStateQueue.front().left) {
            sendStateQueue.front().left = 0;
            Simulator::Schedule(MilliSeconds(0), &Communication::SendNextState, this);
        } else {
            sendStateQueue.front().left -= state_sent;
        }*/
    }

    void Communication::HandlePeerError (Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this << socket);
        int from_node = this->node_id;
        int to_node = -1;
        for (auto &[nid, s] : this->nodeIdToStateSocket) {
            if (s == socket) {
                to_node = nid;
                break;
            }
        }
        std::cout << "HandlePeerError: Node " << from_node << " failed to connect to Node " << to_node << std::endl;
    }

    void Communication::HandlePeerClose (Ptr<Socket> s) {
        NS_LOG_FUNCTION (this << s);
        SetupConnections();
    }
  
}

