#include "ns3/firefly_dynamic_clustering-module.h"

//#include "ns3/aodv-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/Constants.h"

#include <iostream>
#include <cmath>
#include <time.h>
//#include <stdio.h>

//#include "/opt/local/include/tinyxml.h"
//#include <tinyxml.h>

#define LIBXML_SAX1_ENABLED

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlstring.h>




using namespace ns3;

class FireflyExample
{

public:
	FireflyExample ();
	/// Configure script parameters, \return true on successful configuration
	bool Configure (int argc, char **argv);
	/// Run simulation
	void Run ();
	/// Report results
	void Report (std::ostream & os);

private:

	/// Number of nodes
	uint32_t size;
	/// Simulation time, seconds
	double totalTime;
	/// pcap packets?
	bool pcap;
	bool khopca_behavior;
	/// network dimensions
	double Xdim, Ydim;

	uint32_t simulation_random_seed;
	int type_of_reading;
	double beaconing_interval;
	double dynamic_beaconing_checking_interval;
	double clustering_thresh;
	int max_clustering_k;

	bool adaptative_clustering;

	NodeContainer nodes;
	NetDeviceContainer devices;
	Ipv4InterfaceContainer interfaces;


private:

	FireflyClusteringHelper fch;

	void CreateNodes ();
	void CreateDevices ();
	void CreateApplications ();
	void InstallInternetStack ();
	void GetReadingsFromXmlFile ();
	void GenerateRandomPositions ();
	void GetPositionsFromXmlFile ();

	//  static TiXmlDocument *doc;
};

int main (int argc, char **argv)
{
	FireflyExample test;

	if (!test.Configure (argc, argv))
		NS_FATAL_ERROR ("Configuration failed. Aborted.");

	test.Run ();
	test.Report (std::cout);
	return 0;
}

//-----------------------------------------------------------------------------
FireflyExample::FireflyExample () :
		  size (54),
		  totalTime (3600), /* one hour */
		  pcap (false),
		  khopca_behavior (false),
		  Xdim(600.0),
		  Ydim(600.0),
		  simulation_random_seed(time(NULL)),
		  type_of_reading((int)temperature),
		  beaconing_interval(30.0),
		  dynamic_beaconing_checking_interval(0.0),
		  clustering_thresh(4.0),
		  max_clustering_k((uint8_t)3),
		  adaptative_clustering(false)
{
}

bool compare_xml_name(const xmlChar* xc, const char *c) {
	return xmlStrcmp(xc, (const xmlChar *) c);
}

bool get_int_if_name(xmlNode* node_, std::string c, int *num) {
	bool r = compare_xml_name(node_->name,c.c_str());
	if (!r) {
		*num = atoi((const char*) xmlNodeGetContent(node_));
	}
	return r;
}

bool get_double_if_name(xmlNode* node_, std::string c, double *num) {
	bool r = compare_xml_name(node_->name,c.c_str());
	if (!r) {
		*num = atof((const char*) xmlNodeGetContent(node_));
	}
	return !r;
}

int get_int_attribute_of_name(xmlNode* node_, std::string c) {
	return atoi((const char*) xmlGetProp(node_, (const xmlChar *) c.c_str()));
}

double get_double_attribute_of_name(xmlNode* node_, std::string c) {
	return atof((const char*) xmlGetProp(node_, (const xmlChar *) c.c_str()));
}

void FireflyExample::GetReadingsFromXmlFile() {
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1;
	xmlNodePtr nodeLevel2;

	double time, reading = 0;
	int ndx;

	doc = xmlParseFile("/opt/full_data.xml");
	for(nodeLevel1 = doc->children; nodeLevel1 != NULL; nodeLevel1 = nodeLevel1->next) {
		for(nodeLevel2 = nodeLevel1->children; nodeLevel2 != NULL; nodeLevel2 = nodeLevel2->next) {
			if (nodeLevel2->type == XML_TEXT_NODE) continue;

			switch ((type_of_readings)type_of_reading) {
			case (temperature): {
				reading = get_double_attribute_of_name(nodeLevel2,"temp");
				break;
			}
			case (humidity): {
				reading = get_double_attribute_of_name(nodeLevel2,"hum");
				break;
			}
			case (light): {
				reading = get_double_attribute_of_name(nodeLevel2,"light");
				break;
			}
			case (energy): {
				reading = get_double_attribute_of_name(nodeLevel2,"volt");
				break;
			}
			}

			time = get_double_attribute_of_name(nodeLevel2,"time");
			ndx = get_int_attribute_of_name(nodeLevel2,"mote_id");
			//			std::cout << "read " << reading << " for " << ndx << " at " << time/10 << "\n";
			FireflyClusteringHelper::addReading(nodes.Get(ndx-1),reading,time /*/10*/); // FIXME this /10 should be a fucking parameter! if so..
		}
	}
	xmlFreeDoc(doc);
}

