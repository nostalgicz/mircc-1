#include "mircc-consumer.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include <fstream>

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerMIRCC");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerMIRCC);

TypeId
ConsumerMIRCC::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerMIRCC")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<ConsumerMIRCC>()

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::m_seqMax), MakeIntegerChecker<uint32_t>())
      .AddAttribute("appid", "app id",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::appid), MakeIntegerChecker<uint32_t>())
      .AddAttribute("logtms", "log t ms",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::logtms), MakeIntegerChecker<uint32_t>())
      .AddAttribute("start_ms", "start time ms",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::m_start_ms), MakeIntegerChecker<uint32_t>())
      .AddAttribute("end_ms", "end time ms",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::m_end_ms), MakeIntegerChecker<uint32_t>())
      .AddAttribute("greedy_ms", "start_greedy_ms",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::greedy_ms), MakeIntegerChecker<uint32_t>())
      .AddAttribute("greedy_rate", "greedy_rate",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerMIRCC::greedy_rate), MakeIntegerChecker<uint32_t>())
    ;

  return tid;
}

ConsumerMIRCC::ConsumerMIRCC()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

ConsumerMIRCC::~ConsumerMIRCC()
{
}


void
ConsumerMIRCC::ScheduleNextPacket()
{
  std::printf("");
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";
/*
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &Consumer::, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &Consumer::SendPacket, this);*/
}

void
ConsumerMIRCC:: StopApplication()
{
  //获取发送事件表的迭代器
  auto it = pathId2EventId_p.begin();

  //遍历发送事件表
  while(it != pathId2EventId_p.end())
  {
    //取消发送事件
    EventId tmp = it->second;
    Simulator::Cancel(tmp);

    //迭代器后移
    it++;
  }


    auto it2 = pathId2EventId_s.begin();

  //遍历发送事件表
  while(it2 != pathId2EventId_s.end())
  {
    //取消发送事件
    EventId tmp = it2->second;
    Simulator::Cancel(tmp);

    //迭代器后移
    it2++;
  }

  Simulator::Cancel(explore_event);
  Simulator::Cancel(log_event);
  // 调用父类停止face等接口服务
  App::StopApplication();
}

void 
ConsumerMIRCC::mircclog()
{
  if (logtms == 0) {
    return;
  }
  log_event = Simulator::Schedule(Seconds((double)logtms/1000), &ConsumerMIRCC::mircclog, this);
  double time = logcnt * (double)logtms / 1000;

  std::ofstream ofs;
  logcnt += 1;
  if (logcnt == 1)
  {
    return;
  }

  ofs.open(std::to_string(appid) + ".txt",std::ios::app);

  ofs << time << "	Src1	258	appFace://	OutData	" << pktcnt_p / ((double)logtms / 1000) << "	" << pktcnt_p * 1.024 / ((double)logtms / 1000)<< "	" << pktcnt_p << "	" << pktcnt_p * 1.024 << std::endl;
  pktcnt_p = 0;
  ofs << time << "	Src1	259	appFace://	OutData	" << pktcnt_s / ((double)logtms / 1000) << "	" << pktcnt_s * 1.024 / ((double)logtms / 1000)<< "	" << pktcnt_s << "	" << pktcnt_s * 1.024 << std::endl;
  pktcnt_s = 0;
  
  //
  ofs << time << "	Src1	260	appFace://	OutInterests	" << inscnt_p / ((double)logtms / 1000) << "	" << 0 << "	" << inscnt_p << "	" << 0 << std::endl;
  inscnt_p = 0;
  ofs << time << "	Src1	261	appFace://	OutInterests	" << inscnt_s / ((double)logtms / 1000) << "	" << 0 << "	" << inscnt_s << "	" << 0 << std::endl;
  inscnt_s = 0;
  ofs << time << "	Src1	262	appFace://	OutInterests	" << schcnt_p / ((double)logtms / 1000) << "	" << 0 << "	" << schcnt_p << "	" << 0 << std::endl;
  schcnt_p = 0;
  ofs << time << "	Src1	263	appFace://	OutInterests	" << schcnt_s / ((double)logtms / 1000) << "	" << 0 << "	" << schcnt_s << "	" << 0 << std::endl;
  //
  
  if (appid == 1)
  {
    for (auto it = pathId2EventId_p.begin();it != pathId2EventId_p.end();it++)
    {
        NS_LOG_DEBUG("delay" << Simulator::GetDelayLeft(it->second));
    }
  }

  schcnt_s = 0;
  ofs.close();


}


