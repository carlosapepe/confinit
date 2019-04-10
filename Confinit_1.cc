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

 class ConfinitiOnOff : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  ConfinitiOnOff();

  virtual ~ConfinitiOnOff();

  /**
   * \brief Set the total number of bytes to send.
   *
   * Once these bytes are sent, no packet is sent again, even in on state.
   * The value zero means that there is no limit.
   *
   * \param maxBytes the total number of bytes to send
   */
  void SetMaxBytes (uint64_t maxBytes);

  /**
   * \brief Return a pointer to associated socket.
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;

 /**
  * \brief Assign a fixed random variable stream number to the random variables
  * used by this model.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  //helpers
  /**
   * \brief Cancel all pending events.
   */
  void CancelEvents ();

  // Event handlers
  /**
   * \brief Start an On period
   */
  void StartSending ();
  /**
   * \brief Start an Off period
   */
  void StopSending ();
  /**
   * \brief Send a packet
   */
  void SendPacket ();

  Ptr<Socket>     m_socket;       //!< Associated socket
  Address         m_peer;         //!< Peer address
  bool            m_connected;    //!< True if connected
  Ptr<RandomVariableStream>  m_onTime;       //!< rng for On Time
  Ptr<RandomVariableStream>  m_offTime;      //!< rng for Off Time
  DataRate        m_cbrRate;      //!< Rate that data is generated
  DataRate        m_cbrRateFailSafe;      //!< Rate that data is generated (check copy)
  uint32_t        m_pktSize;      //!< Size of packets
  uint32_t        m_residualBits; //!< Number of generated, but not sent, bits
  Time            m_lastStartTime; //!< Time last packet sent
  uint64_t        m_maxBytes;     //!< Limit total number of bytes sent
  uint64_t        m_totBytes;     //!< Total bytes sent so far
  EventId         m_startStopEvent;     //!< Event id for next start or stop event
  EventId         m_sendEvent;    //!< Event id of pending "send packet" event
  TypeId          m_tid;          //!< Type of the socket used

  /// Traced Callback: transmitted packets.
  TracedCallback<Ptr<const Packet> > m_txTrace;

private:
  /**
   * \brief Schedule the next packet transmission
   */
  void ScheduleNextTx ();
  /**
   * \brief Schedule the next On period start
   */
  void ScheduleStartEvent ();
  /**
   * \brief Schedule the next Off period start
   */
  void ScheduleStopEvent ();
  /**
   * \brief Handle a Connection Succeed event
   * \param socket the connected socket
   */
  void ConnectionSucceeded (Ptr<Socket> socket);
  /**
   * \brief Handle a Connection Failed event
   * \param socket the not connected socket
   */
  void ConnectionFailed (Ptr<Socket> socket);
};

S_LOG_COMPONENT_DEFINE ("ConfinitiConfinitOnOff");

NS_OBJECT_ENSURE_REGISTERED (ConfinitiConfinitOnOff);

TypeId
ConfinitiConfinitOnOff::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ConfinitiConfinitOnOff")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<ConfinitiConfinitOnOff> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&ConfinitOnOff::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&ConfinitOnOff::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&ConfinitOnOff::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&ConfinitOnOff::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&ConfinitOnOff::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ConfinitOnOff::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&ConfinitOnOff::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&ConfinitOnOff::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


ConfinitOnOff::ConfinitOnOff ()
  : m_socket (0),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0)
{
  NS_LOG_FUNCTION (this);
}

ConfinitOnOff::~ConfinitOnOff()
{
  NS_LOG_FUNCTION (this);
}

void
ConfinitOnOff::SetMaxBytes (uint64_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
ConfinitOnOff::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

int64_t
ConfinitOnOff::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}

void
ConfinitOnOff::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void ConfinitOnOff::StartApplication () // Called at time specified by Start
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
        MakeCallback (&ConfinitOnOff::ConnectionSucceeded, this),
        MakeCallback (&ConfinitOnOff::ConnectionFailed, this));
    }
  m_cbrRateFailSafe = m_cbrRate;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();
}

void ConfinitOnOff::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("ConfinitOnOff found null socket to close in StopApplication");
    }
}

void ConfinitOnOff::CancelEvents ()
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
void ConfinitOnOff::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void ConfinitOnOff::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void ConfinitOnOff::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);
      Time nextTime (Seconds (bits /
                              static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet
      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &ConfinitOnOff::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void ConfinitOnOff::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &ConfinitOnOff::StartSending, this);
}

void ConfinitOnOff::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &ConfinitOnOff::StopSending, this);
}

void ConfinitOnOff::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> packet = Create<Packet> (m_pktSize);
  m_txTrace (packet);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();
}

void ConfinitOnOff::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void ConfinitOnOff::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

/* ========================================================================
 * Experiment
 * ========================================================================
 */

void ConfinitSimulation2(uint32_t nNodes) { //std::string simDate, uint32_t fixNodeNumber

	NS_LOG_INFO ("Confinit - Setting parameters...");
    double start = 0.0;
    double stop = 15.0;
    //std::string traceFile = "scratch/ostermalm_003_1_new.tr";
    //std::string scenarioSimFile;
    //std::string tracesFolder;
	//std::vector <int> healthNodes; // Store nodes with health interest
	//std::vector <int> attendingPriorities; // Store nodes attending priorities
	//std::vector <int> emergencyTimes; // Store times to nodes enters in emergency
    //std::vector<std::string> interestsTemp;
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
    Ptr<ConfinitOnOff> OnOffApp = CreateObject<ConfinitOnOff> ();
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