bool
FireflyExample::Configure (int argc, char **argv)
{
	// LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);

	//	std::cout << "___" << (int) max_clustering_k << "<<<\n"; std::flush(std::cout);

	CommandLine cmd;

	cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
	cmd.AddValue("khopca","Should use default KHOPCA behavior?",khopca_behavior);
	cmd.AddValue ("size", "Number of nodes.", size);
	cmd.AddValue ("time", "Simulation time, s.", totalTime);
	cmd.AddValue ("Xdim",  "X dimension", Xdim);
	cmd.AddValue ("Ydim",  "Y dimension", Ydim);
	//  light = 0,
	//  	temperature = 1,
	//  	humidity = 2,
	//  	energy = 3

	cmd.AddValue ("seed",  "Randomness seed", simulation_random_seed);
	cmd.AddValue ("readingtype",  "Type of reading (light=0,temperature=1,humidity=2,energy=3)", type_of_reading);
	cmd.AddValue ("interval",  "Firefly beaconing interval", beaconing_interval);
	cmd.AddValue ("cthresh",  "Clustering thresh (absolute value)", clustering_thresh);
	cmd.AddValue("maxk", "Maximum clustering distance K to cluster head", max_clustering_k);
	cmd.AddValue("adaptativeInterval", "Adaptative interval, should be dynamically adjusted?", adaptative_clustering);
	cmd.AddValue("adaptativeCheckInterval","Time between each checking of beaconing criteria", dynamic_beaconing_checking_interval);

	cmd.Parse (argc, argv);

	// if was not specified
	if (dynamic_beaconing_checking_interval < 0.001)
		dynamic_beaconing_checking_interval = beaconing_interval/3;

	//  std::cout << "it is " << (adaptative_clustering ? "true" : "false") << "\n";

	//  std::cout << "___" << (int) max_clustering_k << "<<<\n"; std::flush(std::cout);

	SeedManager::SetSeed (simulation_random_seed /*time (NULL)*/);

	return true;
}

void
FireflyExample::Run ()
{
	CreateNodes ();
	CreateDevices ();
	InstallInternetStack ();
	CreateApplications ();
	FireflyClustering::initializeReadingGUIDependencies((type_of_readings)type_of_reading);
	GetReadingsFromXmlFile();

	std::cout << "Starting simulation for " << totalTime << "s ...\n";

	Simulator::Stop (Seconds (totalTime));

	Simulator::Run ();
	Simulator::Destroy ();
}

void FireflyExample::CreateApplications() {
	fch.SetAttribute("Interval",TimeValue (Seconds (beaconing_interval)));
	fch.SetAttribute("ClusterThresh",DoubleValue(clustering_thresh));
	fch.SetAttribute("MaxClusteringK",UintegerValue(max_clustering_k));
	fch.SetAttribute("AdaptativeInterval",BooleanValue(adaptative_clustering));
	fch.SetAttribute("KHOPCARules",BooleanValue(khopca_behavior));
	fch.SetAttribute("DynamicCheckingInterval",TimeValue(Seconds(dynamic_beaconing_checking_interval)));

	fch.Install(nodes);
}

void
FireflyExample::Report (std::ostream &)
{
}

std::string doubleToString(double val)
{
	std::ostringstream out;
	out << val;
	return out.str();
}

void
FireflyExample::GenerateRandomPositions() {
	// Create static grid
	MobilityHelper mobility;

	ns3::RandomVariable Xgen = ns3::UniformVariable(0.0, Xdim);
	ns3::RandomVariable Ygen = ns3::UniformVariable(0.0, Ydim);

	Ptr<ListPositionAllocator> initialAlloc =
			CreateObject<ListPositionAllocator> ();
	for (uint32_t i = 0; i < nodes.GetN (); ++i) {
		initialAlloc->Add (Vector (Xgen.GetValue(), Ygen.GetValue(),0.));
	}
	mobility.SetPositionAllocator(initialAlloc);


	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (nodes);
}


void FireflyExample::GetPositionsFromXmlFile() {
	xmlDocPtr doc;
	xmlNodePtr nodeLevel1, nodeLevel2, nodeLevel3;
	double x,y;
	int ndx;

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> initialAlloc =
			CreateObject<ListPositionAllocator> ();

	// TODO: DO THESE FUCKING THINGS DOWN HERE ON OTHER FUCKING PLACE KRIGHT
	doc = xmlParseFile("/opt/mote_locs.xml");
	for(nodeLevel1 = doc->children; nodeLevel1 != NULL; nodeLevel1 = nodeLevel1->next) {
		for(nodeLevel2 = nodeLevel1->children; nodeLevel2 != NULL; nodeLevel2 = nodeLevel2->next) {
			if (nodeLevel2->type == XML_TEXT_NODE) continue;

			for (nodeLevel3 = nodeLevel2->children; nodeLevel3 != NULL; nodeLevel3 = nodeLevel3->next) {
				if (nodeLevel3->type == XML_TEXT_NODE) continue;
				get_int_if_name(nodeLevel3,"id",&ndx);
				get_double_if_name(nodeLevel3,"x",&x);
				get_double_if_name(nodeLevel3,"y",&y);
			}

			std::cout << "read " << x << "," << y << " for " << ndx << "\n";
			initialAlloc->Add (Vector (x*15, y*15, 0.));
		}
	}

	mobility.SetPositionAllocator(initialAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (nodes);

	xmlFreeDoc(doc);
}

void
FireflyExample::CreateNodes ()
{
	std::cout << "Creating " << (unsigned)size << " nodes\n";
	nodes.Create (size);
	// Name nodes
	for (uint32_t i = 0; i < size; ++i) {
		std::ostringstream os;
		os << "node-" << i;
		Names::Add (os.str (), nodes.Get (i));
	}
	//  GenerateRandomPositions();
	GetPositionsFromXmlFile();
}

void
FireflyExample::CreateDevices ()
{
	std::cout << "Creating devices\n";

	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifiMac.SetType ("ns3::AdhocWifiMac");
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());
	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
	devices = wifi.Install (wifiPhy, wifiMac, nodes);

	if (pcap)
	{
		wifiPhy.EnablePcapAll (std::string ("firefly"));
	}
}

void
FireflyExample::InstallInternetStack ()
{
	std::cout << "Installing internet stack\n";
	//  AodvHelper aodv;
	// you can configure AODV attributes here using aodv.Set(name, value)

	InternetStackHelper stack;
	//  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
	stack.Install (nodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	interfaces = address.Assign (devices);

}