void
ConsumerMIRCC:: StartApplication()
{

  // 调用父类开启face等接口服务
  App::StartApplication();
  
  // 如果logtms == 0，不记录客户端日志
  if (logtms > 0) {
    log_event = Simulator::Schedule(Seconds((double)logtms/1000), &ConsumerMIRCC::mircclog, this);
    std::ofstream ofs;
    ofs.open(std::to_string(appid) + ".txt",std::ios::app);
    ofs << "Time	Node	FaceId	FaceDescr	Type	Packets	Kilobytes	PacketRaw	KilobytesRaw" << std::endl;
    ofs.close();
  }

  // 调用路径探索
  if (pathId2RandomId.size() == 0)
  {//如果节点刚开始运行
    //尝试利用随机数探索路径
    explore_event = Simulator::Schedule(Seconds((double)m_start_ms / 1000 + 0.0000001), &ConsumerMIRCC::PathExplorate, this);
    Simulator::Schedule(Seconds((double)m_end_ms / 1000 + 0.0000001), &ConsumerMIRCC::StopApplication, this);
    if (greedy_rate != 0)
    {
    Simulator::Schedule(Seconds((double)greedy_ms / 1000 + 0.0000001), &ConsumerMIRCC::StartGreedy, this);
    }
  }
}

void 
ConsumerMIRCC ::StartGreedy()
{
  m_greedy_tag = 1;
  return;
}

void 
ConsumerMIRCC::recordSeq(uint64_t seq,uint64_t randId)
{//记录seq到当时使用的randId
  seq2RandomId[seq] = randId;
  return;
}

void
ConsumerMIRCC::SendPacket(uint64_t pathId,uint64_t classId)
{
  if (!m_active)
    return;
   // if (appid == 1)
    //  NS_LOG_DEBUG("MIRCC SendPacket");
  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid
  
  //处理重传兴趣包
  //所以重传未必是用之前的子流重传，可能是新的子流
  //所以重传的兴趣包为一类还是二类完全随机，目前没有固定策略
  while (m_retxSeqs.size()) 
  {
    //取出第一个要重传的兴趣包
    seq = *m_retxSeqs.begin();
    //从重传队列删除
    m_retxSeqs.erase(m_retxSeqs.begin());
    std::cout << "retrans: " << seq << std::endl;
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max())
  {//如果没有需要重传的数据包
    if (m_seqMax != std::numeric_limits<uint32_t>::max())
    {
      if (m_seq >= m_seqMax) 
        {//已经把需要的序列号兴趣包都发完了
        return; // we are totally done
      }
    }

    //没有重传数据包的前提下，顺次获取下一个需要发送的序号
    seq = m_seq++;
  }
  //
  seq2ClassId[seq] = classId;
  seq2PathId[seq] = pathId;
  
  
  if (classId == 1)
  {
      inscnt_p += 1;
  }else
  {
    inscnt_s += 1;
  }
  //
  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  nameWithSequence->appendSequenceNumber(seq);
  //

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  //获取对应PathId（子流）对应的RandId。
  uint64_t tmpRandId;
  if (pathId != 0)
  {
     tmpRandId = pathId2RandomId[pathId];
  }else
  {
    tmpRandId = m_rand->GetValue(0.00001, std::numeric_limits<uint32_t>::max());
  }
  //将RandId作为Tag设置到interest包中。
  interest->setTag(make_shared<lp::PathidTag>(tmpRandId));
  interest->setTag(make_shared<lp::PTag>(classId));

  //如果判断发现是随机数探索，则记录seq和随机数，用来收到data后把随机数和pathid绑定。
  if (pathId == 0)
  {
    recordSeq(seq,tmpRandId);
  }


  WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  //这个函数只负责发包，不在负责调用下一个包的发送
  //ScheduleNextPacket();
}

uint64_t
ConsumerMIRCC:: SumRp()
{
  //获取遍历所有Rp的迭代器
  auto it = pathId2Rp.begin();
  uint64_t res = 0;
  //遍历所有Rp的迭代器
  while (it != pathId2Rp.end())
  {
    res += it->second;
    it++;
  }
  return res;
}

