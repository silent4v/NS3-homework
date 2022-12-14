#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/enum.h"
#include "ns3/error-model.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"
#include "ns3/udp-header.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#define FIRST false

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpComparison");

using NodeIdType = uint32_t;

static std::map<NodeIdType, bool> firstCwnd;                           //!< First congestion window.
static std::map<NodeIdType, bool> firstSshThr;                         //!< First SlowStart threshold.
static std::map<NodeIdType, bool> firstRtt;                            //!< First RTT.
static std::map<NodeIdType, bool> firstRto;                            //!< First RTO.
static std::map<NodeIdType, Ptr<OutputStreamWrapper>> cWndStream;      //!< Congstion window outut stream.
static std::map<NodeIdType, Ptr<OutputStreamWrapper>> ssThreshStream;  //!< SlowStart threshold outut stream.
static std::map<NodeIdType, Ptr<OutputStreamWrapper>> rttStream;       //!< RTT outut stream.
static std::map<NodeIdType, Ptr<OutputStreamWrapper>> rtoStream;       //!< RTO outut stream.
static std::map<NodeIdType, Ptr<OutputStreamWrapper>> nextTxStream;    //!< Next TX outut stream.
static std::map<NodeIdType, Ptr<OutputStreamWrapper>> nextRxStream;    //!< Next RX outut stream.
static std::map<NodeIdType, uint32_t> cWndValue;                       //!< congestion window value.
static std::map<NodeIdType, uint32_t> ssThreshValue;                   //!< SlowStart threshold value.

/**
 * Get the Node Id From Context.
 *
 * \param context The context.
 * \return the node ID.
 */
static NodeIdType GetNodeIdFromContext(std::string context) {
  std::size_t const n1 = context.find_first_of('/', 1);
  std::size_t const n2 = context.find_first_of('/', n1 + 1);
  return std::stoul(context.substr(n1 + 1, n2 - n1 - 1));
}

// Congestion window trace connection.
#pragma region Cwnd

static void CwndTracer(std::string context, uint32_t oldval, uint32_t newval) {
  uint32_t nodeId = GetNodeIdFromContext(context);
  auto& fs = (*cWndStream[nodeId]->GetStream());
  if (firstCwnd[nodeId] && FIRST) {
    fs << "0.0 " << oldval << std::endl;
    firstCwnd[nodeId] = false;
  }
  fs << Simulator::Now().GetSeconds() << " " << newval << std::endl;
  cWndValue[nodeId] = newval;

  if (!firstSshThr[nodeId]) {
    auto& thresh_fs = *ssThreshStream[nodeId]->GetStream();
    thresh_fs << Simulator::Now().GetSeconds() << " " << ssThreshValue[nodeId] << std::endl;
  }
};

static void TraceCwnd(std::string cwnd_tr_file_name, uint32_t nodeId) {
  AsciiTraceHelper ascii;
  cWndStream[nodeId] = ascii.CreateFileStream(cwnd_tr_file_name);

  Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
                  MakeCallback(&CwndTracer));
}

#pragma endregion Cwnd

// Slow start threshold trace connection.
#pragma region threshold

static void SsThreshTracer(std::string context, uint32_t oldval, uint32_t newval) {
  uint32_t nodeId = GetNodeIdFromContext(context);

  auto& fs = *ssThreshStream[nodeId]->GetStream();
  if (firstSshThr[nodeId] && FIRST) {
    *ssThreshStream[nodeId]->GetStream() << "0.0 " << oldval << std::endl;
    firstSshThr[nodeId] = false;
  }

  fs << Simulator::Now().GetSeconds() << " " << newval << std::endl;
  ssThreshValue[nodeId] = newval;

  if (!firstCwnd[nodeId]) {
    auto& cwnd_fs = *ssThreshStream[nodeId]->GetStream();
    cwnd_fs << Simulator::Now().GetSeconds() << " " << cWndValue[nodeId] << std::endl;
  }
}

static void TraceSsThresh(std::string ssthresh_tr_file_name, uint32_t nodeId) {
  AsciiTraceHelper ascii;
  ssThreshStream[nodeId] = ascii.CreateFileStream(ssthresh_tr_file_name);
  Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold",
                  MakeCallback(&SsThreshTracer));
}

#pragma endregion threshold

// RTT trace connection.
#pragma region RTT

