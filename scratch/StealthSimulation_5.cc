/* ========================================================================
 * Stealth Simulation
 *
 * Author:  Agnaldo de Souza Batista (asbatista@inf.ufpr.br)
 *
 * Date: July 02, 2018
 * Update(1): January 07, 2019
 * Update(2): January 24, 2019
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

NS_LOG_COMPONENT_DEFINE ("CenarioStealth_v1");

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
 * Statistics class
 * 14Nov18
 *
 * Class for data statistics and collect traces
 *
 * ========================================================================
 */

class Statistics
{
public:
				Statistics (std::string timeLog, std::string folderTraces);

    void 		EmergencyCallback (std::string path, Ipv4Address sourceIp,Ipv4Address destinyIP, std::string message, int attendingPriority);
    void 		BroadcastCallback (std::string path, Ipv4Address nodeIP, int nNeighbors, std::string neighbors);
    void 		ReceiverCallback (std::string path, Ipv4Address recvAdd, Ipv4Address fromAdd, int tagy, std::string msg1, int attendingPriority, int nInterests, std::string msg2);
    Address		GetNodeIpAddress (Ptr <Node> node);
	std::string GetNeighborList (Ptr <Node> node);

    uint32_t m_emergMsgSent; 		//!< Store number of emergency messages sent
    uint32_t m_emergMsgReceived; 	//!< Store number of emergency messages received
    uint32_t m_emergMsgACK; 		//!< Store number of emergency messages confirmed received
    uint32_t m_broadcastReceived;	//!< Store number of broadcast messages received
    uint32_t m_idReceived;			//!< Store number of identification messages received

    std::string m_timeLogFile; 		//!< Store the simulation start moment to append in logs files name
	std::string m_broadTracesFile;	//!< Store broadcast traces file name
	std::string m_emergTracesFile;	//!< Store emergency traces file name
	std::string m_recvTracesFile;	//!< Store received msg traces file name
	std::string m_folderToTraces;	//!< Store traces' for folder name

	std::ofstream m_broadFile;		//!< Store stream for broadcast traces file
	std::ofstream m_emergFile;		//!< Store stream for emergency traces file
	std::ofstream m_recvFile;		//!< Store stream for receiver msg traces file
};

/*
 * Statistics constructor
 * 131118
 *
 * Inputs: NIL
 *
 * Output: NIL
 */

Statistics::Statistics(std::string timeLog, std::string folderTraces)
{
	m_emergMsgSent = 0;
	m_emergMsgReceived = 0;
	m_emergMsgACK = 0;
	m_broadcastReceived = 0;
	m_idReceived = 0;
	m_timeLogFile = timeLog;
	m_folderToTraces = folderTraces;

	// Define and assign log files names
	ostringstream convert;
	convert << folderTraces.c_str() << "stealth_broadcast_traces_"
			<< timeLog.substr(0,timeLog.size()-2).c_str() << ".txt";
	m_broadTracesFile = convert.str();

	convert.str("");
	convert.clear();
	convert << folderTraces.c_str() << "stealth_emergency_traces_"
			<< timeLog.substr(0,timeLog.size()-2).c_str() << ".txt";
	m_emergTracesFile = convert.str();

	convert.str("");
	convert.clear();
	convert << folderTraces.c_str() << "stealth_received_traces_"
			<< timeLog.substr(0,timeLog.size()-2).c_str() << ".txt";
	m_recvTracesFile = convert.str();
}

/*
 * Statistics of StatusOn Application - Emergency situation
 * 131118
 *
 * Inputs:
 * path: String with the path to method
 * packet: Packet sent
 *
 * Output: NIL
 */


void
Statistics::EmergencyCallback (std::string path, Ipv4Address sourceIp, Ipv4Address destinyIP, std::string message, int attendingPriority)
{
	m_emergMsgSent++;
	double timeReceived = Simulator::Now().GetSeconds();

	// Save all emergency messages sent in only one file
	m_emergFile.open(m_emergTracesFile.c_str(), ios::out | ios::app);
	m_emergFile << timeReceived << "\t" << sourceIp << "\t" << destinyIP << "\t" << message.c_str() << "\t" << attendingPriority << endl;
	m_emergFile.close();

	ostringstream convert;
	convert << m_folderToTraces.c_str() << "emergency_messages_sent_"
			<< sourceIp << ".txt";

	// Save received messages individually by IP address
	m_emergFile.open(convert.str(), ios::out | ios::app);
	m_emergFile << timeReceived << "\t" << sourceIp << "\t" << destinyIP << "\t" << message.c_str() << "\t" << attendingPriority << endl;
	m_emergFile.close();
}

/*
 * Statistics of StealthOnOff Application - Searching neighbors
 * 151118
 *
 * Inputs:
 * path: String with the path to method
 * node: Node that sent broadcast message
 *
 * Output: NIL
 */

void
Statistics::BroadcastCallback (std::string path, Ipv4Address nodeIP, int nNeighbors, std::string neighbors)
{
	double timeReceived = Simulator::Now().GetSeconds();

	// Save all brodcasts in only one file
	m_broadFile.open(m_broadTracesFile.c_str(), ios::out | ios::app);
	m_broadFile << timeReceived << "\t" << nodeIP
			  << "\t" << nNeighbors << "\t" << neighbors << endl;
	m_broadFile.close();

	ostringstream convert;
	convert << m_folderToTraces.c_str() << "broadcasts_"
			<< nodeIP << ".txt";

	// Save broadcasts individually by IP address
	m_broadFile.open(convert.str(), ios::out | ios::app);
	m_broadFile << timeReceived << "\t" << nodeIP
				  << "\t" << nNeighbors << "\t" << neighbors << endl;
	m_broadFile.close();

}


/*
 * Statistics of StealthPacketSink Application
 * Receiving messages and answers
 * 151118
 *
 * Inputs:
 * path: String with the path to method
 * packet: Packet received
 *
 * Output: NIL
 */
void
Statistics::ReceiverCallback (std::string path, Ipv4Address recvAdd, Ipv4Address fromAdd,
							  int tagy, std::string msg1, int attendingPriority, int nInterests,
							  std::string msg2)
{
	double timeNow = Simulator::Now().GetSeconds();

	switch (tagy) {
	case 0: // Broadcast - Searching neighbors
			m_broadcastReceived++;
			break;
	case 1: // Unicast - Send Id to answer a broadcast
			m_idReceived++;
			break;
	case 2: // Unicast - Emergency msg
			m_emergMsgReceived++;
			break;
	case 3: // Unicast - Confirm reception of an emergency message
			m_emergMsgACK++;
			break;
	}

	// Save all messages received in only one file
	m_recvFile.open(m_recvTracesFile.c_str(), ios::out | ios::app);
	m_recvFile << timeNow << "\t" << fromAdd << "\t" << recvAdd << "\t"
			  << tagy << "\t" << msg1.c_str() << "\t" << attendingPriority << "\t"
			  << nInterests << "\t" << msg2.c_str() << endl;
	m_recvFile.close();

	ostringstream convert;
	convert << m_folderToTraces.c_str() << "messages_received_"
			<< recvAdd << ".txt";

	// Save received messages individually by IP address
	m_recvFile.open(convert.str(), ios::out | ios::app);
	m_recvFile << timeNow << "\t" << fromAdd << "\t" << recvAdd << "\t"
			  << tagy << "\t" << msg1.c_str() << "\t" << attendingPriority << "\t"
			  << nInterests << "\t" << msg2.c_str() << endl;
	m_recvFile.close();
}


/* ------------------------------------------------------------------------
 * End of Statistics class
 * ------------------------------------------------------------------------
 */


/* ========================================================================
 * StealthOnOff class
 *
 * Inherited from Application class
 *
 * Create and send packages
 *
 * ========================================================================
 */

class StealthOnOff : public Application
{
public:
    static TypeId 			GetTypeId (void);
							StealthOnOff ();
							~StealthOnOff ();
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
StealthOnOff::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StealthOnOff")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<StealthOnOff> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&StealthOnOff::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&StealthOnOff::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&StealthOnOff::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&StealthOnOff::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&StealthOnOff::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&StealthOnOff::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&StealthOnOff::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&StealthOnOff::m_txTrace),
                     "ns3::Packet::TracedCallback")
	.AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
				     MakeTraceSourceAccessor (&StealthOnOff::m_txTraceWithAddresses),
					 "ns3::Packet::TwoAddressTracedCallback")
	.AddTraceSource ("Broadcasts", "A broadcast is sent to search neighbors",
                     MakeTraceSourceAccessor (&StealthOnOff::m_nodeTrace),
					 "ns3::StealthOnOff::TracedCallback")
  ;
  return tid;
}

StealthOnOff::StealthOnOff ()
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

StealthOnOff::~StealthOnOff()
{
  NS_LOG_FUNCTION (this);
}

void
StealthOnOff::Setup(Address address, uint32_t protocolId)
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
StealthOnOff::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}

// Application Methods
void StealthOnOff::StartApplication () // Called at time specified by Start
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
        MakeCallback (&StealthOnOff::ConnectionSucceeded, this),
        MakeCallback (&StealthOnOff::ConnectionFailed, this));
    }
  m_cbrRateFailSafe = m_cbrRate;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();
}

void StealthOnOff::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  //else
  //  {
  //    NS_LOG_WARN ("StealthOnOff found null socket to close in StopApplication");
  //  }
}

void StealthOnOff::CancelEvents ()
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
void StealthOnOff::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void StealthOnOff::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();
  ScheduleStartEvent ();
}

// Private helpers
void StealthOnOff::ScheduleNextTx ()
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
      Time nextTime (Seconds (0.5));

      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime, &StealthOnOff::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void StealthOnOff::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &StealthOnOff::StartSending, this);
}

void StealthOnOff::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &StealthOnOff::StopSending, this);
}

void StealthOnOff::SendPacket ()
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
			       << ":Stop searching neighbors (StealthOnOff).");
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


void StealthOnOff::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void StealthOnOff::ConnectionFailed (Ptr<Socket> socket)
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
StealthOnOff::GetNodeIpAddress ()
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
StealthOnOff::PrintMyNeighborList ()
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

// Get node neighbor list
// 19Nov18

std::string
StealthOnOff::GetMyNeighborsList ()
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
 * End of StealthOnOff class
 * ------------------------------------------------------------------------
 */


