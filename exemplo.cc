#include "ns3/propagation-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"


#include "ns3/applications-module.h" 
#include "ns3/ipv4-global-routing-helper.h" 

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>



NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");

using namespace ns3;


int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double rss = -80;  // -dBm, hodnota -80 do premennej rss
  bool verbose = false;
  
  
   // Set up some default values for the simulation.
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1024));////ake veľké zateženie by malo byť paketa, posiela jeden 1024b paket
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("20kb/s"));
  
  // serverNode, clientNodes 
  NS_LOG_INFO ("Create nodes."); 
  NodeContainer serverNode;
  serverNode.Create (1); 
  NodeContainer clientNodes; 
  clientNodes.Create (4); 
  NodeContainer allNodes = NodeContainer (serverNode, clientNodes);

  //.. The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  
   //wifiPhy
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
   // This is one parameter that matters when using FixedRssLossModel
   // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (0) ); 
   // ns-3 supports RadioTap and Prism tracing extensions for 802.11b,... rozširené trasovanie pre 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 


  // wifiChannel
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode, devices na všetky uzla
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, allNodes);

  //.. Note that with FixedRssLossModel, the positions below are not 
  // used for received signal strength. 
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  positionAlloc->Add (Vector (-5.0, 0.0, 0.0));
  positionAlloc->Add (Vector (0.0, 5.0, 0.0));
  positionAlloc->Add (Vector (0.0, -5.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);
   
     //internet
  InternetStackHelper internet;
  internet.Install (allNodes);
 
    //ipv4, i
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  //TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  
  // Create a packet sink on the star "hub" to receive these packets
  uint16_t port = 50000; //do premennej port pridám hodnotu 50000
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
  
  ApplicationContainer sinkApp = sinkHelper.Install (serverNode);
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (3.0)); //pridaj....

  // Create the OnOff applications to send UDP to the server
  OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  //normally wouldn't need a loop here but the server IP address is different
  //on each p2p subnet
  ApplicationContainer clientApps;
  for(uint32_t j=0; j<clientNodes.GetN (); ++j) //men na j....
    {
      AddressValue remoteAddress (InetSocketAddress (i.GetAddress (0), port)); 
      clientHelper.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (clientHelper.Install (clientNodes.Get (j)));
    }
  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (3.0));  //pridaj...                           
  
  //Naplníme smerovacie tabuľky
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
   //configure tracing
  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-moje.tr")); //ASCII .tc výstupného súboru simulátora
  wifiPhy.EnablePcapAll ("wifi-simple-adhoc-moje"); //Príklad výstupného súboru s príponou .pcap
  
  
  AnimationInterface anim ("wifi-simple-adhoc-moje.xml");
  
  
  // Run simulation for 10 seconds
  
  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();
  
  return 0;
}