static void RttTracer(std::string context, Time oldval, Time newval) {
  uint32_t nodeId = GetNodeIdFromContext(context);
  auto& fs = *rttStream[nodeId]->GetStream();
  if (firstRtt[nodeId] && FIRST) {
    *rttStream[nodeId]->GetStream() << "0.0 " << oldval.GetMilliSeconds() << std::endl;
    firstRtt[nodeId] = false;
  }
  fs << Simulator::Now().GetSeconds() << " " << newval.GetMilliSeconds() << std::endl;
}

static void TraceRtt(std::string rtt_tr_file_name, uint32_t nodeId) {
  AsciiTraceHelper ascii;
  rttStream[nodeId] = ascii.CreateFileStream(rtt_tr_file_name);
  Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/0/RTT",
                  MakeCallback(&RttTracer));
}

#pragma endregion RTT

// RTO trace connection.
#pragma region RTO

static void RtoTracer(std::string context, Time oldval, Time newval) {
  uint32_t nodeId = GetNodeIdFromContext(context);
  auto& fs = *rtoStream[nodeId]->GetStream();
  if (firstRto[nodeId] && FIRST) {
    fs << "0.0 " << oldval.GetSeconds() << std::endl;
    firstRto[nodeId] = false;
  }
  fs << Simulator::Now().GetSeconds() << " " << newval.GetSeconds() << std::endl;
}

static void TraceRto(std::string rto_tr_file_name, uint32_t nodeId) {
  AsciiTraceHelper ascii;
  rtoStream[nodeId] = ascii.CreateFileStream(rto_tr_file_name);
  Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/0/RTO",
                  MakeCallback(&RtoTracer));
}

#pragma endregion RTO

// Next TX trace connection.

#pragma region TX

static void NextTxTracer(std::string context, SequenceNumber32 old [[maybe_unused]], SequenceNumber32 nextTx) {
  uint32_t nodeId = GetNodeIdFromContext(context);
  auto& fs = *nextTxStream[nodeId]->GetStream();
  fs << Simulator::Now().GetSeconds() << " " << nextTx << std::endl;
}

static void TraceNextTx(std::string& next_tx_seq_file_name, uint32_t nodeId) {
  AsciiTraceHelper ascii;
  nextTxStream[nodeId] = ascii.CreateFileStream(next_tx_seq_file_name);
  NS_LOG_UNCOND("Start flow" << std::to_string(nodeId) << " At " << Simulator::Now().GetSeconds() << " sec");
  Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/0/NextTxSequence",
                  MakeCallback(&NextTxTracer));
}

#pragma endregion TX

// Next RX trace connection.
#pragma region RX

static void NextRxTracer(std::string context, SequenceNumber32 old [[maybe_unused]], SequenceNumber32 nextRx) {
  uint32_t nodeId = GetNodeIdFromContext(context);
  auto& fs = *nextRxStream[nodeId]->GetStream();
  fs << Simulator::Now().GetSeconds() << " " << nextRx << std::endl;
}

static void TraceNextRx(std::string& next_rx_seq_file_name, uint32_t nodeId) {
  AsciiTraceHelper ascii;
  nextRxStream[nodeId] = ascii.CreateFileStream(next_rx_seq_file_name);
  NS_LOG_UNCOND("Rx Trace flow" << std::to_string(nodeId) << " start at " << Simulator::Now().GetSeconds() << " sec");
  Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/1/RxBuffer/NextRxSequence",
                  MakeCallback(&NextRxTracer));
}

#pragma endregion RX

/**
 * Congestion window change callback
 */
// static void CwndChange(uint32_t oldCwnd, uint32_t newCwnd) {
//   NS_LOG_UNCOND(Simulator::Now().GetMilliSeconds() << " ms\t" << newCwnd);
// }

/**
 * Rx drop callback
 */
static void RxDrop(Ptr<const Packet> p) { NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds()); }

