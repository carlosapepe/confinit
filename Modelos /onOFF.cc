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
