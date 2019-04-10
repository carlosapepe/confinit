/* ========================================================================
 * Experiment
 * ========================================================================
 */

void ConfinitSimulation2(uint32_t nNodes, std::string simDate, uint32_t fixNodeNumber) {

	NS_LOG_INFO ("CONFINIT - Setting parameters...");
    double start = 0.0;
    double stop = 900.0;
    std::string traceFile = "scratch/ostermalm_003_1_new.tr";
    std::string scenarioSimFile;
    std::string tracesFolder;
	std::vector <int> healthNodes; // Store nodes with health interest
	std::vector <int> attendingPriorities; // Store nodes attending priorities
	std::vector <int> emergencyTimes; // Store times to nodes enters in emergency
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

    // Vector to store interests to be distributed to all nodes
    //std::vector <std::vector<std::string>> interestsTo(100,std::vector<std::string>(5," "));
    //std::vector<std::string> competencesTo (100," ");

    // Vector to store interests to be distributed to a fixed node
    //std::vector<std::string> fixInterestPatients = {"health","tourism","music","movies","books"};
    //std::string fixCompetencePatients = "other";

    //std::vector<std::string> fixInterestAttending = {"health","tourism","music","movies","books"};
    //std::string fixCompetenceAttending = "doctor";

    //----------------------------------------------------------------------------------
    // Create a folder for traces (stealth_traces) inside ns3 folder
	//----------------------------------------------------------------------------------

    tracesFolder = "confinit_traces/";
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
    convert << tracesFolder.c_str() << "confinit_simulation_scenario_"
    		<< simDate.substr(0,simDate.size()-2).c_str() << ".txt";
    scenarioSimFile = convert.str();

    // Create a string stream to store simulation scenario data
    std::stringstream fileSim;

    // Save start seed in file
    fileSim << "Start seed: " << seed << endl<< endl;

    // Competences to be distributed to all nodes
    // Only one per node
    //std::vector<std::string> competences = {"doctor","nurse","caregiver","other"};
    //std::vector<int> nCompetences = {10,15,20,55};

    // Save competences and its distribution in scenario configuration data file
    //fileSim << "Competences: " << competences[0].c_str() << ", " << competences[1].c_str() << ", "
    		//<< competences[2].c_str() << ", " << competences[3].c_str() << endl;

    //fileSim << "Competences distribution: " << nCompetences[0] << ", " << nCompetences[1] << ", "
        		//<< nCompetences[2] << ", " << nCompetences[3] << endl<< endl;

    // Interests to be distributed to all nodes
    // At least one interest per node and a maximum of all interests
    //std::vector<std::string> interestss = {"health","tourism","music","movies","books"};
    //std::vector<int> nInterests = {20,30,45,60,15};

    // Create and distribute interests and competences to assign to nodes
    //competencesTo = DistributeCompetences (competences, nCompetences, 100, seed);
    //interestsTo = DistributeInterests (interestss, nInterests, 100, seed);

    // If a node has to be fixed during simulation, change its
    // interests and competence to fixed
    /*if (fixNodeNumber != 0)
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
    }*/
    // Get the nodes with health interest to have status changed during simulation
    //healthNodes = GetsHealthInterestNodes(interestsTo);

    // Remove attending nodes from health nodes' list, cause they cannot enter in
    // emergency situation
    /* std::vector<int>::iterator ith;
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
	    } */

	// Create attending priorities' list
    //attendingPriorities = ShuffleAttendingPriorities ((int) healthNodes.size(), seed);

    // Import node's mobility from the trace file
    // Necessary to use a helper from NS2
    //Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);

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
    /*
    NS_LOG_INFO ("CONFINIT - Assign competence and interests ...");

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
    */
    // ----------------------------------------------------------------------

    // Assign Competence and Interests for all nodes used in the simulations
    /*
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
    */
	//----------------------------------------------------------------------------------
	// Set wifi network - Ad Hoc
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
	// Install applications
	//----------------------------------------------------------------------------------

	NS_LOG_INFO ("CONFINIT - Install applications ...");

	//----------------------------------------------------------------------------------
	// Set Sink application
	//----------------------------------------------------------------------------------

    Address SinkBroadAddress (InetSocketAddress (Ipv4Address::GetAny (), port)); // SinkAddress for messages

    NS_LOG_INFO ("CONFINIT - Install Sink application ...");

    //----------------------------------------------------------------------------------
	// Set Sink application
	//----------------------------------------------------------------------------------
    // Install Sink in all nodes
    /*
    for (it = Nodes.Begin() ; it != Nodes.End(); it++)
    {
    	Ptr<StealthPacketSink> SinkApp  = CreateObject<StealthPacketSink> ();
    	(*it)->AddApplication(SinkApp);

    	SinkApp->SetStartTime (Seconds (start));
    	SinkApp->SetStopTime (Seconds (stop));

    	SinkApp->Setup(SinkBroadAddress, 1, competences); // 1 -> UDP, 2 -> TCP
    }
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
        start+=0.2; // Avoid to start all the OnOff together
    }

   	Statistics statistics (simDate, tracesFolder.c_str());

    //----------------------------------------------------------------------------------
   	// Set StatusOn application
   	//----------------------------------------------------------------------------------
    /*
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
    */
    //----------------------------------------------------------------------------------
    // Saving simulation scenario data
    //----------------------------------------------------------------------------------

    // Create a file and save simulation scenario data
    NS_LOG_INFO ("CONFINIT - Saving simulation scenario data ...");

    CreateSimScenarioFile(scenarioSimFile.c_str (), simDate, fileSim.str());

    //----------------------------------------------------------------------------------
   	// Callback configuration
   	//----------------------------------------------------------------------------------
   	NS_LOG_INFO ("CONFINIT - Configuring callbacks ...");
    /*
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
    */
	// Callback Trace to Collect data from StealthOnOff Application
   	// Installed only in nodes with health interest, for while 15Nov18
	for (uint8_t i = 0 ; i != healthNodes.size() ; i++)
	{
		uint32_t nodeID = (*(it + healthNodes[i]))->GetId();
		std::ostringstream paramTest;
		paramTest << "/NodeList/" << (nodeID) << "/ApplicationList/*/$ns3::StealthOnOff/Broadcasts";
		Config::Connect (paramTest.str().c_str(), MakeCallback (&Statistics::BroadcastCallback, &statistics));
	}
    /*
	// Callback Trace to Collect data from StealthPacketSink Application
	// Installed in all nodes 15Nov18
	for (it = Nodes.Begin(); it != Nodes.End(); it++)
	{
		uint32_t nodeID = (*it)->GetId();
		std::ostringstream paramTest;
		paramTest << "/NodeList/" << (nodeID) << "/ApplicationList/*/$ns3::StealthPacketSink/SinkTraces";
		Config::Connect (paramTest.str().c_str(), MakeCallback (&Statistics::ReceiverCallback, &statistics));
	}
    */
	//----------------------------------------------------------------------------------
	// Enabling pcap
	//----------------------------------------------------------------------------------
	wifiPhy.EnablePcapAll("ConfinitSimulation",true);

	//----------------------------------------------------------------------------------
	// Animation Interface NetAnim
	//----------------------------------------------------------------------------------

	// (Insert code here!)

	//----------------------------------------------------------------------------------
	// Start / Stop simulation
	//----------------------------------------------------------------------------------

	NS_LOG_INFO ("CONFINIT - Starting Simulation...");
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

	NS_LOG_INFO ("CONFINIT - Initializing...");

	CommandLine cmd;
	cmd.AddValue ("nNodes", "Number of node devices", nNodes);
	cmd.AddValue ("fixNode", "Number of nodes with fixed settings", fixNode);
	cmd.Parse (argc,argv);

    simTime = GetTimeOfSimulationStart();

    //SeedManager::SetRun(nRun); // update seed to n executions
    if (fixNode < 4 )
    	StealthSimulation2(nNodes, simTime, fixNode);
    else
    	std::cout << "CONFINIT - Error: Maximum 3 fixed nodes!" << std::endl;

	NS_LOG_INFO ("CONFINIT - Done!...");

	return 0;
}

/* ------------------------------------------------------------------------
 * End of Main
 * ------------------------------------------------------------------------
 */