uint64_t
ConsumerMIRCC:: MaxRp()
{
  //获取遍历所有Rp的迭代器
  auto it = pathId2Rp.begin();
  uint64_t res = 0;
  //遍历所有Rp的迭代器
  while (it != pathId2Rp.end())
  {
    res = std::max(res,it->second);
    it++;
  }
  return res;
}

void
ConsumerMIRCC::SchedulePacketForPathId(uint64_t pathId,uint64_t classId)
{
  SendPacket(pathId,classId);
  if (classId == 1) {
    //发送第一类兴趣包，参考当先path的Rp在所有Rp之和的占比 乘上最大Rp
    uint64_t tmpRp;
    if (m_greedy_tag == 0)
    {
      tmpRp = pathId2Rp[pathId] * MaxRp() / std::max(SumRp(),(uint64_t)1) ;
      tmpRp = std::max(tmpRp,DATA_SIZE);
    }else
    {
      tmpRp = pathId2Rp[pathId] * greedy_rate / std::max(SumRp(),(uint64_t)1) ;
      tmpRp = std::max(tmpRp,DATA_SIZE);
    }

    schcnt_p += 1;
    pathId2EventId_p[pathId] = Simulator::Schedule(Seconds(1.0 / (tmpRp / DATA_SIZE)), &ConsumerMIRCC::SchedulePacketForPathId,this, pathId,classId);

  } else {
    //发送第二类兴趣包
    uint64_t tmpRs ;
    if (m_greedy_tag == 0)
    {
       tmpRs = pathId2Rs[pathId];
      tmpRs = std::max(tmpRs,DATA_SIZE);
    }else
    {
      return;
    }
    schcnt_s += 1;
   
  
    pathId2EventId_s[pathId] = Simulator::Schedule(Seconds(1.0 / (tmpRs / DATA_SIZE)), &ConsumerMIRCC::SchedulePacketForPathId,this, pathId,classId);

  }

}

void 
ConsumerMIRCC:: PathExplorate()
{
  //路径探索的兴趣包全部标为第一类，原论文没有明确说，但是我觉得这样更合理
  //因为用于路径探索的兴趣包很重要，如果被丢掉了，consumer可能迟迟无法开始正式发送兴趣包
  if (appid == 1)
  NS_LOG_DEBUG("PahtExplorate");
  if (appid == 1)
  for (auto i = pathId2RandomId.begin();i != pathId2RandomId.end();i++)
  {
    
    NS_LOG_DEBUG("pathId" << i->first << " RandId" << i->second);
    NS_LOG_DEBUG("Rp" << pathId2Rp[i->first] << " Rs" << pathId2Rs[i->first]);
  }
  explore_event = Simulator::Schedule(Seconds(0.1), &ConsumerMIRCC::PathExplorate, this);
  

  //PathId为0表示正在进行路径探索
  SendPacket(0,1);

}