/* ========================================================================
 * StatusOn class
 * 30/10/2018
 *
 * Inherited from Application class
 *
 * Create and send packages for emergency situations
 *
 * ========================================================================
 */

class StatusOn : public Application
{
public:
    static TypeId 			GetTypeId (void);
    						StatusOn ();
							~StatusOn ();
	void 					Setup(uint32_t protocolId, std::vector <std::string> competences);
    int64_t 				AssignStreams (int64_t stream);
    void					PrintMyNeighborList ();
    std::string				GetMyNeighborsList ();
    void 					AnnounceStopping ();

private: // inherited from OnOffApplication base class.

  void 						StartApplication (void);    // Called at time specified by Start
  void 						StopApplication (void);     // Called at time specified by Stop
  void 						CancelEvents ();
  void 						StartSending ();
  void 						StopSending ();
  void 						SendPacket ();
  Address					GetNodeIpAddress ();
  void 						StopAllApplications ();
  void 					    SendMessage(Address addressTo, std::string message, int attendingPriority, uint8_t tagy);
  bool 						HasHealthInterest (std::vector<std::string> interests);

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

  TracedCallback<Ptr<const Packet>> m_txTrace;  //!< Traced Callback: transmitted packets.
  TracedCallback<Ipv4Address, Ipv4Address, std::string, int > m_emergTrace;  //!< Traced Callback: emergency messages sent
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses; //!< Traced Callback: transmitted packets with addresses

  std::vector <std::string> m_competences;		//!< Store competences distributed
  Ipv4Address				m_nodeIP;			//!< Node's IPv4 Address
  std::string 				m_criticalInfo;		//!< Store node's critical information
  int		 				m_attendingPriority;//!< Store node's attending priority
  bool						m_sentData;			//!< Indicate critical data was sent

private:
  void 						ScheduleNextTx ();
  void 						ScheduleStartEvent ();
  void 						ScheduleStopEvent ();
  void 						ConnectionSucceeded (Ptr<Socket> socket);
  void 						ConnectionFailed (Ptr<Socket> socket);

};

TypeId
StatusOn::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StatusOn")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<StatusOn> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&StatusOn::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&StatusOn::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&StatusOn::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&StatusOn::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&StatusOn::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&StatusOn::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&StatusOn::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&StatusOn::m_txTrace),
                     "ns3::Packet::TracedCallback")
	.AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
				     MakeTraceSourceAccessor (&StatusOn::m_txTraceWithAddresses),
				     "ns3::Packet::TwoAddressTracedCallback")
	.AddTraceSource ("EmergNodes", "Node at emergency moment",
				     MakeTraceSourceAccessor (&StatusOn::m_emergTrace),
					 "ns3::StatusOn::TracedCallback")
  ;
  return tid;
}

StatusOn::StatusOn ()
{
  m_socket = 0;
  m_connected = false;
  m_residualBits = 0;
  m_lastStartTime = Seconds (0);
  m_totBytes = 6; // Total bytes sent on time
  m_maxBytes = 0;
  m_pktSize = 6;
  m_sentData = false;
  m_attendingPriority = 0;
  NS_LOG_FUNCTION (this);
}

StatusOn::~StatusOn()
{
  NS_LOG_FUNCTION (this);
}

void
StatusOn::Setup(uint32_t protocolId, std::vector <std::string> competences)
{
  NS_LOG_FUNCTION (this);
  //m_peer = address;
  m_node = GetNodeIpAddress ();
  m_nodeIP = InetSocketAddress::ConvertFrom(m_node).GetIpv4 ();
  m_competences = competences;

  // Choose protocol
  if (protocolId == 1) //1 Udp
	m_tid = ns3::UdpSocketFactory::GetTypeId();
  else // 2 tcp
	m_tid = ns3::TcpSocketFactory::GetTypeId();
}

int64_t
StatusOn::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}

// Application Methods
void
StatusOn::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  std::string competence;
  std::vector<std::string> neighborInterests;
  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
  	 		       << ":Entering in Emergency situation!");
  Ptr <Node> ThisNode = this->GetNode();
  bool healthInterest = false;
  //PrintMyNeighborList();

  // Change node's status to true. This will change
  // others application behavior from now on:
  // - Stop searching neighbors
  // - Stop send emergency message after first one sent
  ThisNode->SetAttribute("Status", BooleanValue(true));
  //uint32_t nApp = ThisNode->GetNApplications();

  // Announce stopping operation
  AnnounceStopping();

  // PrintMyNeighborList ();
  if (ThisNode->IsThereAnyNeighbor() == false) // node has no neighbors
  {
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  	           << ":Node has no neighbors to share critical information!");
	  StopAllApplications();
	  /*
	  double tempo = Simulator::Now ().GetSeconds ();
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  	 		       << ":Stop receiving new messages! (StealthPacketSink)");
	  Ptr<Application> appSink = ThisNode->GetApplication(0); // Sink application
	  appSink->SetStopTime(Seconds(tempo + 1.0));
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	 		       << ":Stop searching neighbors (StealthOnOff)");
	  Ptr<Application> appOnOff = ThisNode->GetApplication(1); // OnOff application
	  appOnOff->SetStopTime(Seconds(tempo + 1.0));
	  ThisNode->SetAttribute("Status", BooleanValue(false));
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	 		       << ":Stopping due to emergency (StatusOn)");
	  StopApplication (); // Stop application not sending emergency message
	  */
  }
  else // execute only if node has neighbors at start moment
  {
	  // Only nodes with health interest can access critical data
	  while (!healthInterest)
	  {
		  if (!ThisNode->IsThereAnyNeighbor())
			  break;
		  // Get node plus trust Ip address - 06Out18
		  m_peer = ThisNode->GetPlusTrustNeighbor (m_competences);
		  neighborInterests = ThisNode->GetNeighborInterests(m_peer);
		  if (HasHealthInterest(neighborInterests))
			  healthInterest = true;
		  else
			  // Remove selected neighbor from list
			  ThisNode->UnregisterNeighbor(m_peer);
	  }
	  if (!healthInterest)
	  {
		  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
		  	           << ":Node has no neighbors to share critical information!");
		  StopAllApplications();
	  }
	  else
	  {
		  competence = ThisNode->GetNeighborCompetence (m_peer);
		  m_criticalInfo = ThisNode->GetCriticalInfo (competence);// Get node critical information for this competence
		  m_attendingPriority = ThisNode->GetServicePriority();

		  std::cout << m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  		  			<< ":Higher score neighbor is " << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
						<< " (" << competence << ") and gets " << m_criticalInfo
						<< " with attending priority of " << m_attendingPriority << endl;

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
					  MakeCallback (&StatusOn::ConnectionSucceeded, this),
					  MakeCallback (&StatusOn::ConnectionFailed, this));
		  }
		  m_cbrRateFailSafe = m_cbrRate;

		  // Insure no pending event
		  CancelEvents ();
		  ScheduleStartEvent ();
	  }
  }
}

void
StatusOn::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
      m_socket->Close ();
}

void
StatusOn::CancelEvents ()
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
void
StatusOn::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void
StatusOn::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();
  ScheduleStartEvent ();
}

// Private helpers
void
StatusOn::ScheduleNextTx ()
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
      Time nextTime (Seconds (0.5));

      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime, &StatusOn::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void
StatusOn::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &StatusOn::StartSending, this);
}

void
StatusOn::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &StatusOn::StopSending, this);
}

void
StatusOn::SendPacket ()
{
  NS_LOG_FUNCTION (this);
  std::string competence;
  std::ostringstream msgToSend;
  std::vector<std::string> neighborInterests;
  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr <Node> ThisNode = this->GetNode();
  bool healthInterest = false;

  // Stop application only if critical data sent was already received
  if (ThisNode->GetServiceStatus())
  {
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
		       << ":Stopping due to emergency (StatusOn)");
	  StopApplication();
  }
  else if(m_sentData)
  {
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  	           << ":Node " << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
				   << " didn't confirm receiving critical data!");

	  PrintMyNeighborList();

	  //Remove selected neighbor from neighbor's list
	  ThisNode->UnregisterNeighbor(m_peer);
	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  	  	           << ":Node " << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
	  				   << " removed from neighbor's list.");
	  PrintMyNeighborList();

	  if (ThisNode->IsThereAnyNeighbor() == false) // node has no neighbors
	  {
		  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
		  	           << ":Node has no neighbors to share critical information!");
		  StopAllApplications();
	  }
	  else
	  {
		  // Only nodes with health interest can access critical data
		  while (!healthInterest)
		  {
			  if (!ThisNode->IsThereAnyNeighbor())
			  	  break;
			  //Choose another neighbor to send critical data
			  m_peer = ThisNode->GetPlusTrustNeighbor (m_competences); // Get plus trust neighbor
			  neighborInterests = ThisNode->GetNeighborInterests(m_peer);
			  if (HasHealthInterest(neighborInterests))
			  	  healthInterest = true;
			  else
				  // Remove selected neighbor from list
				  ThisNode->UnregisterNeighbor(m_peer);
		  }
		  if (!healthInterest)
		  {
			  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
			  	           << ":Node has no neighbors to share critical information!");
			  StopAllApplications();
		  }
		  else
		  {
			  healthInterest = false;
			  competence = ThisNode->GetNeighborCompetence (m_peer);
			  //Get critical data to the new neighbor
			  m_criticalInfo = ThisNode->GetCriticalInfo (competence);// Get node critical information for this competence
			  m_attendingPriority = ThisNode->GetServicePriority();

			  std::cout << m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
		  		  	<< ":Higher score neighbor is " << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
				    << " (" << competence << ") and gets " << m_criticalInfo
					<< " with attending priority " << endl;

			  //msgToSend << m_criticalInfo;
			  //msgToSend.append("-");
			  //msgToSend.append(m_attendingPriority);
			  //msgToSend.append("*"); //Add * to act as a msg limit
			  SendMessage(m_peer,m_criticalInfo.c_str(), m_attendingPriority ,2);
			  //SendMessage(m_peer,msgToSend.str(),m_attendingPriority,2);
			  //SendMessage(m_peer,msgToSend.c_str(),2);
		  }
	  }
  }
  else
  {
  msgToSend << m_criticalInfo << "-" << m_attendingPriority << "*";
  //msgToSend = m_criticalInfo;
  //msgToSend.append("*"); //Add * to act as a msg limit

  // Check if node is in emergency situation
  // If it is not, stop StatusOn
  // A node enters in emergency situation just
  // one time per simulation (10Nov18)
  if (ThisNode->GetStatus())
  {
	  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> (msgToSend.str().c_str()), msgToSend.str().size());

	  // Tag value 0: Broadcast
	  // 		   1: Unicast - Identification
	  //		   2: Unicast - Emergency message
	  //		   3: Unicast - Confirm receive Emergency message

	  MyTag tag; // Create a tag
	  tag.SetSimpleValue(2); // Set tag value
	  packet->AddPacketTag(tag); // Add tag to the packet

	  m_txTrace (packet);
	  m_socket->Send (packet); //Send just one message
	  m_totBytes += m_pktSize;

	  InetSocketAddress receiverAddress = InetSocketAddress::ConvertFrom (m_peer); // Receiver address

	  Address localAddress;
	  m_socket->GetSockName (localAddress);

	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  					  << ":Sent message - " << m_criticalInfo.c_str()
						  << " with attending priority " << m_attendingPriority
						  << " to "
	  					  << receiverAddress.GetIpv4 ()
	  					  << " - Tag " << (int)tag.GetSimpleValue());
	  m_txTraceWithAddresses (packet, localAddress, receiverAddress);

	  // Trace for statistics
	  m_emergTrace(m_nodeIP, receiverAddress.GetIpv4 (), m_criticalInfo,m_attendingPriority);
	  m_sentData = true;
  } // close last if

  }
}

