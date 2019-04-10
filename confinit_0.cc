/* ========================================================================
 * CONFINIT Simulation
 *
 * Author: Carlos Alberto Pedroso Junior (capjunior@inf.ufpr.br)
 *
 * Date: December 01, 2018
 * Update(1):
 * Update(2):
 * ========================================================================
 */

#include "ns3/address-utils.h"
#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/applications-module.h"
#include "ns3/attribute.h"
#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/packet-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/stats-module.h"
#include "ns3/string.h"
#include "ns3/tag.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-module.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <random>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "sys/types.h"
#include "sys/stat.h"

/*
 * Model:
 *
 * 	   WiFi (Ad hoc)
 *  Node1		  Node2
 *  Source		  Sink
 *   (*) --------> (*)
 *  10.0.0.1	 10.0.0.2
 *  OnOff		 OnOff		>> Search neighbors
 * PacketSink	 PacketSink	>> Receive messages and answers
 * StatusOn		 StatusOn	>> Controls emergency situation
 */


using namespace ns3;
using namespace std;

/* ========================================================================
 * MyTag class
 *
 * Inherited from Tag class
 *
 * Create and add tags to packets
 *
 * ========================================================================
 */

class MyTag : public Tag
{
public:

  static TypeId 		GetTypeId (void);
  virtual TypeId 		GetInstanceTypeId (void) const;
  virtual uint32_t 		GetSerializedSize (void) const;
  virtual void 			Serialize (TagBuffer i) const;
  virtual void 			Deserialize (TagBuffer i);
  virtual void 			Print (std::ostream &os) const;
  void 					SetSimpleValue (uint8_t value);
  uint8_t 				GetSimpleValue (void) const;

private:
  uint8_t 				m_simpleValue;  //!< tag value
};


/**
 * \brief Get the type ID.
 * \return the object TypeId
 */

TypeId
MyTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyTag")
    .SetParent<Tag> ()
    .AddConstructor<MyTag> ()
    .AddAttribute ("SimpleValue",
                   "A simple value",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&MyTag::GetSimpleValue),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
MyTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MyTag::GetSerializedSize (void) const
{
  return 1;
}

void
MyTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_simpleValue);
}

void
MyTag::Deserialize (TagBuffer i)
{
  m_simpleValue = i.ReadU8 ();
}

void
MyTag::Print (std::ostream &os) const
{
  //os << "v=" << (uint32_t)m_simpleValue;
  std::cout << "Tag " << (uint32_t)m_simpleValue << std::endl;
}

/**
 * Set the tag value
 * \param value The tag value.
 */
void
MyTag::SetSimpleValue (uint8_t value)
{
  m_simpleValue = value;
}


/**
 * Get the tag value
 * \return the tag value.
 */

uint8_t
MyTag::GetSimpleValue (void) const
{
  return m_simpleValue;
}

/* ------------------------------------------------------------------------
 * End of MyTag class
 * ------------------------------------------------------------------------
 */

/* ========================================================================
 * ConfinitOnOFF class
 *
 * Inherited from Application class
 *
 * Create and send packages
 *
 * ========================================================================
 */

class ConfinitOnOFF : public Application
{
public:
    static TypeId 			GetTypeId (void);
							ConfinitOnOFF ();
							~ConfinitOnOFF ();
	void 					Setup(Address address, uint32_t protocolId);
    int64_t 				AssignStreams (int64_t stream);

private: // inherited from OnOffApplication base class.

  void 						StartApplication (void);    // Called at time specified by Start
  void 						StopApplication (void);     // Called at time specified by Stop
  void 						CancelEvents ();
  void 						StartSending ();
  void 						StopSending ();
  void 						SendPacket ();
  Address					GetNodeIpAddress ();
  void 						PrintMyNeighborList ();
  std::string				GetMyNeighborsList();

  Ptr<Socket>     			m_socket;       	//!< Associated socket
  Address         			m_peer;         	//!< Peer address
  bool            			m_connected;    	//!< True if connected
  Ptr<RandomVariableStream> m_onTime;       	//!< rng for On Time
  Ptr<RandomVariableStream> m_offTime;      	//!< rng for Off Time
  DataRate        			m_cbrRate;      	//!< Rate that data is generated
  DataRate        			m_cbrRateFailSafe;  //!< Rate that data is generated (check copy)
  uint32_t        			m_pktSize;      	//!< Size of packets
  uint32_t        			m_residualBits; 	//!< Number of generated, but not sent, bits
  Time            			m_lastStartTime;	//!< Time last packet sent
  uint64_t        			m_maxBytes;     	//!< Limit total number of bytes sent
  uint64_t        			m_totBytes;     	//!< Total bytes sent so far
  EventId         			m_startStopEvent;   //!< Event id for next start or stop event
  EventId         			m_sendEvent;    	//!< Event id of pending "send packet" event
  TypeId          			m_tid;          	//!< Type of the socket used
  Address					m_node;				//!< Node's IP Address
  Ipv4Address				m_nodeIP;			//!< Node's IPv4 Address