void
ConsumerMIRCC::tryBindRandId(uint64_t seq, uint64_t pathId)
{
  if (seq2RandomId.find(seq) == seq2RandomId.end())
  {
    //没有找到seq对应的需要绑定的pathId
    return;
  }

  if (pathId2RandomId.find(pathId) != pathId2RandomId.end())
  {
    //随机数对应了一条已经找到对应随机数的pathid，没有发现新路径
    return;
  }
  //将随机数绑定到pathid上
  pathId2RandomId[pathId] = seq2RandomId[seq];
  return;
}

  void
  ConsumerMIRCC::try_refresh_p(uint64_t pathId,uint64_t Rp)
  {

    if (pathId2EventId_p.find(pathId) == pathId2EventId_p.end())
    {
      uint64_t tmpRp = Rp * MaxRp() / std::max(SumRp(),(uint64_t)1);
       tmpRp = std::max(tmpRp,DATA_SIZE);
      pathId2EventId_p[pathId] = Simulator::Schedule(Seconds(1.0 / (tmpRp / DATA_SIZE)), &ConsumerMIRCC::SchedulePacketForPathId,this, pathId,1);
      return;
    }

    auto tmp1 = Simulator::GetDelayLeft(pathId2EventId_p[pathId]);
    

    uint64_t tmpRp = Rp * MaxRp() / std::max(SumRp(),(uint64_t)1);
    tmpRp = std::max(tmpRp,DATA_SIZE);
    Time tmp2 = Seconds(1.0 / (tmpRp / DATA_SIZE));

    if (tmp1 > tmp2)
    {
      Simulator::Cancel(pathId2EventId_p[pathId]);
      pathId2EventId_p[pathId] = Simulator::Schedule(Seconds(1.0 / (tmpRp / DATA_SIZE)), &ConsumerMIRCC::SchedulePacketForPathId,this, pathId,1);
    }
  }

  void 
  ConsumerMIRCC::try_refresh_s(uint64_t pathId,uint64_t Rs)
  {
    if (pathId2EventId_s.find(pathId) == pathId2EventId_s.end())
    {
      uint64_t tmpRs = Rs;
      tmpRs = std::max(tmpRs,DATA_SIZE);
      pathId2EventId_s[pathId] = Simulator::Schedule(Seconds(1.0 / (tmpRs / DATA_SIZE)), &ConsumerMIRCC::SchedulePacketForPathId,this, pathId,2);
      return;
    }

    
    auto tmp1 = Simulator::GetDelayLeft(pathId2EventId_s[pathId]);
    

    uint64_t tmpRs = Rs;
    tmpRs = std::max(tmpRs,DATA_SIZE);
    Time tmp2 = Seconds(1.0 / (tmpRs / DATA_SIZE));

    if (tmp1 > tmp2)
    {
      Simulator::Cancel(pathId2EventId_s[pathId]);
      pathId2EventId_s[pathId] = Simulator::Schedule(Seconds(1.0 / (tmpRs / DATA_SIZE)), &ConsumerMIRCC::SchedulePacketForPathId,this, pathId,2);
    }
  }



void
ConsumerMIRCC::OnData(shared_ptr<const Data> data)
{
  if (appid == 1)
  NS_LOG_FUNCTION(this << data);
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  
  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
 // if (appid == 1)
  //NS_LOG_INFO("< DATA for " << seq);

  //查看当前数据包是否有探索兴趣包产生，如果是的话将探索出的pathid和对应的randId绑定
  auto tmpPathid = data->getTag<lp::PathidTag>();
  if (tmpPathid != nullptr)
  {
      tryBindRandId(seq, *tmpPathid);
  }
  //if (appid == 1)
  //NS_LOG_DEBUG("seq " << seq << " ClassId " << seq2ClassId[seq]);
  if (seq2ClassId[seq] == 1)
  {
    pktcnt_p += 1;
  }else
  {
    pktcnt_s += 1;
  }
  seq2ClassId.erase(seq);
  seq2RandomId.erase(seq);
  seq2PathId.erase(seq);



  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  //if (appid == 1)
  //NS_LOG_DEBUG("Hop count!!: " << hopCount);


  uint64_t Rp = 0;
  auto rpTag = data->getTag<lp::RpTag>();
  if (rpTag != nullptr)
  {
    Rp = *rpTag;
  }
  pathId2Rp[*tmpPathid] = Rp;
  if (appid==1)
    NS_LOG_INFO("appid=" << appid << "get Rp: " << Rp);

  
  try_refresh_p(*tmpPathid,Rp);

  uint64_t Rs = 0;
  auto rsTag = data->getTag<lp::RsTag>();
  if (rsTag != nullptr)
  {
    Rs = *rsTag;
  }
  pathId2Rs[*tmpPathid] = Rs;
  if (appid==1)
    NS_LOG_INFO("appid=" << appid << "get Rs: " << Rs);

  try_refresh_s(*tmpPathid,Rs);

  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
  if (entry != m_seqLastDelay.end()) {
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
  }

  entry = m_seqFullDelay.find(seq);
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);
} 

void
ConsumerMIRCC::OnTimeout(uint32_t sequenceNumber) {
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";
  std::cout << "Timeout: " << sequenceNumber << std::endl;

  m_rtt->IncreaseMultiplier(); // Double the next RTO
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
                 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  SchedulePacketForPathId(seq2PathId[sequenceNumber], seq2ClassId[sequenceNumber]);
}
}// namespace ndn
} // namespace ns3