void
StatusOn::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void StatusOn::ConnectionFailed (Ptr<Socket> socket)
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
StatusOn::GetNodeIpAddress ()
{
  Ptr <Node> PtrNode = this->GetNode();
  Ptr<Ipv4> ipv4 = PtrNode->GetObject<Ipv4> ();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
  Ipv4Address ipAddr = iaddr.GetLocal ();
  return (InetSocketAddress (ipAddr, 9));
}

// Print node neighbor list
// 07Nov18

void
StatusOn::PrintMyNeighborList ()
{
	std::vector<Address> neighborList;
	Ptr <Node> ThisNode = this->GetNode();
	if (ThisNode->IsThereAnyNeighbor() == false)
		std::cout << m_nodeIP << ": No neighbors!" << endl;
	else
	{
		neighborList = ThisNode->GetNeighborIpList();
		std::cout << m_nodeIP << ": My neighbors are: " << endl;
		for (uint8_t i = 0; i < neighborList.size(); i++)
		{
			std::cout << InetSocketAddress::ConvertFrom(neighborList[i]).GetIpv4 () << ":"
					  << InetSocketAddress::ConvertFrom(neighborList[i]).GetPort () << endl;
		}
	}
}

// Get node neighbor list
// 19Nov18

std::string
StatusOn::GetMyNeighborsList ()
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
		{
			neighbors << InetSocketAddress::ConvertFrom(neighborList[i]).GetIpv4 () << ", ";
		}
	}
	return (neighbors.str().substr(0,neighbors.str().size()-2));
}

/* Send broadcast message announcing stopping operation
 * 070119
 *
 * Inputs: NIL
 * Outputs: NIL
 */

void
StatusOn::AnnounceStopping ()
{
  std::string message = "Stopping!";
  Address addressTo = InetSocketAddress (Ipv4Address ("255.255.255.255"), 9);

  Ptr <Node> ThisNode = this->GetNode();
  //std::size_t endOfMsg;
  Ptr<Socket> socket = Socket::CreateSocket (GetNode (), m_tid);

  if (socket->Bind() == -1)
	  NS_FATAL_ERROR ("Failed to bind socket");

  socket->Connect(addressTo);
  socket->SetAllowBroadcast (true);
  socket->ShutdownRecv ();

  Ptr<Packet> packet;
  packet = Create<Packet> (reinterpret_cast<const uint8_t*> (message.c_str()), message.size());

  MyTag tag;
  tag.SetSimpleValue(4);
  packet->AddPacketTag(tag);
  socket->Send (packet);
  socket->Close ();

  //endOfMsg = message.find("*");
  //message = message.substr(0,endOfMsg).c_str();

  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	      << ":Announce stopping operation - Tag 4");
}

/* Stop all the applications installed
 * 090119
 *
 * Inputs: NIL
 * Outputs: NIL
 */

void
StatusOn::StopAllApplications ()
{
  NS_LOG_FUNCTION (this);
  Ptr <Node> ThisNode = this->GetNode();
  double tempo = Simulator::Now ().GetSeconds ();

  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
  	 		       << ":Stop receiving new messages! (StealthPacketSink)");
  Ptr<Application> appSink = ThisNode->GetApplication(0); // Sink application
  appSink->SetStopTime(Seconds(tempo + 1.0));

  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
 		       << ":Stop searching neighbors (StealthOnOff)");
  Ptr<Application> appOnOff = ThisNode->GetApplication(1); // OnOff application
  appOnOff->SetStopTime(Seconds(tempo + 1.0));

  ThisNode->SetAttribute("Status", BooleanValue(false));
  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
 		       << ":Stopping due to emergency (StatusOn)");

  StopApplication (); // Stop application not sending emergency message
}

/* Send messages
 * 140119
 *
 * Inputs:
 * address: Remote address
 * message: Text message to be sent
 * tagy: Tag to add
 */

void
StatusOn::SendMessage (Address addressTo, std::string message, int attendingPriority, uint8_t tagy)
{
  std::ostringstream msgToSend;
  msgToSend << message << "-" << attendingPriority <<"*";

  Ptr <Node> ThisNode = this->GetNode();
  // modify destiny address port to 9
  Address DestinyAddress (InetSocketAddress (Ipv4Address (InetSocketAddress::ConvertFrom(addressTo).GetIpv4 ()), 9));
  Ptr<Socket> socket = Socket::CreateSocket (GetNode (), m_tid);
  if (socket->Bind() == -1)
  	{
	  NS_FATAL_ERROR ("Failed to bind socket");
    }
  socket->Connect(DestinyAddress);
  Ptr<Packet> packet;

  packet = Create<Packet> (reinterpret_cast<const uint8_t*> (msgToSend.str().c_str()), msgToSend.str().size());
  MyTag tag;
  tag.SetSimpleValue(tagy);
  packet->AddPacketTag(tag);
  socket->Send (packet);
  socket->Close ();

  InetSocketAddress receiverAddress = InetSocketAddress::ConvertFrom (addressTo); // Receiver address

  Address localAddress;
  socket->GetSockName (localAddress);
  message = message.substr(0,message.length()-1).c_str();
  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
  					  << ":Sent message - " << message.c_str() << " to "
  					  << receiverAddress.GetIpv4 ()
  					  << " - Tag " << (int)tag.GetSimpleValue());
  m_txTraceWithAddresses (packet, localAddress, receiverAddress);

  // Trace for statistics
  m_emergTrace(m_nodeIP, receiverAddress.GetIpv4 (), message, attendingPriority);
}

/* Checks if there is health interest
 * 210119
 *
 * Inputs:
 * interests: A vector of strings with a node's interests
 *
 * Output:
 * true if has health interest, 0 otherwise
 */

bool
StatusOn::HasHealthInterest (std::vector<std::string> interests)
{
	bool HealthInterest = false;

    for (uint8_t i = 0; i < interests.size(); i++)
	  	 {
		  if (interests[i] == "health")
		  	  {
			  HealthInterest = true;
			  break;
		  	  }
	  	 }

	return HealthInterest;
}

/* ------------------------------------------------------------------------
 * End of StatusOn class
 * ------------------------------------------------------------------------
 */

/* ========================================================================
 * StealthPacketSink class
 *
 * Inherited from Application class
 *
 * Receive and analyze packets
 *
 * ========================================================================
 */

class StealthPacketSink : public Application
{
public:
	static TypeId GetTypeId (void);
	StealthPacketSink ();
	virtual ~StealthPacketSink ();
	void Setup(Address addressTo, uint32_t protocolId, std::vector <std::string> competences);

private:
	// inherited from Application base class.
	void 					StartApplication ();
	void 					StopApplication ();
	void 					PacketReceived(Ptr<Socket> socket);
	void 					ManipulateRead (Ptr<Socket> socket);
	void 					ManipulatePeerClose (Ptr<Socket> socket);
	void 					ManipulatePeerError (Ptr<Socket> socket);
	void 					ManipulateAccept (Ptr<Socket> s, const Address& from);
	void 					SendAnswer(Address addressTo, std::string message, uint8_t tagy);

	// added by me
	int 					GetNCommonInterests (std::vector<std::string> neighborInterests);
	bool 					HasHealthInterest (std::vector<std::string> interests);
	double 					EvaluateNeighborTrust (std::string competence, std::vector<std::string> interests);
	void 					CreateCompetenceTaxonomy ();
	int 					CompetenceDistanceToRoot (std::string competence);
	int 					CommonDistanceToRoot (std::string competence);
	int 					FindTaxonomyIndex (std::string competence);
	Address					GetNodeIpAddress ();
	void 					PrintMyNeighborList ();


	// inherited from Application base class.
    Address         		m_local;        //!< Local address to bind to
	uint64_t        		m_totalRx;      //!< Total bytes received
	TypeId          		m_tid;          //!< Protocol TypeId
	Ptr<Socket>     		m_socket;       //!< Listening socket
	std::list<Ptr<Socket>> 	m_socketList; 	//!< the accepted sockets

	// added by me
	std::string				m_myId;			//!< store competence + interests
    Address         		m_node;        	//!< Application node address
    std::vector <std::string> m_competences;//!< store competences distributed
    Ipv4Address				m_nodeIP;			//!< Node's IPv4 Address

    struct CompetenceHandlerEntry {
	    std::string competence;   //!< competence label
	    std::string father;       //!< father's label
	};

	// Typedef for competence taxonomy
	typedef std::vector<struct CompetenceHandlerEntry> CompetenceTaxonomy;

	CompetenceTaxonomy m_competencesTaxonomy;  //!< store taxonomy competence list

	TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace; //!< Traced Callback: received packets, source address.
    TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses; //!<Traced Callback: received packets, source and destionation address.
	TracedCallback<Ipv4Address, Ipv4Address, int, std::string, int, int, std::string> m_sinkTrace; //!< Traced Callback: received messages
};