  TracedCallback<Ptr<const Packet>> m_txTrace;  //!< Traced Callback: transmitted packets.
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses; //!< Traced Callback: transmitted packets with addresses
  TracedCallback<Ipv4Address, int, std::string> m_nodeTrace;  //!< Traced Callback: broadcast messages

private:
  void 						ScheduleNextTx ();
  void 						ScheduleStartEvent ();
  void 						ScheduleStopEvent ();
  void 						ConnectionSucceeded (Ptr<Socket> socket);
  void 						ConnectionFailed (Ptr<Socket> socket);

};

TypeId
ConfinitOnOFF::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ConfinitOnOFF")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<ConfinitOnOFF> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&ConfinitOnOFF::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&ConfinitOnOFF::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&ConfinitOnOFF::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&ConfinitOnOFF::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&ConfinitOnOFF::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ConfinitOnOFF::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&ConfinitOnOFF::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&ConfinitOnOFF::m_txTrace),
                     "ns3::Packet::TracedCallback")
	.AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
				     MakeTraceSourceAccessor (&ConfinitOnOFF::m_txTraceWithAddresses),
					 "ns3::Packet::TwoAddressTracedCallback")
	.AddTraceSource ("Broadcasts", "A broadcast is sent to search neighbors",
                     MakeTraceSourceAccessor (&ConfinitOnOFF::m_nodeTrace),
					 "ns3::ConfinitOnOFF::TracedCallback")
  ;
  return tid;
}

ConfinitOnOFF::ConfinitOnOFF ()
{
  m_socket = 0;
  m_connected = false;
  m_residualBits = 0;
  m_lastStartTime = Seconds (0);
  m_totBytes = 6; // Total bytes sent on time
  m_maxBytes = 0;
  m_pktSize = 6;
  NS_LOG_FUNCTION (this);
}

ConfinitOnOFF::~ConfinitOnOFF()
{
  NS_LOG_FUNCTION (this);
}

void
ConfinitOnOFF::Setup(Address address, uint32_t protocolId)
{
  NS_LOG_FUNCTION (this);
  m_peer = address;
  m_node = GetNodeIpAddress ();
  m_nodeIP = InetSocketAddress::ConvertFrom(m_node).GetIpv4 ();

  // Choose protocol
  if (protocolId == 1) //1 Udp
	m_tid = ns3::UdpSocketFactory::GetTypeId();
  else // 2 tcp
	m_tid = ns3::TcpSocketFactory::GetTypeId();
}

int64_t
ConfinitOnOFF::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}

// Application Methods
void ConfinitOnOFF::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();

      m_socket->SetConnectCallback (
      MakeCallback (&ConfinitOnOFF::ConnectionSucceeded, this),
      MakeCallback (&ConfinitOnOFF::ConnectionFailed, this));
    }
  m_cbrRateFailSafe = m_cbrRate;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();
}

void ConfinitOnOFF::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  //else
  //  {
  //    NS_LOG_WARN ("ConfinitOnOFF found null socket to close in StopApplication");
  //  }
}

void ConfinitOnOFF::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
  m_cbrRateFailSafe = m_cbrRate;
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}

// Event handlers
void ConfinitOnOFF::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void ConfinitOnOFF::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();
  ScheduleStartEvent ();
}

// Private helpers
void ConfinitOnOFF::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);

      // Original - Don't remove these comments
      //Time nextTime (Seconds (bits /
      //                        static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet

      // 080918 - Modified to send just one packet in onTime interval, since m_OnTime = 1
      Time nextTime (Seconds (0.3));

      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime, &ConfinitOnOFF::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void ConfinitOnOFF::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &ConfinitOnOFF::StartSending, this);
}

void ConfinitOnOFF::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &ConfinitOnOFF::StopSending, this);
}

