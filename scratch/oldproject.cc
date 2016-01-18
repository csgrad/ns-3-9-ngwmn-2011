#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/global-routing-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestMeshScript");

int
main (int argc, char *argv[])
{
  int repeats = 10;
  int r;
  
  for(r = 0; r<repeats; r++)
  {

  SeedManager::SetRun (r);
  int		m_maxPackets = 100000;
  int       m_xSize = 6;
  int       m_ySize = 6;
  double    m_step = 100;
  double    m_randomStart = 0.1;
  double    m_totalTime = 500;
  double    m_packetInterval = 0.01;
  uint16_t  m_packetSize = 1024;
  uint16_t  m_port = 4000;
  uint32_t  m_nIfaces = 1;
  bool      m_chan = true;
  bool      m_pcap = false;
  std::string m_stack = "ns3::Dot11sStack";
  std::string m_root = "ff:ff:ff:ff:ff:ff";
  
  /// List of network nodes
  NodeContainer nodes;
  /// List of all mesh point devices
  NetDeviceContainer meshDevices;
  //Addresses of interfaces:
  Ipv4InterfaceContainer interfaces;
  // MeshHelper. Report is not static methods
  MeshHelper mesh;
    
  /*
   * Create m_ySize*m_xSize stations to form a grid topology
   */
  nodes.Create (m_ySize*m_xSize);
  // Configure YansWifiChannel
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  /*
   * Create mesh helper and set stack installer to it
   * Stack installer creates all needed protocols and install them to
   * mesh point device
   */
  mesh = MeshHelper::Default ();
  if (!Mac48Address (m_root.c_str ()).IsBroadcast ())
    {
      mesh.SetStackInstaller (m_stack, "Root", Mac48AddressValue (Mac48Address (m_root.c_str ())));
    }
  else
    {
      //If root is not set, we do not use "Root" attribute, because it
      //is specified only for 11s
      mesh.SetStackInstaller (m_stack);
    }
  if (m_chan)
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
    }
  else
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
    }
  mesh.SetMacType ("RandomStart", TimeValue (Seconds(m_randomStart)));
  // Set number of interfaces - default is single-interface mesh point
  mesh.SetNumberOfInterfaces (m_nIfaces);
  // Install protocols and return container if MeshPointDevices
  meshDevices = mesh.Install (wifiPhy, nodes);
  // Setup mobility - static grid topology
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (m_step),
                                 "DeltaY", DoubleValue (m_step),
                                 "GridWidth", UintegerValue (m_xSize),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  if (m_pcap)
    wifiPhy.EnablePcapAll (std::string ("mp-"));
  
  InternetStackHelper internetStack;
  internetStack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (meshDevices);
  
  /*
  UdpEchoServerHelper echoServer (m_port);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (m_xSize*m_ySize-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (m_totalTime));
  
  UdpEchoClientHelper echoClient (interfaces.GetAddress (0), m_port);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (m_maxPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (m_totalTime));  
  */
  
  UdpServerHelper server(m_port);
  ApplicationContainer apps = server.Install (nodes.Get (m_xSize*m_ySize-1));
  apps.Start (Seconds (200.0));
  apps.Stop (Seconds (m_totalTime));

  UdpClientHelper client (interfaces.GetAddress (m_xSize*m_ySize-1), m_port);
  client.SetAttribute ("MaxPackets", UintegerValue (m_maxPackets));
  client.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  client.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  apps = client.Install (nodes.Get (0));
  apps.Start (Seconds (210.0));
  apps.Stop (Seconds (m_totalTime));
  
  UdpClientHelper client1 (interfaces.GetAddress (m_xSize*m_ySize-1), m_port);
  client1.SetAttribute ("MaxPackets", UintegerValue (m_maxPackets));
  client1.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
  client1.SetAttribute ("Interval", TimeValue (Seconds (m_packetInterval)));
  apps = client1.Install (nodes.Get (m_xSize));
  apps.Start (Seconds (210.0));
  apps.Stop (Seconds (m_totalTime));
  
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
  
  //std::cout << "LOST: " << server.GetServer ()->GetLost () << std::endl;
  //std::cout << "REC: " << server.GetServer ()->GetReceived () << std::endl;
  Time totalDelay = server.GetServer ()->GetTotalDelay ();
  double delayPerPacket = (double)totalDelay.GetSeconds() / (double)server.GetServer ()->GetReceived ();
  double pdr = (double)server.GetServer ()->GetReceived () / ((double)server.GetServer ()->GetReceived () + (double)server.GetServer ()->GetLost ());
  std::cout << "PDR: " << pdr << "\t DELAY: " << delayPerPacket << "s" << std::endl;
  
  }//end of repeats
  return 0;
}