TypeId
StealthPacketSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StealthPacketSink")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<StealthPacketSink> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (), MakeAddressAccessor (&StealthPacketSink::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&StealthPacketSink::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&StealthPacketSink::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
	.AddTraceSource ("RxWithAddresses", "A packet has been received",
		             MakeTraceSourceAccessor (&StealthPacketSink::m_rxTraceWithAddresses),
		    		 "ns3::Packet::TwoAddressTracedCallback")
	.AddTraceSource ("SinkTraces", "A message has been received",
					 MakeTraceSourceAccessor (&StealthPacketSink::m_sinkTrace),
					 "ns3::StealthPacketSink::TracedCallback")
  ;
  return tid;
}

StealthPacketSink::StealthPacketSink ()
{
	NS_LOG_FUNCTION (this);
	m_socket = 0;
	m_totalRx = 0;
}

StealthPacketSink::~StealthPacketSink (){
	NS_LOG_FUNCTION (this);
}

/*
 * \brief: Setup StealthPacketSink application in startup
 * \param: toAddress -> IP address to bind to
 * 		   protocolId -> Type of protocol to be used to
 * 						 1 -> UDP
 * 						 2 -> TCP
 */
void
StealthPacketSink::Setup (Address toAddress, uint32_t protocolId, std::vector <std::string> competences){
	NS_LOG_FUNCTION (this);
	m_node = GetNodeIpAddress ();
	m_nodeIP = InetSocketAddress::ConvertFrom(m_node).GetIpv4 ();
	m_local = toAddress;
	m_socket = 0;
	m_totalRx = 0;
	m_competences = competences;

	// Initialize Node's Id
	// A string (myId) for sending in response to neighbors search
	Ptr <Node> ThisNode = this->GetNode();
	std::vector<std::string> interests =  ThisNode->GetInterests();
	std::string competence = ThisNode->GetCompetence();
	m_myId = competence;
	for (std::vector<std::string>::iterator i = interests.begin ();
	     i != interests.end (); i++)
		{
		 m_myId.append(",");
	     m_myId.append(*i);
		}
	m_myId.append("*");
	// Choose protocol
	if (protocolId == 1) //1 Udp
		m_tid = ns3::UdpSocketFactory::GetTypeId();
	else // 2 tcp
		m_tid = ns3::TcpSocketFactory::GetTypeId();
}

/*
 * \brief: Stop StealthPacketSink application
 * \param: NIL
 */
void
StealthPacketSink::StopApplication (){
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

/*
 * \brief: Start StealthPacketSinkapplication
 * \param: NIL
 */
void
StealthPacketSink::StartApplication (){
  NS_LOG_FUNCTION (this);

  // Create competence taxonomy;
  CreateCompetenceTaxonomy();

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->SetAllowBroadcast(true);
      if (m_socket->Bind (m_local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      m_socket->Listen ();
      m_socket->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&StealthPacketSink::PacketReceived, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&StealthPacketSink::ManipulateAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&StealthPacketSink::ManipulatePeerClose, this),
    MakeCallback (&StealthPacketSink::ManipulatePeerError, this));
}


/*
 * \brief: Handle a packet received by the application
 * \param: socket - the receiving socket
 */
void
StealthPacketSink::PacketReceived(Ptr<Socket> socket) {

  Ptr<Packet> packet;
  Ptr <Node> ThisNode = this->GetNode();
  Address from;
  Address newFrom;
  Ipv4Address fromIP; // Store origin node IPv4 to display
  uint8_t i = 0;
  std::size_t found;
  std::string competenceReceived;
  std::string newBuffer;
  std::string criticalInfoRx;
  int attendingPriorityRx;
  std::string oldBuffer;
  //std::string interests;
  std::vector<std::string> interestsReceived;
  double nodeTrust = 0.0;
  std::size_t endOfMsg;

  // Tag value 0: Broadcast - Search neighbors
  // 		   1: Unicast - Send Identification
  //		   2: Unicast - Emergency message
  //		   3: Unicast - Confirm receive Emergency message
  //		   4: Broadcast - Announce stopping operation

  MyTag tag; // Create a tag

  NS_LOG_FUNCTION (this << socket);
  Address localAddress;
  std::stringstream interestsN;
  int nInterests = 0;

  while ((packet = socket->RecvFrom (from)))
    {
	  fromIP = InetSocketAddress::ConvertFrom(from).GetIpv4 ();

	  // Register all address with port = 9
      newFrom = InetSocketAddress (fromIP, 9);

      if (packet->GetSize () == 0)
        { //EOF
          break;
        }
      m_totalRx += packet->GetSize ();

      packet->PeekPacketTag(tag);
      std::vector<Address> neighborIpList;
      neighborIpList = ThisNode->GetNeighborIpList();

      if (InetSocketAddress::IsMatchingType (from))
        {
	  	  newBuffer = "";
	  	  newBuffer.clear();
    	  uint8_t *buffer = new uint8_t [packet->GetSize ()];
	  	  packet->CopyData (buffer, packet->GetSize ());
	  	  newBuffer = (char *)buffer;

	  	  switch(tag.GetSimpleValue())
	  	  {
	  	  case 0: // Actions when receive a broadcast message (searching neighbors' nodes)
	  	  	  /*	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  	  	   	   	   	   	   << ":Receive a search neighbors message from " << fromIP);
	  	  	    */SendAnswer(from, m_myId, 1); // Answers with node's identification
	  	  	      m_sinkTrace(m_nodeIP, fromIP, 0,"Hello!",0,0," ");
	  	 		  break;

	  	  case 1: // Actions when received an identification message
	  	  	  	  //NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  	  	  	  // 	  	  	   << ":Receive identification from " << fromIP
				  //			   << " to register in neighbor's list!");

	  	  	  	  // Recover neighbor's competence received
	  	  	  	  found = newBuffer.find(",");
	  	  	  	  endOfMsg = newBuffer.find("*");
	  	  	  	  newBuffer = newBuffer.substr(0,endOfMsg).c_str();

	  	  	  	  competenceReceived = newBuffer.substr(0,found);

	  	  	  	  // Recover  neighbor's interests received
	  	  	      while (found < newBuffer.size())
	  	  	      {
	  	  	    	  i = found+1;
	  	  	    	  found = newBuffer.find(",",i);
	  	  	    	  if (found != string::npos)
	  	  	    	  {
	  	  	    		  interestsReceived.push_back(newBuffer.substr(i,found-i));
	  	  	    		  interestsN << newBuffer.substr(i,found-i) << "\t";
	  	  	    		  nInterests++;
	  	  	    	  }
	  	  	    	  else
	  	  	    	  {
	  	  	    		  interestsReceived.push_back(newBuffer.substr(i,endOfMsg));
	  	  	    		  interestsN << newBuffer.substr(i,endOfMsg);
	  	  	    		  nInterests++;
	  	  	    		  break;
	  	  	    	  }
	  	  	      }

	  	  	      // Register all address with port = 9
	  	  	      //newFrom = InetSocketAddress (fromIP, 9);

	  	  	      // A node will be include in node's neighbors list if and only
	  	  	      // if they have any common interest
	  	  	      if (GetNCommonInterests(interestsReceived) == 0)
	  	  	    	  break;

	  	  	      if (HasHealthInterest (ThisNode->GetInterests()) == true)
	  	  	    	  nodeTrust = EvaluateNeighborTrust(competenceReceived, interestsReceived);

	  	  	      if (ThisNode->IsAlreadyNeighbor(newFrom) == true) // if is already a neighbor, turn Around True
	  	  	  	      ThisNode->TurnNeighborOn(newFrom); // Turn On an existent neighbor
	  	  	      else
	  	  	    	  ThisNode->RegisterNeighbor(newFrom, competenceReceived, interestsReceived, nodeTrust);
	  	  	      m_sinkTrace(m_nodeIP, fromIP, 1, competenceReceived.c_str(),0, nInterests, interestsN.str());
	  	  	      break;

	  	  case 2: // Actions when receive an emergency message
	  		  	  // Answer to sender only - 07Nov18

	  		  	  // Recover information received and attending priority in emergency situation
	  		  	  newBuffer = (char *)buffer;
	  		  	  oldBuffer = newBuffer.c_str();
	  		  	  endOfMsg = newBuffer.find("*");
	  		  	  newBuffer = newBuffer.substr(0,endOfMsg).c_str();
	  		  	  endOfMsg = newBuffer.find("-");
	  		  	  criticalInfoRx = newBuffer.substr(0,endOfMsg).c_str();
	  		  	  attendingPriorityRx = atoi(newBuffer.substr(endOfMsg + 1, endOfMsg+2).c_str());

	  		  	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	  		  			  	   << ":Received emergency message - " << criticalInfoRx << " - from " << fromIP
							   << " with attending priority " << attendingPriorityRx
							   << " - Tag "<< (int)tag.GetSimpleValue());

	  		  	  // Confirm receive emergency message - tag 3
	  		  	  SendAnswer(from,oldBuffer,3); // Pending including text content
	  		  	  m_sinkTrace(m_nodeIP, fromIP, 2, criticalInfoRx.c_str(),attendingPriorityRx,0," ");
	  	  	  	  break;

	  	  case 3: // Actions when receive a message that confirms receiving emergency message - 07Nov18

	  		  	  // Recover information received in emergency situation
	  			  newBuffer = (char *)buffer;

	  			  endOfMsg = newBuffer.find("*");
	  		  	  newBuffer = newBuffer.substr(0,endOfMsg).c_str();
	  		  	  endOfMsg = newBuffer.find("-");
	  		  	  criticalInfoRx = newBuffer.substr(0,endOfMsg).c_str();
	  		  	  attendingPriorityRx = atoi(newBuffer.substr(endOfMsg + 1, endOfMsg+2).c_str());

	  		  	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
  		  	  			   << ":Received a message confirming reception of an emergency message - "
						   << criticalInfoRx << " - from " << fromIP
						   << " with attending priority " << attendingPriorityRx
						   << " - Tag "<< (int)tag.GetSimpleValue());

  		  	  	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
  			  		      << ":Stop receiving new messages! (StealthPacketSink)");
  		  	  	  m_sinkTrace(m_nodeIP, fromIP, 3, criticalInfoRx.c_str(),attendingPriorityRx,0," ");

  		  	  	  // Node received a message confirming receiving critical data
  		  	  	  // Change service status to true
  		  	  	  ThisNode->SetAttribute("ServiceStatus", BooleanValue(true));

  			  	  StopApplication();
  		  	  	  break;

	  	  case 4: // Actions when receive stop announce message - 07Jan19
	  		  	  // Remove sender from neighbor list
	  		  	  ThisNode->UnregisterNeighbor(newFrom);
  		  	  	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
		  	  			   << ":Received Stop Announce from " << fromIP
						   << ". Neighbor removed from my neighbor list - Tag 4");
	  		  	  break;

	  	  default:// do nothing
	  		      break;
	  	  }
        }

      socket->GetSockName (localAddress);

      m_rxTrace (packet, from);
      m_rxTraceWithAddresses (packet, from, localAddress);
    }
}