void ConfinitOnOFF::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr <Node> ThisNode = this->GetNode();
  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> ("Hello!"), 6);

  // Check if node status is 1 (Emergency) and service status. If both true, stop OnOff application
  // Stop searching for neighbors
  // 01Nov18 modif 110119

  if (ThisNode->GetStatus() && ThisNode->GetServiceStatus())
  {
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
			       << ":Stop searching neighbors (ConfinitOnOFF).");
	  StopApplication ();
  }
  else
  {
  // Tag value 0: Broadcast
  // 		   1: Unicast - Identification
  //		   2: Unicast - Emergency message
  //		   3: Unicast - Confirm receive Emergency message
  // 		   4: Broadcast - Announce stopping operation

  MyTag tag; // Create a tag
  tag.SetSimpleValue(0); // Set tag value
  packet->AddPacketTag(tag); // Add tag to the packet

  // Unregister nodes not around
  ThisNode->UnregisterOffNeighbors();

  // TurnOff live neighbors
  ThisNode->TurnOffLiveNeighbors();

  m_txTrace (packet);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;

  InetSocketAddress receiverAddress = InetSocketAddress::ConvertFrom (m_peer); // Receiver address

  Address localAddress;
  m_socket->GetSockName (localAddress);
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
	  if (receiverAddress.GetIpv4().IsBroadcast()) // Broadcast message
	  {
/*		NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	                 << ":Sent message to search neighbors!");
	*/  }
	  else
	  {
		NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
		             << ":Sent message to " << receiverAddress.GetIpv4 ()
					 << " - Tag "<< (int)tag.GetSimpleValue());
	  }
	  m_txTraceWithAddresses (packet, localAddress, receiverAddress);
   }
  m_nodeTrace(m_nodeIP, ThisNode->GetNNeighbors(), GetMyNeighborsList());
  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();
  }
}

void ConfinitOnOFF::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void ConfinitOnOFF::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

/* Get node NIC address where application is installed
 * 300918
 *
 * Input: NIL
 *
 * Output: Interface address
 */

Address
ConfinitOnOFF::GetNodeIpAddress ()
{
  Ptr <Node> PtrNode = this->GetNode();
  Ptr<Ipv4> ipv4 = PtrNode->GetObject<Ipv4> ();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
  Ipv4Address ipAddr = iaddr.GetLocal ();
  return (InetSocketAddress (ipAddr, 9));
}

/* Print a node neighbors' list
 * 300918
 *
 * Input: NIL
 *
 * Output: NIL
 */

void
ConfinitOnOFF::PrintMyNeighborList ()
{
	std::vector<Address> neighborList;
	bool alive;
	Ptr <Node> ThisNode = this->GetNode();
	neighborList = ThisNode->GetNeighborIpList();
	std::cout << m_nodeIP << " - My neighbors are: " << endl;
	for (uint8_t i = 0; i < neighborList.size(); i++)
	{
		alive = ThisNode->IsAliveNeighbor(neighborList[i]);
		std::cout << neighborList[i] << " - Around: " << alive << endl;
	}
}

std::string
ConfinitOnOFF::GetMyNeighborsList ()
{
	std::stringstream neighbors;
	std::vector<Address> neighborList;
	Ptr <Node> ThisNode = this->GetNode();
	if (ThisNode->IsThereAnyNeighbor() == false)
		neighbors << "";
	else
	{
		neighborList = ThisNode->GetNeighborIpList();
		for (uint8_t i = 0; i < neighborList.size(); i++)
			neighbors << InetSocketAddress::ConvertFrom(neighborList[i]).GetIpv4 () << "\t";
	}
	return (neighbors.str().substr(0,neighbors.str().size()-1));
}

/* ------------------------------------------------------------------------
 * End of ConfinitOnOFF class
 * ------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------
 * Ex...
 *---------------------------------------------------------------------------
 */

void ConfinitSimulation2(uint32_t nNodes) { //std::string simDate, uint32_t fixNodeNumber

	NS_LOG_INFO ("Confinit - Setting parameters...");
    double start = 0.0;
    double stop = 15.0;
    std::string traceFile = "scratch/ostermalm_003_1_new.tr";
    //std::string scenarioSimFile;
    //std::string tracesFolder;
	//std::vector <int> healthNodes; // Store nodes with health interest
	//std::vector <int> attendingPriorities; // Store nodes attending priorities
	//std::vector <int> emergencyTimes; // Store times to nodes enters in emergency
      std::vector<std::string> interestsTemp;
    //std::vector <int> fixedNodesPatients = {52,69,70};
    //std::vector <int> fixedNodesAttending = {63};

    NodeContainer::Iterator it;
    uint16_t port = 9;
    int i = 0;
    int x = 0;

    // Creating and defining seeds to be used
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);

         // Import node's mobility from the trace file
        // Necessary to use a helper from NS2
        //Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);

    	// Create nodes to be used during simulation
    	NodeContainer Nodes;
    	Nodes.Create (nNodes);

    	// Install node's mobility in all nodes
    	//ns2.Install();

        // Initialize node's status and service status
        for (it = Nodes.Begin() ; it != Nodes.End() ; it++)
        {
        	(*it)->SetAttribute("Status", BooleanValue(false));
        	(*it)->SetAttribute("ServiceStatus", BooleanValue(false));
        }


	// Set WIFI network - Ad Hoc
	//----------------------------------------------------------------------------------

	NS_LOG_INFO ("CONFINIT - Configuring wifi network (Ad Hoc)...");

	// Create wifi network 802.11a

	WifiHelper wifi;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
			                      StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold",
								  UintegerValue (0));

	// MAC Layer non QoS
	WifiMacHelper wifiMac;
	wifiMac.SetType ("ns3::AdhocWifiMac");

	// PHY layer
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());

	// Creating and installing netdevices in all nodes
	NetDeviceContainer devices;
	devices = wifi.Install (wifiPhy, wifiMac, Nodes);

	// Create and install Internet stack protocol
	InternetStackHelper stack;
	stack.Install (Nodes);

	// Set IPv4 address to node's interfaces
	Ipv4AddressHelper address;
	Ipv4InterfaceContainer NodesInterface;
	address.SetBase ("192.168.1.0", "255.255.255.0");

	NodesInterface = address.Assign (devices);
 
