/*
 * Experiment on WMN for NGWMN workshop paper
 * presented in Japan at NWWMN in November 2010
 * Jason Ernst
 * 
 * Requires updates to the following files:
 * src/devices/mesh/hwmp-protocol.cc
 *   - update for Mixed Bias scheduling
 * src/applications/udpclient-cc
 *   - update for reporting the total delay from successful deliveries
 */

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
#include <fstream>

/* variable parameters */
#define REPEATS 			10					/* how many repeats for statistical purposes 	*/
#define MAXPACKETS 			100000				/* max packets before simulation terminates		*/
#define TOTALTIME			100					/* max time (s) before simulation terminates	*/
#define PACKETINTERVAL		0.1					/* inter-arrival rate */
#define PACKETSIZE			1024				

#define XNODES				3					/* # of nodes in x-direction					*/
#define YNODES				3					/* # of nodes in y-direction					*/
#define	SEPARATIONDISTANCE	100					/* distance between nodes in meters 			*/

/* Logging options */
#define PCAPENABLED 		false

/* Fixed parameters */
#define RANDOMSTART			0.5					/* maximum random start delay for MAC address so that all nodes don't start at once */
#define PORT				4000
#define INTERFACES			1					/* number of wireless interfaces on each device	*/

using namespace ns3;

int main(int argc, char * argv[])
{
	int r;
	
	/* Required for mesh setup */
	bool m_chan = true;
	std::string m_root = "ff:ff:ff:ff:ff:ff";
	std::string m_stack = "ns3::Dot11sStack";
	
	std::cout << "Network size: " << XNODES*YNODES << " REPEATS: " << REPEATS << " RATE: " << PACKETINTERVAL << " TIME: " << TOTALTIME << std::endl;
	
	std::ofstream pdrfile;
	pdrfile.open ("/home/jernst/pdr.txt");
	std::ofstream delayfile;
	delayfile.open("/home/jernst/delay.txt");
	
	pdrfile << "Network size: " << XNODES*YNODES << " REPEATS: " << REPEATS << " RATE: " << PACKETINTERVAL << " TIME: " << TOTALTIME << std::endl;
	delayfile << "Network size: " << XNODES*YNODES << " REPEATS: " << REPEATS << " RATE: " << PACKETINTERVAL << " TIME: " << TOTALTIME << std::endl;
	
	/* Repeats the Simulation for statistical purposes */
	for (r = 0; r < REPEATS; r++)
	{
		/* Use the seed manager to make the experiment reproducable */
		SeedManager::SetRun(r);
		
		NodeContainer nodes;					/* List of all network nodes in the simulation */
		NetDeviceContainer meshRouterDevices; 	/* List of all Mesh Routers */
		Ipv4InterfaceContainer interfaces;		/* List of all interfaces */
		YansWifiPhyHelper wifiPhy;
		YansWifiChannelHelper wifiChannel;
		MeshHelper mesh;
		MobilityHelper mobility;
		
		nodes.Create(XNODES * YNODES);			/* Construct the topology of the network */
		
		/* Set the physical layer helper for the wireless mesh nodes */
		wifiPhy = YansWifiPhyHelper::Default ();
		wifiChannel = YansWifiChannelHelper::Default ();
		wifiPhy.SetChannel (wifiChannel.Create ()); 
		
		/* Set up the wireless mesh helper */
		mesh = MeshHelper::Default ();
		if (!Mac48Address (m_root.c_str ()).IsBroadcast ())
			mesh.SetStackInstaller (m_stack, "Root", Mac48AddressValue (Mac48Address (m_root.c_str ())));
		else
			mesh.SetStackInstaller (m_stack);
		if (m_chan)
			mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
		else
			mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
		
		mesh.SetMacType ("RandomStart", TimeValue (Seconds(RANDOMSTART)));
		mesh.SetNumberOfInterfaces (INTERFACES);
		meshRouterDevices = mesh.Install (wifiPhy, nodes);
		
		/* Set up mobility model */
		mobility.SetPositionAllocator (
			"ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (0.0),
			"DeltaX", DoubleValue (SEPARATIONDISTANCE),
			"DeltaY", DoubleValue (SEPARATIONDISTANCE),
			"GridWidth", UintegerValue (XNODES),
			"LayoutType", StringValue ("RowFirst")
		);
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install(nodes);
		
		if(PCAPENABLED)
			wifiPhy.EnablePcapAll (std::string ("mp-"));
		
		/* Set up the Internet stack, assign IP addresses */
		InternetStackHelper internetStack;
		internetStack.Install (nodes);
		Ipv4AddressHelper address;
		address.SetBase ("10.1.1.0", "255.255.255.0");
		interfaces = address.Assign (meshRouterDevices);
		
		/* Gateway */
		UdpServerHelper server(PORT);
		ApplicationContainer serverapp = server.Install (nodes.Get (0));
		serverapp.Start (Seconds (TOTALTIME / 2));
		serverapp.Stop (Seconds (TOTALTIME));
		
		/* Source MRs Note: do not start two sources at same time */
		UniformVariable u(0,5);
		UdpClientHelper client (interfaces.GetAddress (0), PORT);
		client.SetAttribute ("MaxPackets", UintegerValue (MAXPACKETS));
		client.SetAttribute ("PacketSize", UintegerValue (PACKETSIZE));
		client.SetAttribute ("Interval", TimeValue (Seconds (PACKETINTERVAL)));
		ApplicationContainer clientapp = client.Install (nodes.Get (XNODES * YNODES - 1));
		clientapp.Start (Seconds (1));
		clientapp.Stop (Seconds (TOTALTIME));
		
		UdpClientHelper client1 (interfaces.GetAddress (0), PORT);
		client1.SetAttribute ("MaxPackets", UintegerValue (MAXPACKETS));
		client1.SetAttribute ("PacketSize", UintegerValue (PACKETSIZE));
		client1.SetAttribute ("Interval", TimeValue (Seconds (PACKETINTERVAL)));
		ApplicationContainer client1app = client1.Install (nodes.Get (1));
		client1app.Start (Seconds (1));
		client1app.Stop (Seconds (TOTALTIME));
		
		Simulator::Stop (Seconds (TOTALTIME));
		Simulator::Run ();
		Simulator::Destroy ();
		
		/* Note: these statistics require a modified upd server so that total delay can be returned */
		Time totalDelay = server.GetServer ()->GetTotalDelay ();
		double delayPerPacket = (double)totalDelay.GetSeconds() / (double)server.GetServer ()->GetReceived ();
		double pdr = (double)server.GetServer ()->GetReceived () / ((double)server.GetServer ()->GetReceived () + (double)server.GetServer ()->GetLost ());
		std::cout << "PDR: " << pdr << " DELAY: " << delayPerPacket << "s" << std::endl;	
		pdrfile << pdr << std::endl;
		delayfile << delayPerPacket << std::endl;
	}
	pdrfile.close();
	delayfile.close();
	std::cout << '\a';
	
	return 0;
}