void
StealthPacketSink::ManipulatePeerClose (Ptr<Socket> socket)
{
  std::cout << "STEALTH - ManipulatePeerClose" << std::endl;
  NS_LOG_FUNCTION (this << socket);
}

void
StealthPacketSink::ManipulatePeerError (Ptr<Socket> socket)
{
  std::cout << "STEALTH - ManipulatePeerError" << std::endl;
  NS_LOG_FUNCTION (this << socket);
}

void
StealthPacketSink::ManipulateAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  m_socketList.push_back (s);
}

/* Send Answer messages
 *
 * Inputs:
 * address: Remote address
 * message: Text message to be sent
 * tagy: Tag to add
 */

void
StealthPacketSink::SendAnswer (Address addressTo, std::string message, uint8_t tagy)
{
  Ptr <Node> ThisNode = this->GetNode();
  std::size_t endOfMsg;
  std::string criticalInfoRx;
  int attendingPriorityRx;

  // modify destiny address port to 9
  Address DestinyAddress (InetSocketAddress (Ipv4Address (InetSocketAddress::ConvertFrom(addressTo).GetIpv4 ()), 9));
  Ptr<Socket> socket = Socket::CreateSocket (GetNode (), m_tid);
  if (socket->Bind() == -1)
  	{
	  NS_FATAL_ERROR ("Failed to bind socket");
    }
  socket->Connect(DestinyAddress);
  Ptr<Packet> packet;

  packet = Create<Packet> (reinterpret_cast<const uint8_t*> (message.c_str()), message.size());
  MyTag tag;
  tag.SetSimpleValue(tagy);
  packet->AddPacketTag(tag);
  socket->Send (packet);
  socket->Close ();

  switch (tagy)
  {
  case 1: // Send message with node's identification
	      /*NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	       	   	   	   << ":Sent identification message to "
					   << InetSocketAddress::ConvertFrom(DestinyAddress).GetIpv4 ());
  	  	  */
	  	  //std::cout << "Id sent: " << message.c_str() << endl;
	      break;

  case 3: // Send message to confirm receiving an emergency message - 07Nov18

	  	  endOfMsg = message.find("*");
	  	  message = message.substr(0,endOfMsg).c_str();
	  	  message = message.substr(0,endOfMsg).c_str();
		  endOfMsg = message.find("-");
		  criticalInfoRx = message.substr(0,endOfMsg).c_str();
		  attendingPriorityRx = atoi(message.substr(endOfMsg + 1, endOfMsg+2).c_str());


	  	  NS_LOG_INFO (m_nodeIP << ":" << Simulator::Now ().GetSeconds ()
	      << ":Sent message to " << InetSocketAddress::ConvertFrom(DestinyAddress).GetIpv4 ()
		  << " confirming receiving its emergency message - " << criticalInfoRx.c_str()
		  << " with attending priority " << attendingPriorityRx
		  <<" -  Tag " << (int)tagy);

	  	  // If sent an emergency message, stop StealthPacketSink. Node will
	  	  // not receive any more message from now on. (10Nov18)
	  	  break;

  default://do nothing
	  	  break;
  }

}


/* Get how much common interests a neighbor node has with this one
 * 250918
 *
 * Inputs:
 * neighborInterests: A vector of strings with neighbor interests
 *
 * Output:
 * Number of common interests. 0 means no common interests.
 */

int
StealthPacketSink::GetNCommonInterests (std::vector<std::string> neighborInterests)
{
	int iCommon = 0;

	Ptr <Node> ThisNode = this->GetNode();
	std::vector<std::string> nodeInterests =  ThisNode->GetInterests();

    for (uint8_t i = 0; i < nodeInterests.size(); i++)
    	{
        	for (uint8_t n = 0; n < neighborInterests.size(); n++)
        		{
        		if (!nodeInterests[i].compare(neighborInterests[n]))
        			iCommon++;
        		}
		}
	return iCommon;
}


/* Checks if there is health interest
 * 250918
 *
 * Inputs:
 * interests: A vector of strings with a node's interests
 *
 * Output:
 * true if has health interest, 0 otherwise
 */

bool
StealthPacketSink::HasHealthInterest (std::vector<std::string> interests)
{
	bool HealthInterest = false;

    for (uint8_t i = 0; i < interests.size(); i++)
	  	 {
		  if (interests[i] == "health")
		  	  {
			  HealthInterest = true;
			  break;
		  	  }
	  	 }

	return HealthInterest;
}


/* Evaluate node's trust
 * 280918
 *
 * Inputs:
 * interests: A vector of strings with a node's interests
 *
 * Output:
 * trust value
 */

double
StealthPacketSink::EvaluateNeighborTrust (std::string competence, std::vector<std::string> interests)
 {
	double interestsTrust = 0.0;
	double competenceTrust = 0.0;

	// Evaluate trust related to node's competence
	int c1 = CompetenceDistanceToRoot ("doctor");
	int c2 = CompetenceDistanceToRoot (competence);
	int c3 = CommonDistanceToRoot (competence);

	competenceTrust = (2 * (double)c3)/((double)c1 + (double)c2);

	// Evaluate trust related to common interests

	int commonInterests = GetNCommonInterests (interests);
	int nodeInterests = this->GetNode()->GetInterests().size();

	interestsTrust = (double)commonInterests / (double)nodeInterests;

	// Sum interests and competence trusts

	return ((competenceTrust + interestsTrust) / 2);
 }


/* Create a taxonomy for competences
 * 280918
 *
 * Populate a vector with competence taxonomy
 */

void
StealthPacketSink::CreateCompetenceTaxonomy ()
{
	struct CompetenceHandlerEntry competence;
    competence.competence = "people";
    competence.father = "";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "health";
    competence.father = "people";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "other";
    competence.father = "people";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "medicine";
    competence.father = "health";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "nursing";
    competence.father = "health";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "doctor";
    competence.father = "medicine";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "nurse";
    competence.father = "nursing";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "pratical";
    competence.father = "nursing";
    m_competencesTaxonomy.push_back (competence);
    competence.competence = "caregiver";
    competence.father = "pratical";
    m_competencesTaxonomy.push_back (competence);
}

/* Calculate competence distance to taxonomy root
 * 290918
 *
 * Input: Competence to search
 *
 * Output: Competence distance to root
 */

int
StealthPacketSink::CompetenceDistanceToRoot (std::string competence)
{
	int distance = 0;
	std::string father;
	CompetenceTaxonomy::iterator i = m_competencesTaxonomy.begin ();
	// search for the competence label == competence
	for (; i != m_competencesTaxonomy.end (); i++)
		   if (i->competence == competence)
		 	  break;
	father = i->father;
	while (father != ""){
    	distance++;
    	for (i = m_competencesTaxonomy.begin ();
    		 i != m_competencesTaxonomy.end (); i++)
    			if (i->competence == father)
    			{
    			   father = i->father;
    			   break;
    			}
    }
    return distance;
}


/* Calculate common distance to taxonomy root of a competence
 * and doctor competence
 * 290918
 *
 * Input: Competence to evaluate
 *
 * Output: Common competence distance to root
 */

int
StealthPacketSink::CommonDistanceToRoot (std::string competence)
{
	int z = FindTaxonomyIndex (competence);
	int i = FindTaxonomyIndex ("doctor");

	while (m_competencesTaxonomy[i].father != "")
	{
		if (m_competencesTaxonomy[i].father == m_competencesTaxonomy[z].father)
			break;
		else
			if (m_competencesTaxonomy[z].father != "")
				z = FindTaxonomyIndex (m_competencesTaxonomy[z].father);
			else
			{
				z = FindTaxonomyIndex (competence);
				i = FindTaxonomyIndex (m_competencesTaxonomy[i].father);
			}
	}

	return FindTaxonomyIndex (m_competencesTaxonomy[i].father);
}


/* Search index to competence in taxonomy
 * 290918
 *
 * Input: Competence to search index
 *
 * Output: index to competence
 */

int
StealthPacketSink::FindTaxonomyIndex (std::string competence)
{
	uint8_t i = 0;
	// search for the competence label == competence
	for (; i < m_competencesTaxonomy.size (); i++)
		   if (m_competencesTaxonomy[i].competence == competence)
		 	  break;
	return int(i);
}


/* Get node NIC where application is installed
 * 300918
 *
 * Input: NIL
 *
 * Output: Interface address
 */

Address
StealthPacketSink::GetNodeIpAddress ()
{
  Ptr <Node> PtrNode = this->GetNode();
  Ptr<Ipv4> ipv4 = PtrNode->GetObject<Ipv4> ();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
  Ipv4Address ipAddr = iaddr.GetLocal ();
  return (InetSocketAddress (ipAddr, 9));
}

void
StealthPacketSink::PrintMyNeighborList ()
{
	std::vector<Address> neighborList;
	Ptr <Node> ThisNode = this->GetNode();
	neighborList = ThisNode->GetNeighborIpList();
	std::cout << m_nodeIP << " - My neighbors are: " << endl;
	for (uint8_t i = 0; i < neighborList.size(); i++)
	{
		std::cout << InetSocketAddress::ConvertFrom(neighborList[i]).GetIpv4 () << ":"
				  << InetSocketAddress::ConvertFrom(neighborList[i]).GetPort () << endl;
	}
}

/* ------------------------------------------------------------------------
 *  End of StealthPacketSink Class
 * ------------------------------------------------------------------------
 */

/*
 * Create a vector with random numbers
 * 071018
 *
 * Inputs
 * nNodes: Total number of nodes to use
 *
 * Output:
 * nVector: Vector with int nNodes unsorted. *
 */

