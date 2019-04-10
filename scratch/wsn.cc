// Network topology
//
//       n0   n1   n2   n3  
//       |    |    |    |
//       =================
//        WSN (802.15.4)
//
//Based on the examples wsn-ping6.cc, ping6.cc, example-ping-lr-wpan.cc and udp-echo.cc

#include "ns3/core-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/single-model-spectrum-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/applications-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/internet-stack-helper.h"
#include <ns3/ipv6-address-helper.h>
#include <ns3/sixlowpan-helper.h>
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include <ns3/spectrum-value.h>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WSN");

void ReceivePacket (Ptr<Socket> dest, Ptr<Socket> socket)
{
    //dest->Connect(remote);
    while (socket->Recv ())
    {
        NS_LOG_UNCOND ("Node " << socket->GetNode()->GetId() << " received one packet!");
        dest->Send (Create<Packet>());
    }
    //dest->Close();
}

void ReceiveDestiny (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Pacote chegou ao no 3");
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, 
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      NS_LOG_UNCOND("Node 0 sends packet number "<< pktCount);
      Simulator::Schedule (pktInterval, &GenerateTraffic, 
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int main()
{

    LogComponentEnable ("WSN", LOG_LEVEL_INFO);
    LogComponentEnable ("Socket", LOG_LEVEL_INFO);
    //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
    //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

	//Create 4 nodes, and a device for each one
	Ptr<Node> n0 = CreateObject<Node>();
	Ptr<Node> n1 = CreateObject<Node>();
	Ptr<Node> n2 = CreateObject<Node>();
	Ptr<Node> n3 = CreateObject<Node>();
	double txPower = 0;
	uint32_t channelNumber = 11;
	
	NodeContainer nodes (n0, n1, n2, n3);

	Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
	Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();
	Ptr<LrWpanNetDevice> dev2 = CreateObject<LrWpanNetDevice>();
	Ptr<LrWpanNetDevice> dev3 = CreateObject<LrWpanNetDevice>();

	//NetDeviceContainer lrwpanDevices = NetDeviceContainer(dev0, dev1, dev2, dev3);
	NetDeviceContainer lrwpanDevices;
	lrwpanDevices.Add(dev0);
	lrwpanDevices.Add(dev1);
	lrwpanDevices.Add(dev2);
	lrwpanDevices.Add(dev3);
	
	//std::cout<<lrwpanDevices.GetN();

	dev0->SetAddress(Mac16Address("00:01"));
	dev1->SetAddress(Mac16Address("00:02"));
	dev2->SetAddress(Mac16Address("00:03"));
	dev3->SetAddress(Mac16Address("00:04"));

	//Each device must be attached to same channel
	Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
	Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel>();
	Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
	channel->AddPropagationLossModel(propModel);
	channel->SetPropagationDelayModel(delayModel);

	dev0->SetChannel(channel);
	dev1->SetChannel(channel);
	dev2->SetChannel(channel);
	dev3->SetChannel(channel);
	
	//To complete configuration, a LrWpanNetDevice must be added to a node
	n0->AddDevice(dev0);
	n1->AddDevice(dev1);
	n2->AddDevice(dev2);
	n3->AddDevice(dev3);

	//Set distance between nodes
	Ptr<ConstantPositionMobilityModel> sender0 = CreateObject<ConstantPositionMobilityModel>();
	sender0->SetPosition(Vector(0,0,0));
	dev0->GetPhy()->SetMobility(sender0);
	Ptr<ConstantPositionMobilityModel> sender1 = CreateObject<ConstantPositionMobilityModel>();
	sender1->SetPosition(Vector(5,0,0));
	dev1->GetPhy()->SetMobility(sender1);
	Ptr<ConstantPositionMobilityModel> sender2 = CreateObject<ConstantPositionMobilityModel>();
	sender0->SetPosition(Vector(10,0,0));
	dev2->GetPhy()->SetMobility(sender2);
	Ptr<ConstantPositionMobilityModel> sender3 = CreateObject<ConstantPositionMobilityModel>();
	sender3->SetPosition(Vector(15,0,0));
	dev3->GetPhy()->SetMobility(sender3);

	//configure power of trasmission
	LrWpanSpectrumValueHelper svh;
	Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
	dev0->GetPhy ()->SetTxPowerSpectralDensity (psd);
	dev1->GetPhy ()->SetTxPowerSpectralDensity (psd);
	dev2->GetPhy ()->SetTxPowerSpectralDensity (psd);
	dev3->GetPhy ()->SetTxPowerSpectralDensity (psd);
	
	//Install protocol stack at the nodes
	InternetStackHelper internetv6;
	internetv6.Install (nodes);

	SixLowPanHelper sixlowpan;
  	NetDeviceContainer devices = sixlowpan.Install (lrwpanDevices); 
 
  	Ipv6AddressHelper ipv6;
  	ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  	Ipv6InterfaceContainer deviceInterfaces;
  	deviceInterfaces = ipv6.Assign (devices);
  	deviceInterfaces.SetForwarding (0, true);
    deviceInterfaces.SetDefaultRouteInAllNodes (0);
	
	Address address0, address1, address2, address3; //This is necessary for configure correctly the app layer.
    address0 = Address(deviceInterfaces.GetAddress (0,1));
	address1 = Address(deviceInterfaces.GetAddress (1,1));
	address2 = Address(deviceInterfaces.GetAddress (2,1));
	address3 = Address(deviceInterfaces.GetAddress (3,1));

	//Check the address of the nodes
	//std::cout<<address0<<std::endl;
	//std::cout<<address1<<std::endl;
	//std::cout<<address2<<std::endl;
	//std::cout<<address3<<std::endl;
	
	Ptr<Ipv6> ipv6_0 = n0->GetObject<Ipv6> ();
	Ptr<Ipv6> ipv6_1 = n1->GetObject<Ipv6> ();
	Ptr<Ipv6> ipv6_2 = n2->GetObject<Ipv6> ();
	
	//Add static routing
	NS_LOG_INFO ("Criando rotas estáticas.");
	Ipv6StaticRoutingHelper ipv6RoutingHelper;
	Ptr<Ipv6StaticRouting> staticRouting1 = ipv6RoutingHelper.GetStaticRouting (ipv6_0);
    staticRouting1->AddHostRouteTo (deviceInterfaces.GetAddress (3,1), deviceInterfaces.GetAddress (1,1), 1);
	Ptr<Ipv6StaticRouting> staticRouting2 = ipv6RoutingHelper.GetStaticRouting (ipv6_1);
    staticRouting2->AddHostRouteTo (deviceInterfaces.GetAddress (3,1), deviceInterfaces.GetAddress (2,1), 1);
	Ptr<Ipv6StaticRouting> staticRouting3 = ipv6RoutingHelper.GetStaticRouting (ipv6_2);
    staticRouting3->AddHostRouteTo (deviceInterfaces.GetAddress (3,1), deviceInterfaces.GetAddress (3,1), 1);
    
    
    //Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
    //ipv6RoutingHelper.PrintRoutingTableAt (Seconds (0.0), n0, routingStream);
    //ipv6RoutingHelper.PrintRoutingTableAt (Seconds (3.0), n1, routingStream);
    
    //Add app layer UDP
    NS_LOG_INFO ("Adicionando camada UDP.");
    uint16_t port = 4000;
    //uint32_t MaxPacketSize = 100; //bytes
    uint32_t packetSize = 100; //bytes
    //uint32_t maxPacket = 100; //100 packets
    Time interPacketInterval = Seconds (0.5);
    uint32_t numPackets = 100; //100 packets
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    
    //A ideia eh que o no receba e encaminhe o pacote para o proximo no
    //node 1
    Ptr<Socket> inter1Recv = Socket::CreateSocket (n1, tid);
    Ptr<Socket> inter1Sender = Socket::CreateSocket (n1, tid);
    Inet6SocketAddress local1 = Inet6SocketAddress (Ipv6Address::GetAny (), port); //escuta 4000
    Inet6SocketAddress remote1 = Inet6SocketAddress (deviceInterfaces.GetAddress (2,1), port+1);//envia 4001
    inter1Recv->Bind (local1);
    inter1Sender->Connect(remote1);
    inter1Recv->SetRecvCallback (MakeBoundCallback(&ReceivePacket, inter1Sender));
    
    //node 2
    Ptr<Socket> inter2Recv = Socket::CreateSocket (n2, tid);
    Ptr<Socket> inter2Sender = Socket::CreateSocket (n2, tid);
    Inet6SocketAddress local2 = Inet6SocketAddress (Ipv6Address::GetAny (), port+1);//escuta 4001
    Inet6SocketAddress remote2 = Inet6SocketAddress (deviceInterfaces.GetAddress (3,1), port);//envia 4000
    inter2Recv->Bind (local2);
    inter2Sender->Connect(remote2);
    inter2Recv->SetRecvCallback (MakeBoundCallback(&ReceivePacket, inter2Sender));
    
    //node 3
    Ptr<Socket> sink = Socket::CreateSocket (n3, tid);
    Inet6SocketAddress local3 = Inet6SocketAddress (Ipv6Address::GetAny (), port); //escuta 4000
    sink->Bind (local3);
    sink->SetRecvCallback (MakeCallback (&ReceiveDestiny));

    //node 0
    Ptr<Socket> source = Socket::CreateSocket (n0, tid);
    Inet6SocketAddress remote0 = Inet6SocketAddress (deviceInterfaces.GetAddress (1,1), port);
    source->SetAllowBroadcast (false);
    source->Connect (remote0);
    
    //AsciiTraceHelper ascii;
    // .EnableAsciiAll (ascii.CreateFileStream ("wsn.tr"));
    // .EnablePcapAll (std::string ("wsn"), true);
    
	//adicionar chamada de simulação
	
	NS_LOG_INFO ("Run Simulation.");
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (1.0), &GenerateTraffic, 
                                  source, packetSize, numPackets, interPacketInterval);

    Simulator::Stop (Seconds (100.0));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
    
    return 0;
}
	
   /*
    ApplicationContainer apps;
    UdpServerHelper server (port);
    apps = server.Install (n2);
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));
    
    ApplicationContainer apps1 = server.Install (n1);
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));
    
    ApplicationContainer apps2 = server.Install (n2);
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));
    
    UdpClientHelper client (deviceInterfaces.GetAddress (1,1), port);
    client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
    client.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
    apps = client.Install (n0);
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (10.0));
    */