//----------------------------------------------------------------------------------
/*
// Set Sink application
//----------------------------------------------------------------------------------

	Address SinkBroadAddress (InetSocketAddress (Ipv4Address::GetAny (), port)); // SinkAddress for messages

    NS_LOG_INFO ("CONFINIT - Install Sink application ...");

   // Install Sink in all nodes

    for (it = Nodes.Begin() ; it != Nodes.End(); it++)
        {
        	Ptr<ConfinitPacketSink> SinkApp  = CreateObject<ConfinitPacketSink> ();
        	(*it)->AddApplication(SinkApp);

        	SinkApp->SetStartTime (Seconds (start));
        	SinkApp->SetStopTime (Seconds (stop));

        	//SinkApp->Setup(SinkBroadAddress, 1, competences); // 1 -> UDP, 2 -> TCP
        }
/*
 *
 */
//----------------------------------------------------------------------------------
// Set OnOff application
//----------------------------------------------------------------------------------

// Install OnOff in all nodes

NS_LOG_INFO ("CONFINIT - Install OnOff application ...");

for (it = Nodes.Begin() ; it !=  Nodes.End() ; it++)
{
    Ptr<ConfinitOnOFF> OnOffApp = CreateObject<ConfinitOnOFF> ();
    (*it)->AddApplication(OnOffApp);

    // Set to send to broadcast address
    OnOffApp->Setup(InetSocketAddress (Ipv4Address ("255.255.255.255"), 9),1); // 1 -> UDP, 2 -> TCP

    OnOffApp->SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    OnOffApp->SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=3]"));
    OnOffApp->SetAttribute ("DataRate",StringValue ("500kb/s"));
    OnOffApp->SetAttribute ("PacketSize", UintegerValue (6));

    OnOffApp->SetStartTime (Seconds (start));
    OnOffApp->SetStopTime (Seconds (stop));
    //start+=0.2; // Avoid to start all the OnOff together
}

	//Statistics statistics (simDate, tracesFolder.c_str());

//----------------------------------------------------------------------------------
/*
std::string
GetTimeOfSimulationStart()
{
	time_t now = time(0);
    tm *ltm = localtime(&now);
    ostringstream convert;

    if (ltm->tm_mday < 10)
    	convert << "0";
    convert << ltm->tm_mday;

    if ((ltm->tm_mon+1) < 10)
       	convert << "0";
    convert << ltm->tm_mon + 1;

    convert << 1900 + ltm->tm_year << "_" ;

    if (ltm->tm_hour < 10)
    	convert << "0";
    convert << ltm->tm_hour;

    if (ltm->tm_min < 10)
    	convert << "0";
    convert << ltm->tm_min << endl<< endl;

	return convert.str();
}
*/
/* ========================================================================
 * Main
 * ========================================================================
 */
int
main (int argc, char *argv[])
{
	// Set Parameters
	uint32_t nNodes = 100;
	std::string simTime;
	uint32_t  fixNode = 0;

	LogComponentEnable ("CenarioStealth_v1", LOG_LEVEL_INFO);

	//NS_LOG_INFO ("STEALTH - Initializing...");

	CommandLine cmd;
	cmd.AddValue ("nNodes", "Number of node devices", nNodes);
	cmd.AddValue ("fixNode", "Number of nodes with fixed settings", fixNode);
	cmd.Parse (argc,argv);

    //simTime = GetTimeOfSimulationStart();

    //SeedManager::SetRun(nRun); // update seed to n executions
    if (fixNode < 4 )
    	ConfinitSimulation2(nNodes, simTime, fixNode);
    else
    	std::cout << "STEALTH - Error: Maximum 3 fixed nodes!" << std::endl;

	//NS_LOG_INFO ("STEALTH - Done!...");

	return 0;
}

/* ------------------------------------------------------------------------
 * End of Main
 * ------------------------------------------------------------------------
 */





 
 
 
 
 
 