std::vector<int>
ShuffleNodesIndexes(int nNodes, unsigned seed)
{
	std::vector<int> nVector;
	int i = 0;
	// set nNodes values to a vector
	for (; i < nNodes; ++i)
		 nVector.push_back(i); // 1 2 3 4 5 6 7 8 9

	// Choosing a seed
	//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);

    // shuffling vector values
    std::shuffle ( nVector.begin(), nVector.end(), rng);

	// print out content:
	//std::cout << "nVector contains:";
	//for (std::vector<int>::iterator it=nVector.begin(); it!=nVector.end(); ++it)
	//     std::cout << ' ' << *it;

	return nVector;
}


/*
 * Create a vector with random numbers
 * 071018
 *
 * Inputs
 * nNodes: Total number of nodes to use
 *
 * Output:
 * nVector: Vector with int nNodes unsorted. *
 */

std::vector<int>
ShuffleAttendingPriorities (int nNodes, unsigned seed)
{
	std::vector<int> nVector;
	int i = 0;
	int p = 1;
	// set nNodes attending priorities to a vector
	for (; i < nNodes; ++i)
	{
		 nVector.push_back(p);
		 p++;
		 if (p > 3)
			 p = 1;
	}
	// Choosing a seed
	//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);

    // shuffling vector values
    std::shuffle ( nVector.begin(), nVector.end(), rng);

	// print out content:
	//std::cout << "nVector contains: " << (int)nVector.size() << " - ";
	//for (std::vector<int>::iterator it=nVector.begin(); it!=nVector.end(); ++it)
	//     std::cout << ' ' << *it;

	return nVector;
}


/*
 * Event for test
 */

void event (int arg0, int arg1)
{
	std::cout << "STEALTH - Event called at " << arg0 << " seconds" << std::endl;
	arg0 = Simulator::Now().GetSeconds();
	Simulator::Schedule (Seconds (arg1), &event, arg0, arg1);
}


/*
 * Generate numbers for distribute interests
 * 121018
 *
 * Inputs
 * nInterests: Number of interests to be distributed to
 *
 * Output:
 * numbersTo: Numbers to be used to distribute interests
 *
 */

std::vector<int>
GenerateNForInterests(int nInterests, unsigned seed)
{
	std::vector<int> numbersTemp;
	std::vector<int> numbersTo;
	std::vector<int> indexTo;
	std::vector<int>::iterator d;
	int temp;
    bool erase = false;
    int i;

	// Generate numbers until a limit of digits
	for (i = 0; i < ((nInterests+1)*(pow (10,nInterests-1))); i++)
		if ( i > ((int)pow(10,nInterests - 1) - 1))
			numbersTemp.push_back(i);

	// Remove digits bigger than nInterests
	for (d = numbersTemp.begin (); d != numbersTemp.end ();)
	{
		temp = *d;
		for (int n = 0; n < nInterests;)
		{
			if ((temp % 10) < (nInterests+1))
			{
				temp /= 10;
				n++;
			}
			else
			{
				erase = true;
				numbersTemp.erase(d);
				break;
			}
		}
		if (erase == false)
			++d;
		else
			erase = false;
	}

	// Remove repeated digits together, except zeros
	for (d = numbersTemp.begin (); d != numbersTemp.end ();)
	{
		temp = *d;
		for (int n = 0; n < (nInterests-1);)
		{
			if ((temp % 10) != ((temp / 10) % 10) || ((temp % 10) == 0))
			{
				temp /= 10;
				n++;
			}
			else
			{
				erase = true;
				numbersTemp.erase(d);
				break;
			}
		}
		if (erase == false)
			++d;
		else
			erase = false;
	}

	// Remove repeated digits separated, except zeros
	for (d = numbersTemp.begin (); d != numbersTemp.end ();)
	{
		temp = *d;
		for (int n = 0; n < (nInterests-1);)
		{
			for (int i = 0; i < (nInterests-2);)
			{
				if ((temp % 10) != ((temp / ((int)pow(10,i+2))) % 10) || ((temp % 10) == 0))
					i++;
				else
				{
					erase = true;
					numbersTemp.erase(d);
					break;
				}
			}
			if (erase == true)
				break;
			else
			{
				temp /= 10;
				n++;
			}
		}
		if (erase == false)
			++d;
		else
			erase = false;
	}


	// Choose 100 to be used to as index to pick a number in the vector
	// numbersTemp. They'll be the 100 first ones in the list shuffled.

	indexTo = ShuffleNodesIndexes(1045, seed);

	for (int z = 0 ; z < 100 ; z++)
	{
			//std::cout << indexTo[z] << " ";
			numbersTo.push_back(numbersTemp[indexTo[z]]);
	}

	return numbersTo;
}


/*
 * Distribute interests in a vector of strings with nNodes positions. A
 * node can have at least one interest to all possible interests
 * 071018 - 171018
 *
 * Inputs
 * interests: A string with interests to be distributed, separated by
 * 			  commas
 * nNodes: Number of nodes which interests will be distributed to
 *
 * Output:
 * interestsTo: Vector of strings with interests to be distributed to nodes
 * 				separated by commas
 */

std::vector <std::vector<std::string>>
DistributeInterests (std::vector<std::string> interests, // interests to be distributed to
		   	   	   	 std::vector<int> intDistribution, // interests distribution
		             uint8_t nNodes, // number of node to assign interests
					 unsigned seed) // seed for randon numbers
{
	std::vector <std::vector<std::string>> interestsTo(nNodes,std::vector<std::string>(intDistribution.size()," ")); //store interests;
	std::vector<int> indexNodes;
	std::vector<int> indexInterests;

	int i = 0;
	int x = 0;
	int y = 0;
	int temp = 0;
	int zeros;
	int fourZeros = 0;
	int n = static_cast<int>(intDistribution.size());

	int onePosition = n;

	// Number of health interests to be removed
	// This keeps this interest about 20% of nNodes
	int healthToRemove = 50;

	indexInterests = GenerateNForInterests(n, seed);
	int nInt = static_cast<int>(indexInterests.size());

	// Initialize indexNodes to number of interests
	for (i = 0; i != nInt; i++)
		indexNodes.push_back(0);

	// Count zeros and check for health interests
	for (i = 0; i != nInt; i++)
	{
		temp = indexInterests[i];
		zeros = 0;
		for (x = 0; x != n; x++)
		{
			if((temp % 10) != 0) // don't compute zeros
			{
				indexNodes[(temp % 10)-1] += 1;
				if((temp % 10) == 1) // Correspond to health interest
					onePosition = x;
			}
			else
			{
				zeros += 1;
				if (zeros == (n - 1))
					fourZeros++;
			}
			temp /= 10;
		}

		// Remove health interests of healthToRemove indexes to balance the use of the mechanism
		// and creation of community of interests
		if ((zeros != (n-1)) && onePosition != n && healthToRemove != 0) // No 4 zeros inside
		{
			if (onePosition != (n-1))
			{
				// Remove health interest from this number
				// only first 50 numbers
				indexInterests[i] -= (int)(pow (10,onePosition)); //Remove health interest here
				healthToRemove--;
			}
			onePosition = n;
		}
	}

	// Assign strings of interest to interestsTo positions
	// Pass through all indexes created
	for (i = 0; i != nInt; i++)
	{
		temp = indexInterests[i];
		y = 0;

		//pass through all the interests to be distributed to
		for (x = 0; x != n; x++)
		{
			if ((temp % 10) != 0)
			{
				interestsTo[i][y] = interests[((temp % 10) -1)];
				y++;
			}
			temp /= 10;
		}
	}

	// list interestsTo vector
	for (i = 0; i != nInt; i++)
	{
		for (x = 0; x != n;)
		{
			if (interestsTo[i][x] != " ")
				++x;
			else
				interestsTo[i][x].erase();
		}
	}

	return interestsTo;
}


/*
 * Distribute competences in a vector of strings with nNodes positions. A
 * node can have only one competence through all possible competences.
 * 071018
 *
 * Inputs
 * competences: A string with competences to be distributed, separated by
 * 			  commas
 * nNodes: Number of nodes which competences will be distributed to
 *
 * Output:
 * interestsTo: Vector of strings with competences to be distributed to nodes
 */

std::vector<std::string>
DistributeCompetences (std::vector<std::string> competences, // competences to be distributed to
					   std::vector<int> compDistribution, // competences distribution
					   uint8_t nNodes, // number of node to assign competences
					   unsigned seed) // seed for shuffling numbers
{
	// store competences individually. Must be initialized
	// before use
	std::vector<std::string> competencesTo (nNodes,"");

	std::vector<int> indexCompetence;
	std::size_t z;
	int y = 0;

	// Shuffle a number of nNodes of integer to be used
	// as index to distribute competences
    indexCompetence = ShuffleNodesIndexes (nNodes, seed);

    // Choose the competence to be distributed to, one by one
	for (z = 0; z < compDistribution.size(); z++)
	{
		// Assign competence to a number of elements in compDistribution
		for (int x = 0; x < compDistribution[z]; x++)
			competencesTo[indexCompetence[x + y]] = competences[z];
		y += compDistribution[z];
	}
	return competencesTo;
}

/*
 * Checks nodes for the ones who have health interest
 * 291018
 *
 * Inputs
 * interestsTo: Vector of strings with competences to be distributed to nodes
 *
 * Output:
 * healthNodes: Vector of integers with index of nodes who have health interest
 */

std::vector <int>
GetsHealthInterestNodes(std::vector <std::vector<std::string>> interestsTo)
{
	std::vector <int> healthNodes;
	int i;
	int x;

    for (i = 0; i < 100; i++) // Pass through all nodes
    {
    	for (x = 0; x < 5; x++) // Pass through all the interests to be distributed to
    		if (interestsTo[i][x] == "health")
    		{
    			healthNodes.push_back(i);
    			break; //break to go to the next node
    		}
    }
    return healthNodes;
}


/*
 * Generate times for nodes with health interest enter in emergency status
 * 121118
 *
 * Inputs
 * numberOfNodes: Number of nodes with health interest
 *
 * Output:
 * emergencyTimes: Vector of integers with times to nodes with health interest
 */

std::vector <int>
GenerateEmergTimes(int numberOfNodes, unsigned seed)
{
	std::vector <int> emergencyTimes(numberOfNodes,0);
	int number;

	// Choosing a seed
	//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    //std::default_random_engine rng(seed);

    for (int i = 0; i < numberOfNodes;)
    {
    	number = rand() % 1000;
    	//if (number > 25 && number < 96) // to be used during tests
       	if (number > 25 && number < 896) // searching neighbors begins after 20s
    	{
    		emergencyTimes[i] = number;
    	   	i++;
     	}
    }
	return emergencyTimes;
}

