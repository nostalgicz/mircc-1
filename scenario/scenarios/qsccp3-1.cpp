#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "../extensions/mircc-strategy.hpp"

namespace ns3
{

    int
    main(int argc, char *argv[])
    {
        CommandLine cmd;
        cmd.Parse(argc, argv);

        AnnotatedTopologyReader topologyReader("", 25);
        topologyReader.SetFileName("topologies/qsccp3-1.txt");
        topologyReader.Read();

        // Install NDN stack on all nodes
        ndn::StackHelper ndnHelper;
        ndnHelper.setPolicy("nfd::cs::lru");
        // ndnHelper.setCsSize(10000);
        ndnHelper.setCsSize(0);
        ndnHelper.InstallAll();

        // Choosing forwarding strategy
        ndn::StrategyChoiceHelper::InstallAll<nfd::fw::MIRCCStrategy>("/");

        // Installing global routing interface on all nodes
        ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
        ndnGlobalRoutingHelper.InstallAll();

        // Getting containers for the consumer/producer
        Ptr<Node> C1 = Names::Find<Node>("C1");
        Ptr<Node> C2 = Names::Find<Node>("C2");
        Ptr<Node> C3 = Names::Find<Node>("C3");

        Ptr<Node> P1 = Names::Find<Node>("P1");
        Ptr<Node> P2 = Names::Find<Node>("P2");
        Ptr<Node> P3 = Names::Find<Node>("P3");
        Ptr<Node> P4 = Names::Find<Node>("P4");

        ndn::AppHelper consumerHelper("ns3::ndn::ConsumerMIRCC");
        consumerHelper.SetAttribute("logtms",StringValue("0"));
        consumerHelper.SetAttribute("start_ms",StringValue("0"));
        consumerHelper.SetAttribute("end_ms",StringValue("200000"));

        // C1
        consumerHelper.SetPrefix("/A");
        consumerHelper.Install("C1");

        // C2
        consumerHelper.SetPrefix("/B");
        consumerHelper.SetAttribute("start_ms",StringValue("30000"));
        consumerHelper.SetAttribute("end_ms",StringValue("200000"));
        consumerHelper.Install("C2");

        // C3
        consumerHelper.SetPrefix("/C");
        consumerHelper.SetAttribute("start_ms",StringValue("60000"));
        consumerHelper.SetAttribute("end_ms",StringValue("200000"));
        consumerHelper.SetAttribute("greedy_ms",StringValue("150000"));
        consumerHelper.SetAttribute("greedy_rate",StringValue("6000000"));
        consumerHelper.Install("C3");


        ndn::AppHelper producerHelper("ns3::ndn::Producer");
        producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

        // Register /dst1 prefix with global routing controller and
        // install producer that will satisfy Interests in /dst1 namespace
        // P1
        ndnGlobalRoutingHelper.AddOrigins("/A", P1);
        ndnGlobalRoutingHelper.AddOrigins("/B", P2);
        ndnGlobalRoutingHelper.AddOrigins("/C", P3);
        ndnGlobalRoutingHelper.AddOrigins("/A", P4);
        ndnGlobalRoutingHelper.AddOrigins("/B", P4);
        ndnGlobalRoutingHelper.AddOrigins("/C", P4);

        producerHelper.SetPrefix("/A");
        producerHelper.Install(P1);
        producerHelper.Install(P4);

        producerHelper.SetPrefix("/B");
        producerHelper.Install(P2);
        producerHelper.Install(P4);
        
        producerHelper.SetPrefix("/C");
        producerHelper.Install(P3);
        producerHelper.Install(P4);

        // Calculate all possible routes and install FIBs
        ndn::GlobalRoutingHelper::CalculateRoutes();

        ndn::FibHelper::AddRoute("R2", "/A", "R3", 0);
        ndn::FibHelper::AddRoute("R2", "/B", "R3", 0);
        ndn::FibHelper::AddRoute("R2", "/C", "R3", 0);

        ndn::FibHelper::AddRoute("R2", "/A", "R4", 0);
        ndn::FibHelper::AddRoute("R2", "/B", "R4", 0);
        ndn::FibHelper::AddRoute("R2", "/C", "R4", 0);

        ndn::FibHelper::AddRoute("R2", "/A", "R5", 0);
        ndn::FibHelper::AddRoute("R2", "/B", "R5", 0);
        ndn::FibHelper::AddRoute("R2", "/C", "R5", 0);

        ndn::FibHelper::AddRoute("R4", "/A", "R3", 0);
        ndn::FibHelper::AddRoute("R4", "/B", "R3", 0);
        ndn::FibHelper::AddRoute("R4", "/C", "R3", 0);

        ndn::FibHelper::AddRoute("R4", "/A", "R5", 0);
        ndn::FibHelper::AddRoute("R4", "/B", "R5", 0);
        ndn::FibHelper::AddRoute("R4", "/C", "R5", 0);

        Simulator::Stop(Seconds(200.0));

        ndn::L3RateTracer::InstallAll("qsccp3-1_throughput.txt", Seconds(0.1));
        ndn::AppDelayTracer::InstallAll("qsccp3-1_delay.txt");
        L2RateTracer::InstallAll("qsccp3-1_drop.txt", Seconds(0.1));
        Simulator::Run();
        Simulator::Destroy();

        return 0;
    }

} // namespace ns3

int main(int argc, char *argv[])
{
    return ns3::main(argc, argv);
}
