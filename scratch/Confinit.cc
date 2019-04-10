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
 #include <fstream>
 #include "ns3/core-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/sixlowpan-module.h"
 #include "ns3/lr-wpan-module.h"
 #include "ns3/internet-apps-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/applications-module.h"
 
using namespace ns3;
using namespace std;
 
 NS_LOG_COMPONENT_DEFINE ("Confinit");
 
 int main (int argc, char **argv)
 {
   bool verbose = false;
 
   CommandLine cmd;
   cmd.AddValue ("verbose", "turn on log components", verbose);
   cmd.Parse (argc, argv);
 
   if (verbose)
     {
       LogComponentEnable ("Ping6WsnExample", LOG_LEVEL_INFO);
       LogComponentEnable ("Ipv6EndPointDemux", LOG_LEVEL_ALL);
       LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
       LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
       LogComponentEnable ("Ipv6ListRouting", LOG_LEVEL_ALL);
       LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ALL);
       LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);
       LogComponentEnable ("Ping6Application", LOG_LEVEL_ALL);
       LogComponentEnable ("NdiscCache", LOG_LEVEL_ALL);
       LogComponentEnable ("SixLowPanNetDevice", LOG_LEVEL_ALL);
     }
 
   NS_LOG_INFO ("Create nodes.");
   NodeContainer nodes;
   nodes.Create (4);

   // Set seed for random numbers
   SeedManager::SetSeed (167);
 
   // Install mobility
   MobilityHelper mobility;
   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0),         //Posição no eixo X
                                  "MinY", DoubleValue (10),        //Posição no exixo Y
                                  "DeltaX", DoubleValue (30),      //Espaço entre os nos vertical
                                  "DeltaY", DoubleValue (30),      //Espaço entre os nos horizontal
                                  "GridWidth", UintegerValue (10), //linha vertical 
                                  "LayoutType", StringValue ("RowFirst"));
   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility.Install (nodes);
   
   OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
   onoff.SetConstantRate (DataRate (60000000));
   onoff.SetAttribute ("PacketSize", UintegerValue (2000));

   ApplicationContainer apps = onoff.Install (nodes.Get (0));
   apps.Start (Seconds (0.5));
   apps.Stop (Seconds (250.0));
  
   NS_LOG_INFO ("Create channels.");
   LrWpanHelper lrWpanHelper;
   // Add and install the LrWpanNetDevice for each node
   // lrWpanHelper.EnableLogComponents();
   NetDeviceContainer devContainer = lrWpanHelper.Install(nodes);
   lrWpanHelper.AssociateToPan (devContainer, 10);
 
   std::cout << "Created " << devContainer.GetN() << " devices" << std::endl;
   std::cout << "There are " << nodes.GetN() << " nodes" << std::endl;
 
   /* Install IPv4/IPv6 stack */
   NS_LOG_INFO ("Install Internet stack.");
   InternetStackHelper internetv6;
   internetv6.SetIpv4StackInstall (false);
   internetv6.Install (nodes);
 
   // Install 6LowPan layer
   NS_LOG_INFO ("Install 6LoWPAN.");
   SixLowPanHelper sixlowpan;
   NetDeviceContainer six1 = sixlowpan.Install (devContainer);
 
   NS_LOG_INFO ("Assign addresses.");
   Ipv6AddressHelper ipv6;
   ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
   Ipv6InterfaceContainer i = ipv6.Assign (six1);
 
   NS_LOG_INFO ("Create Applications.");
 
    /* Create a Ping6 application to send ICMPv6 echo request from node zero to
    * all-nodes (ff02::1);
    */
    
   uint32_t packetSize = 1024;
   uint32_t maxPacketCount = 2;
   Time interPacketInterval = Seconds (1.);
   Ping6Helper ping6;
 
   /*
   ping6.SetLocal (i.GetAddress (0));
   ping6.SetRemote (i.GetAddress ());
   */
   /*
   ping6.SetLocal (i1.GetAddress (0, 1));
   ping6.SetRemote (i2.GetAddress (1, 1));
   */
   ping6.SetIfIndex (i.GetInterfaceIndex (0));
   ping6.SetRemote (Ipv6Address::GetAllNodesMulticast ());
   
  /////////////////////////////////////////////////////////////
 
   ping6.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   ping6.SetAttribute ("Interval", TimeValue (interPacketInterval));
   ping6.SetAttribute ("PacketSize", UintegerValue (packetSize));
   ApplicationContainer apps = ping6.Install (nodes.Get (0));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (100));
 
   
 
   AsciiTraceHelper ascii;
   lrWpanHelper.EnableAsciiAll (ascii.CreateFileStream ("ConfinitSimulation.tr"));
   lrWpanHelper.EnablePcapAll (std::string ("ConfinitSimulation"), true);
 
   
  //----------------------------------------------------------------------------------
 // Start / Stop simulation
//----------------------------------------------------------------------------------

	NS_LOG_INFO ("CONFINIT - Starting Simulation...");
	Simulator::Stop (Seconds (100));
	Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
   
 }