/*
 * Checks if generated times for nodes with health interest enter in emergency
 * status have some repeated
 * 121118
 *
 * Inputs
 * emergencyTimes: Vector of integers with times to nodes with health interest
 *
 * Output:
 * repeated: true if has a time repeated, false otherwise.
 */

bool
EmergTimesRepeated(std::vector <int> emergencyTimes)
{
	bool repeated = false;
	int i;
	int x;
	for (i = 0; i < (int)emergencyTimes.size(); i++)
	{
		for (x = 1; x < (int)(emergencyTimes.size() - i); x++)
				if (emergencyTimes [i] == emergencyTimes [i + x])
				{
					repeated = true;
					break;
				}
		if (repeated)
			break;
	}
	return repeated;
}

/* ========================================================================
 * Create simulation scenario file
 * 20Nov18
 *
 * Input:
 * fileName: String with the name of the file to record
 * simulDate: String with simulation Date and Time
 * dataToSave: Strig with data to be saved
 *
 * Output: NIL
 * ========================================================================
 */
void
CreateSimScenarioFile(std::string fileName, std::string simulDate, std::string dataToSave)
{
	std::ofstream fileSimRec;
    fileSimRec.open (fileName.c_str ());
    fileSimRec << "**** Stealth scenario configuration file ****" << endl << endl;
    fileSimRec << "Date: " << simulDate.substr(0,8).c_str() << " - "
   			<< simulDate.substr(9,2).c_str() << ":"
			<< simulDate.substr(11,2).c_str() << "hs" << endl << endl;

    fileSimRec << dataToSave.c_str();

    // Close scenario simulation configuration file
    fileSimRec << "**** End of Stealth scenario configuration file ****" << endl;
    fileSimRec.close();
}

/* ========================================================================
 * Experiment
 * ========================================================================
 */

