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
