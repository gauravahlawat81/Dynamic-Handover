/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "packet-loss-counter.h"

#include "seq-ts-header.h"
#include "sdn-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("sdnServer");

NS_OBJECT_ENSURE_REGISTERED (sdnServer);


TypeId
sdnServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::sdnServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<sdnServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&sdnServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&sdnServer::GetPacketWindowSize,
                                         &sdnServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&sdnServer::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&sdnServer::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

sdnServer::sdnServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received=0;
}

sdnServer::~sdnServer ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t
sdnServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
sdnServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
sdnServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint64_t
sdnServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

void
sdnServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
sdnServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);


  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&sdnServer::HandleRead, this));

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (),
                                                   m_port);
      if (m_socket6->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket6->SetRecvCallback (MakeCallback (&sdnServer::HandleRead, this));

  std::cout << "Server :: " << m_socket << " " << m_socket6 << "\n";
}

void
sdnServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
sdnServer::HandleRead (Ptr<Socket> socket)
{
  std::cout << "Something got..." << socket << "\n";
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      std::cout << packet->ToString() << "--\n";

      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
      if (packet->GetSize () > 0)
        {
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);

          uint8_t *buffer = new uint8_t[packet->GetSize ()];
          packet->CopyData(buffer, packet->GetSize ());
          std::string s = std::string(buffer, buffer+packet->GetSize());
          
          HoProcess(s);

          uint32_t currentSequenceNumber = seqTs.GetSeq ();
          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }
          else if (Inet6SocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }

          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;
        }
    }
    //handover dec | source cell ID | ue rnti | trg cell id
  // std::string name= "0|0|1|2";
  // char data[ name.length() ];
  // std::strcpy(data,name.c_str());
  // m_hoClients[0]->SendData(data , name.length()+1);

    std::cout << "Addr ::" << from << "\n";

}

void 
sdnServer::HoProcess( std::string params ){
std::cout<<"Received --->  "<< params << "\n";

    const char* delim = "|";
    std::vector<std::string> out;
    tokenize(params, delim, out);
    if(out.size() == 1 )
    {
      ut_velocity =  stoi(out[0]);
      return;

    }

    for( int i=2;i< (signed)out.size();i+=2 ){
      sinrTable[{ stoi(out[0]) , stoi(out[1]) }][stoi(out[i])] = stoi(out[i+1]);
      // std:: cout << stoi(out[0]) << " " << stoi(out[1]) << " " << stoi(out[i]) << " " << stod(out[i+1]) << "\n";
    }

    for( auto itr : sinrTable[{ stoi(out[0]) , stoi(out[1]) }] ){
      if( stoi(out[0]) != (signed)itr.first ){
        int rsrp_serving = sinrTable[{ stoi(out[0]) , stoi(out[1]) }][stoi(out[0])];
        int rsrp_target = itr.second;
        bool res = VelocityBasedHO(rsrp_serving,rsrp_target);
        // std::cout<<"Printing out res "<<res<<"\n";
        if(res){
          std::cout <<"---------- Handover Init -----------" << "\n";
          std::string name= "1|" + out[0] + "|" + out[1] + "|" + std::to_string(itr.first);
          std::cout << name << "\n";
          char data[ name.length() ];
          std::strcpy(data,name.c_str());
          m_hoClients[0]->SendData(data , name.length()+1);
        }
      }
    }


}

bool
sdnServer::VelocityBasedHO(int rsrp_serving,int rsrp_target){
  bool decision = false;
  std::cout<<"RSRP Serving and RSRP target "<<rsrp_serving<<" "<<rsrp_target<<"\n";
  if(rsrp_serving > rsrp_target + theta){
    std::cout<<"In velocity based condition one is true\n";
    condition1update();
    decision = a3rsrpAlgo(rsrp_serving,rsrp_target);
  }
  else if(rsrp_serving < rsrp_target + theta){
    std::cout<<"In velocity based condition two is true\n";
    condition3update();
    decision = a3rsrpAlgo(rsrp_serving,rsrp_target);
  }
  else{
    std::cout<<"In velocity based condition three is true\n";
    condition2update();
    decision = a3rsrpAlgo(rsrp_serving,rsrp_target);
  }
  return decision;
}

void
sdnServer::condition1update(){
  if(ut_velocity > ref_velocity){
    HOM-=step_hom;
    TT-=step_tt;
  }
  else{
    HOM+=step_hom;
    TT+=step_tt;
  }
}

void
sdnServer::condition2update(){
  if(ut_velocity > ref_velocity){
    HOM-=step_hom;
    TT-=step_tt;
  }
  else if(ut_velocity == ref_velocity){
  }
  else{
    HOM+=step_hom;
    TT+=step_tt;
  }
}

void
sdnServer::condition3update(){
  HOM-=step_hom;
  TT-=step_tt;
  if(HOM<=0)
  {
    HOM=1;
  }
}

bool
sdnServer::a3rsrpAlgo(int rsrp_serving,int rsrp_target){
  std::cout<<"HOM and TTT are "<<HOM<<" "<<TT<<"\n";
  if(rsrp_target >= rsrp_serving + HOM)
  {
    return true;
  }
  return false;

}

void 
sdnServer::tokenize(std::string const &str, const char* delim,
            std::vector<std::string> &out)
{
    char *token = strtok(const_cast<char*>(str.c_str()), delim);
    while (token != nullptr)
    {
        out.push_back(std::string(token));
        token = strtok(nullptr, delim);
    }
}

} // Namespace ns3