void StealthSimulation2(uint32_t nNodes, std::string simDate, uint32_t fixNodeNumber) {

	NS_LOG_INFO ("STEALTH - Setting parameters...");
    double start = 0.0;
    double stop = 900.0;
    std::string traceFile = "scratch/ostermalm_003_1_new.tr";
    std::string scenarioSimFile;
    std::string tracesFolder;
	std::vector <int> healthNodes; // Store nodes with health interest
	std::vector <int> attendingPriorities; // Store nodes attending priorities
	std::vector <int> emergencyTimes; // Store times to nodes enters in emergency
    std::vector<std::string> interestsTemp;
    std::vector <int> fixedNodesPatients = {52,69,70};
    std::vector <int> fixedNodesAttending = {63};

    NodeContainer::Iterator it;
    uint16_t port = 9;
    int i = 0;
    int x = 0;

    // Creating and defining seeds to be used
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);

    // Vector to store interests to be distributed to all nodes
    std::vector <std::vector<std::string>> interestsTo(100,std::vector<std::string>(5," "));
    std::vector<std::string> competencesTo (100," ");

    // Vector to store interests to be distributed to a fixed node
    std::vector<std::string> fixInterestPatients = {"health","tourism","music","movies","books"};
    std::string fixCompetencePatients = "other";

    std::vector<std::string> fixInterestAttending = {"health","tourism","music","movies","books"};
    std::string fixCompetenceAttending = "doctor";

    //----------------------------------------------------------------------------------
    // Create a folder for traces (stealth_traces) inside ns3 folder
	//----------------------------------------------------------------------------------

    tracesFolder = "stealth_traces/";
    errno = 0;
    int folder = mkdir(tracesFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (folder < 0 && errno != EEXIST)
    	std::cout << "Fail creating folder for traces!"<< endl;

    tracesFolder.append(simDate.substr(0,simDate.size()-2).c_str());
    tracesFolder.append("/");

    // Creates a folder for specific simulation
    folder = mkdir(tracesFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (folder == -1)
    	std::cout << "Fail creating sub folder for specific traces!" << folder << endl;

    ostringstream convert;
    convert << tracesFolder.c_str() << "stealth_simulation_scenario_"
    		<< simDate.substr(0,simDate.size()-2).c_str() << ".txt";
    scenarioSimFile = convert.str();

    // Create a string stream to store simulation scenario data
    std::stringstream fileSim;

    // Save start seed in file
    fileSim << "Start seed: " << seed << endl<< endl;

    // Competences to be distributed to all nodes
    // Only one per node
    std::vector<std::string> competences = {"doctor","nurse","caregiver","other"};
    std::vector<int> nCompetences = {10,15,20,55};

    // Save competences and its distribution in scenario configuration data file
    fileSim << "Competences: " << competences[0].c_str() << ", " << competences[1].c_str() << ", "
    		<< competences[2].c_str() << ", " << competences[3].c_str() << endl;

    fileSim << "Competences distribution: " << nCompetences[0] << ", " << nCompetences[1] << ", "
        		<< nCompetences[2] << ", " << nCompetences[3] << endl<< endl;

    // Interests to be distributed to all nodes
    // At least one interest per node and a maximum of all interests
    std::vector<std::string> interestss = {"health","tourism","music","movies","books"};
    std::vector<int> nInterests = {20,30,45,60,15};

    // Create and distribute interests and competences to assign to nodes
    competencesTo = DistributeCompetences (competences, nCompetences, 100, seed);
    interestsTo = DistributeInterests (interestss, nInterests, 100, seed);

    // If a node has to be fixed during simulation, change its
    // interests and competence to fixed
    if (fixNodeNumber != 0)
    {
    	//Setting attributes to Patient Nodes
    	for(i = 0; i < (int)fixNodeNumber; i++)
    	{
    		// Assign a competence to fixed nodes
    		competencesTo[fixedNodesPatients[i]] = fixCompetencePatients.c_str();
    		// Assign interests to fixed node
    		interestsTo[fixedNodesPatients[i]].clear();
    		for (x = 0; x < (int)fixInterestPatients.size(); x++)
    			interestsTo[fixedNodesPatients[i]].push_back(fixInterestPatients[x].c_str());
    	}
    	// Setting attributes to Attending Nodes
    	for(i = 0; i < (int)fixedNodesAttending.size(); i++)
    	{
    		// Assign a competence to fixed nodes
    		competencesTo[fixedNodesAttending[i]] = fixCompetenceAttending.c_str();
    		// Assign interests to fixed node
    		interestsTo[fixedNodesAttending[i]].clear();
    		for (x = 0; x < (int)fixInterestAttending.size(); x++)
    			interestsTo[fixedNodesAttending[i]].push_back(fixInterestAttending[x].c_str());
    	}
    }
    // Get the nodes with health interest to have status changed during simulation
    healthNodes = GetsHealthInterestNodes(interestsTo);

    // Remove attending nodes from health nodes' list, cause they cannot enter in
    // emergency situation
    std::vector<int>::iterator ith;
    ith = healthNodes.begin();
	for (i = 0 ; i != (int)healthNodes.size() ; i++)
	    {
		for (x = 0 ; x != (int)fixedNodesAttending.size() ; x++)
			    {
					if(healthNodes[i] == fixedNodesAttending[x])
						healthNodes.erase(ith + i);
						//std::cout << "STEALTH - Equal node found!!" <<
						//" - healthNodes[i]: " << healthNodes[i] <<
						//" - fixedNodesAttending[x]: " << fixedNodesAttending[x] << endl;
			    }
	    }

	// Create attending priorities' list
    attendingPriorities = ShuffleAttendingPriorities ((int) healthNodes.size(), seed);

    // Import node's mobility from the trace file
    // Necessary to use a helper from NS2
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);

	// Create nodes to be used during simulation
	NodeContainer Nodes;
	Nodes.Create (nNodes);

	// Install node's mobility in all nodes
	ns2.Install();

    // Initialize node's status and service status
    for (it = Nodes.Begin() ; it != Nodes.End() ; it++)
    {
    	(*it)->SetAttribute("Status", BooleanValue(false));
    	(*it)->SetAttribute("ServiceStatus", BooleanValue(false));
    }
    //----------------------------------------------------------------------------------
	// Assign competence and interests to nodes
	//----------------------------------------------------------------------------------

    NS_LOG_INFO ("STEALTH - Assign competence and interests ...");

    // Store competences and its distribution to save in a scenario file
    fileSim << "Competences assigned to nodes:" << endl;

    for (i = 0; i < 100; i++) // Pass through all nodes
    	fileSim << "Node[" << i << "]: " << competencesTo[i].c_str() << endl;
    fileSim << endl;

    fileSim << "Interests: " << interestss[0].c_str() << ", " << interestss[1].c_str() << ", "
        		<< interestss[2].c_str() << ", " << interestss[3].c_str() << ", "
    			<< interestss[4].c_str() << endl;

    fileSim << "Interests distribution: " << nInterests[0] << ", " << nInterests[1] << ", "
            		<< nInterests[2] << ", " << nInterests[3] << ", "
        			<< nInterests[4] << endl << endl;

    fileSim << "Interests assigned to nodes:" << endl;
    // ----------------------------------------------------------------------

    // Assign Competence and Interests for all nodes used in the simulations
    for (i = 0; i < 100; i++) // Pass through all nodes
    {
    	std::stringstream nodeInterests;
    	interestsTemp.clear();
    	nodeInterests << "Node[" << i << "]: ";
    	for (x = 0; x < (int)interestsTo[i].size(); x++) // Pass through all the interests to be distributed to
    	{
    		if (interestsTo[i][x].empty() == true)
    			break; //break if no more interests inside vector
    		else
    		{
    			nodeInterests << interestsTo[i][x].c_str() << ", ";
    			interestsTemp.push_back(interestsTo[i][x]);
    		}
        }

    	fileSim << nodeInterests.str().substr(0,nodeInterests.str().size() - 2) << endl;

    	// Assign competence to the node i
		Nodes.Get(i)->SetAttribute("Competence", StringValue(competencesTo[i]));
		// Assign interests to the node i
		Nodes.Get(i)->SetInterests(interestsTemp);
		interestsTemp.clear();
    }

    fileSim << endl;

	//----------------------------------------------------------------------------------
	// Set wifi network - Ad Hoc
	//----------------------------------------------------------------------------------

	NS_LOG_INFO ("STEALTH - Configuring wifi network (Ad Hoc)...");

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
	// Install applications
	//----------------------------------------------------------------------------------

	NS_LOG_INFO ("STEALTH - Install applications ...");

	//----------------------------------------------------------------------------------
	// Set Sink application
	//----------------------------------------------------------------------------------

    Address SinkBroadAddress (InetSocketAddress (Ipv4Address::GetAny (), port)); // SinkAddress for messages

    NS_LOG_INFO ("STEALTH - Install Sink application ...");

    //----------------------------------------------------------------------------------
	// Set Sink application
	//----------------------------------------------------------------------------------
    // Install Sink in all nodes

    for (it = Nodes.Begin() ; it != Nodes.End(); it++)
    {
    	Ptr<StealthPacketSink> SinkApp  = CreateObject<StealthPacketSink> ();
    	(*it)->AddApplication(SinkApp);

    	SinkApp->SetStartTime (Seconds (start));
    	SinkApp->SetStopTime (Seconds (stop));

    	SinkApp->Setup(SinkBroadAddress, 1, competences); // 1 -> UDP, 2 -> TCP
    }

    //----------------------------------------------------------------------------------
	// Set OnOff application
	//----------------------------------------------------------------------------------

    // Install OnOff in all nodes

    NS_LOG_INFO ("STEALTH - Install OnOff application ...");

    for (it = Nodes.Begin() ; it !=  Nodes.End() ; it++)
    {
        Ptr<StealthOnOff> OnOffApp = CreateObject<StealthOnOff> ();
        (*it)->AddApplication(OnOffApp);

        // Set to send to broadcast address
        OnOffApp->Setup(InetSocketAddress (Ipv4Address ("255.255.255.255"), 9),1); // 1 -> UDP, 2 -> TCP

        OnOffApp->SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        OnOffApp->SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=3]"));
        OnOffApp->SetAttribute ("DataRate",StringValue ("500kb/s"));
        OnOffApp->SetAttribute ("PacketSize", UintegerValue (6));

        OnOffApp->SetStartTime (Seconds (start));
        OnOffApp->SetStopTime (Seconds (stop));
        start+=0.2; // Avoid to start all the OnOff together
    }

   	Statistics statistics (simDate, tracesFolder.c_str());

    //----------------------------------------------------------------------------------
   	// Set StatusOn application
   	//----------------------------------------------------------------------------------

   	// Generate times to nodes enter in emergency
   	emergencyTimes = GenerateEmergTimes((int)healthNodes.size(), seed);

   	// Install StatusOn only in nodes with health interest
   	NS_LOG_INFO ("STEALTH - Install StatusOn application ...");

    it = Nodes.Begin();
    std::cout << "Healthy nodes (Time, Priority): ";
    fileSim << "Healthy nodes: ";
    std::stringstream nodesHealthy;
    std::stringstream nodesHealthyPriorities;

    bool isEqual;
    for (i = 0 ; i != (int)healthNodes.size() ; i++) // Install in all nodes with health interest
    {
    	isEqual = false;
    	Ptr<StatusOn> StatusOnApp = CreateObject<StatusOn> ();
        (*(it + healthNodes[i]))->AddApplication(StatusOnApp);
        (*(it + healthNodes[i]))->SetAttribute("ServicePriority", UintegerValue(attendingPriorities[i]));
        // Set to send UDP messages
        StatusOnApp->Setup(1, competences); // 1 -> UDP, 2 -> TCP

        StatusOnApp->SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        StatusOnApp->SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=3]"));
        StatusOnApp->SetAttribute ("DataRate",StringValue ("500kb/s"));
        StatusOnApp->SetAttribute ("PacketSize", UintegerValue (6));
        StatusOnApp->SetStopTime (Seconds (stop));
        if (fixNodeNumber != 0) // checks for fix node
        {
        	// Set status of fixed nodes' patients to TRUE
        	for (x = 0 ; x < (int)fixNodeNumber ; x++)
        		if (fixedNodesPatients[x] == healthNodes[i])
        			isEqual = true;
        	// Set status of fixed nodes' attending to FALSE

        }
        if (isEqual)
        {
        	StatusOnApp->SetStartTime (Seconds (485)); // fix node emergency time
            std::cout << healthNodes[i] << " ("<< 485 << ", " << attendingPriorities[i] << "), ";
        }
        else
        {
        	StatusOnApp->SetStartTime (Seconds (emergencyTimes[i]));
        	std::cout << healthNodes[i] << " ("<< emergencyTimes[i] << ", " << attendingPriorities[i] << "), ";
        }
    	nodesHealthy << healthNodes[i] << ", ";
    	nodesHealthyPriorities << attendingPriorities[i] << ", ";
    }
    std::cout << endl;

    fileSim << nodesHealthy.str().substr(0,nodesHealthy.str().size() - 2) << endl;
    fileSim << "Healthy nodes priorities: ";
    fileSim << nodesHealthyPriorities.str().substr(0,nodesHealthyPriorities.str().size() - 2) << endl;

    std::stringstream timesEmerg;
    fileSim << "Emergency times: ";
    for (i = 0 ; i != (int)emergencyTimes.size() ; i++)
    	timesEmerg << emergencyTimes[i] << ", ";
    fileSim << timesEmerg.str().substr(0,timesEmerg.str().size() - 2) << endl;

    if (fixNodeNumber != 0)
    {
       	fileSim << "Fixed Patient Node(s): ";
    	for (i = 0; i < (int)fixNodeNumber; i++)
    		fileSim << fixedNodesPatients[i] << " ";
    	fileSim << "- emergency time: 485s" << endl;
       	fileSim << "Fixed Attending Node(s): ";
    	for (i = 0; i < (int)fixedNodesAttending.size(); i++)
    		fileSim << fixedNodesAttending[i] << " ";
    	fileSim << endl;
    }
    else
       	fileSim << endl;

    //----------------------------------------------------------------------------------
    // Saving simulation scenario data
    //----------------------------------------------------------------------------------

    // Create a file and save simulation scenario data
    NS_LOG_INFO ("STEALTH - Saving simulation scenario data ...");

    CreateSimScenarioFile(scenarioSimFile.c_str (), simDate, fileSim.str());

    //----------------------------------------------------------------------------------
   	// Callback configuration
   	//----------------------------------------------------------------------------------
   	NS_LOG_INFO ("STEALTH - Configuring callbacks ...");

	// Callback Trace to Collect Emergency Messages in StatusOn Application
   	// Installed only in nodes with health interest 15Nov18
	it = Nodes.Begin();
	for (uint8_t i = 0 ; i != healthNodes.size() ; i++)
	{
		uint32_t nodeID = (*(it + healthNodes[i]))->GetId();
		std::ostringstream paramTest;
		paramTest << "/NodeList/" << (nodeID) << "/ApplicationList/*/$ns3::StatusOn/EmergNodes";
		Config::Connect (paramTest.str().c_str(), MakeCallback (&Statistics::EmergencyCallback, &statistics));
	}

	// Callback Trace to Collect data from StealthOnOff Application
   	// Installed only in nodes with health interest, for while 15Nov18
	for (uint8_t i = 0 ; i != healthNodes.size() ; i++)
	{
		uint32_t nodeID = (*(it + healthNodes[i]))->GetId();
		std::ostringstream paramTest;
		paramTest << "/NodeList/" << (nodeID) << "/ApplicationList/*/$ns3::StealthOnOff/Broadcasts";
		Config::Connect (paramTest.str().c_str(), MakeCallback (&Statistics::BroadcastCallback, &statistics));
	}

	// Callback Trace to Collect data from StealthPacketSink Application
	// Installed in all nodes 15Nov18
	for (it = Nodes.Begin(); it != Nodes.End(); it++)
	{
		uint32_t nodeID = (*it)->GetId();
		std::ostringstream paramTest;
		paramTest << "/NodeList/" << (nodeID) << "/ApplicationList/*/$ns3::StealthPacketSink/SinkTraces";
		Config::Connect (paramTest.str().c_str(), MakeCallback (&Statistics::ReceiverCallback, &statistics));
	}

	//----------------------------------------------------------------------------------
	// Enabling pcap
	//----------------------------------------------------------------------------------
	wifiPhy.EnablePcapAll("StealthSimulation",true);

	//----------------------------------------------------------------------------------
	// Animation Interface NetAnim
	//----------------------------------------------------------------------------------

	// (Insert code here!)

	//----------------------------------------------------------------------------------
	// Start / Stop simulation
	//----------------------------------------------------------------------------------

	NS_LOG_INFO ("STEALTH - Starting Simulation...");
	Simulator::Stop (Seconds (900));
	Simulator::Run ();
	Simulator::Destroy ();

	//----------------------------------------------------------------------------------
	// Tracing
	//----------------------------------------------------------------------------------

	std::cout << "----------------- Total data -----------------" << std::endl;
	std::cout << "Total broadcasts received: "
			  << statistics.m_broadcastReceived << std::endl;
	std::cout << "Total identification received: "
				  << statistics.m_idReceived << std::endl;
	std::cout << "Total emergency messages sent: "
			  << statistics.m_emergMsgSent << std::endl;
	std::cout << "Total emergency messages received: "
			  << statistics.m_emergMsgReceived << std::endl;
	std::cout << "Total emergency messages Confirmed: "
			  << statistics.m_emergMsgACK << std::endl;

	//----------------------------------------------------------------------------------
	// Metrics - calcs
	//----------------------------------------------------------------------------------

	// (Insert code here!)

}

/* ------------------------------------------------------------------------
 * End of Experiment
 * ------------------------------------------------------------------------
 */

/* ========================================================================
 * Get date and time of simulation start to be used in log files
 * 19Nov18
 *
 * Input: NIL
 *
 * Output:
 * string: A string with date and time
 * ========================================================================
 */
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

	NS_LOG_INFO ("STEALTH - Initializing...");

	CommandLine cmd;
	cmd.AddValue ("nNodes", "Number of node devices", nNodes);
	cmd.AddValue ("fixNode", "Number of nodes with fixed settings", fixNode);
	cmd.Parse (argc,argv);

    simTime = GetTimeOfSimulationStart();

    //SeedManager::SetRun(nRun); // update seed to n executions
    if (fixNode < 4 )
    	StealthSimulation2(nNodes, simTime, fixNode);
    else
    	std::cout << "STEALTH - Error: Maximum 3 fixed nodes!" << std::endl;

	NS_LOG_INFO ("STEALTH - Done!...");

	return 0;
}

/* ------------------------------------------------------------------------
 * End of Main
 * ------------------------------------------------------------------------
 */