int main(int argc, char* argv[]) {
  std::string transport_proto = "TcpNewReno";  // TcpNewReno, TcpVegas, TcpHighSpeed
  double error_p = 0.001;                      // Packet error rate
  std::string bandwidth = "5Mbps";           // Bottleneck bandwidth
  std::string delay = "35ms";                  // Bottleneck delay
  std::string access_bandwidth = "1Mbps";     // Access link bandwidth
  std::string access_delay = "0.01ms";         // Access link delay
  uint64_t data_mbytes = 0;                    // Number of Megabytes of data to transmit
  uint32_t mtu_bytes = 400;                    // Size of IP packets to send in bytes
  uint16_t connections = 3;                    // Number of flows
  uint32_t run = 3;                            // Run index (for setting repeatable seeds)
  u_int16_t timeGap = 60;
  bool sack = true;

  // @reference
  // https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/8/html/configuring_and_managing_networking/linux-traffic-control_configuring-and-managing-networking
  /**
   * Linux: pfifo_fast
   */
  std::string queue_disc_type = "ns3::PfifoFastQueueDisc";
  std::string recovery = "ns3::TcpClassicRecovery";

  CommandLine cmd(__FILE__);

  cmd.AddValue("transport_proto", "TCP Protocol", transport_proto);
  cmd.Parse(argc, argv);

  std::string prefix_file_name = transport_proto;
  transport_proto = std::string("ns3::") + transport_proto;

  SeedManager::SetSeed(1);
  SeedManager::SetRun(run);

  // User may find it convenient to enable logging
  // LogComponentEnable("TcpVariantsComparison", LOG_LEVEL_ALL);
  // LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
  // LogComponentEnable("PfifoFastQueueDisc", LOG_LEVEL_ALL);

  // Calculate the ADU size
  Header* temp_header = new Ipv4Header();
  uint32_t ip_header = temp_header->GetSerializedSize();

  delete temp_header;
  temp_header = new TcpHeader();
  uint32_t tcp_header = temp_header->GetSerializedSize();
  delete temp_header;

  uint32_t tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);

  // Set the simulation start and stop time
  double startAt = 0;
  double stopAt = (connections + 1) * timeGap;

  NS_LOG_UNCOND("Protocol: " << transport_proto << "\n");
  NS_LOG_UNCOND("Bottleneck bandwidth: " << bandwidth);
  NS_LOG_UNCOND("Bottleneck delay: " << delay << "\n");
  NS_LOG_UNCOND("Local Bandwidth: " << access_bandwidth);
  NS_LOG_UNCOND("Local Delay: " << access_delay << "\n");
  NS_LOG_UNCOND("MTU Bytes: " << mtu_bytes);
  NS_LOG_UNCOND("IP Header size is: " << ip_header << " Bytes");
  NS_LOG_UNCOND("TCP Header size is: " << tcp_header << " Bytes");
  NS_LOG_UNCOND("Segment size is: " << tcp_adu_size << " Bytes");
  NS_LOG_UNCOND("data size to transmit (MB): " << data_mbytes << " (0 = 無限制)");
  NS_LOG_UNCOND("how many connections: " << connections);
  NS_LOG_UNCOND("It start at " << startAt << " sec, end at " << stopAt << " sec");
  NS_LOG_UNCOND("\n=========================\n");
  // 8 KB of TCP buffer
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(8 << 10));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(8 << 10));
  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(sack));
  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));
  // Select TCP variant
  TypeId tcpTid;
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName(transport_proto)));

  // Create gateways, sources, and sinks
  NodeContainer gateways;
  gateways.Create(1);

  NodeContainer sources;
  sources.Create(connections);

  NodeContainer sinks;
  sinks.Create(connections);

  // Configure the error model, Here we use RateErrorModel with packet error rate
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  uv->SetStream(50);
  RateErrorModel error_model;
  error_model.SetRandomVariable(uv);
  error_model.SetUnit(RateErrorModel::ERROR_UNIT_PACKET);
  error_model.SetRate(error_p);

  PointToPointHelper UnReLink;
  UnReLink.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  UnReLink.SetChannelAttribute("Delay", StringValue(delay));
  UnReLink.SetDeviceAttribute("ReceiveErrorModel", PointerValue(&error_model));

  InternetStackHelper stack;
  stack.InstallAll();

  TrafficControlHelper tchPfifo;
  tchPfifo.SetRootQueueDisc("ns3::PfifoFastQueueDisc");

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  // Configure the sources and sinks net devices
  // and the channels between the sources/sinks and the gateways
  PointToPointHelper LocalLink;
  LocalLink.SetDeviceAttribute("DataRate", StringValue(access_bandwidth));
  LocalLink.SetChannelAttribute("Delay", StringValue(access_delay));

  Ipv4InterfaceContainer sink_interfaces;

  DataRate access_b(access_bandwidth);
  DataRate bottle_b(bandwidth);
  Time access_d(access_delay);
  Time bottle_d(delay);

  uint32_t size =
      static_cast<uint32_t>((std::min(access_b, bottle_b).GetBitRate() / 8) * ((access_d + bottle_d) * 2).GetSeconds());

  Config::SetDefault("ns3::PfifoFastQueueDisc::MaxSize",
                     QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, size / mtu_bytes)));

  for (uint32_t i = 0; i < connections; i++) {
    NetDeviceContainer devices;
    devices = LocalLink.Install(sources.Get(i), gateways.Get(0));
    tchPfifo.Install(devices);
    address.NewNetwork();
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    devices = UnReLink.Install(gateways.Get(0), sinks.Get(i));
    tchPfifo.Install(devices);
    address.NewNetwork();
    interfaces = address.Assign(devices);

    sink_interfaces.Add(interfaces.Get(1));
    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));
  }

  NS_LOG_INFO("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  /* Send/Recv Setting */
  uint16_t port = 50000;
  Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

  NS_LOG_UNCOND("flow sources: " << sources.GetN());
  for (uint32_t i = 0; i < sources.GetN(); i++) {
    AddressValue remoteAddress(InetSocketAddress(sink_interfaces.GetAddress(i, 0), port));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(tcp_adu_size));
    BulkSendHelper ftp("ns3::TcpSocketFactory", Address());

    ftp.SetAttribute("Remote", remoteAddress);
    ftp.SetAttribute("SendSize", UintegerValue(tcp_adu_size));
    ftp.SetAttribute("MaxBytes", UintegerValue(data_mbytes * 1 << 20));
    ApplicationContainer sourceApp = ftp.Install(sources.Get(i));
    sourceApp.Start(Seconds(startAt + (i * timeGap)));
    sourceApp.Stop(Seconds(stopAt - 3));

    sinkHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
    NS_LOG_UNCOND("sink from " << startAt + (i * timeGap) << "s to " << stopAt << "s ");
    ApplicationContainer sinkApp = sinkHelper.Install(sinks.Get(i));
    sinkApp.Start(Seconds(startAt));
    sinkApp.Stop(Seconds(stopAt));
  }

  // Set up tracing
  //   std::ofstream ascii;
  //   Ptr<OutputStreamWrapper> ascii_wrap;
  //   ascii.open(prefix_file_name + "-ascii");
  //   ascii_wrap = new OutputStreamWrapper(prefix_file_name + "-ascii", std::ios::out);
  //   stack.EnableAsciiIpv4All(ascii_wrap);

  for (uint16_t index = 0; index < connections; index++) {
    std::string flowString = "";
    if (connections > 1) {
      flowString = "-flow" + std::to_string(index);
    }

    firstCwnd[index + 1] = true;
    firstSshThr[index + 1] = true;
    firstRtt[index + 1] = true;
    firstRto[index + 1] = true;

    NS_LOG_UNCOND("tracing" << flowString << " Schedule arrange: " << startAt + (index * timeGap) + 1 << " sec");

    Simulator::Schedule(Seconds(startAt + (index * timeGap) + 0.1),
                        &TraceCwnd,
                        prefix_file_name + flowString + "-cwnd.data",
                        index + 1);
    Simulator::Schedule(Seconds(startAt + (index * timeGap) + 0.1),
                        &TraceSsThresh,
                        prefix_file_name + flowString + "-ssth.data",
                        index + 1);
    Simulator::Schedule(Seconds(startAt + (index * timeGap) + 0.1),
                        &TraceRtt,
                        prefix_file_name + flowString + "-rtt.data",
                        index + 1);
    Simulator::Schedule(Seconds(startAt + (index * timeGap) + 0.1),
                        &TraceRto,
                        prefix_file_name + flowString + "-rto.data",
                        index + 1);
    Simulator::Schedule(Seconds(startAt + (index * timeGap) + 0.1),
                        &TraceNextTx,
                        prefix_file_name + flowString + "-next-tx.data",
                        index + 1);
    // Simulator::Schedule(Seconds(startAt + (index * timeGap) + 30),
    //                     &TraceNextRx,
    //                     prefix_file_name + flowString + "-next-rx.data",
    //                     connections + index + 1);
  }

  // Flow monitor
  FlowMonitorHelper flowHelper;
  flowHelper.InstallAll();

  Simulator::Stop(Seconds(stopAt));
  Simulator::Run();

  flowHelper.SerializeToXmlFile(prefix_file_name + ".flowmonitor", true, true);

  Simulator::Destroy();
  return 0;
}