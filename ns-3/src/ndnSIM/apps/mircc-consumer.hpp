

#ifndef NDN_CONSUMER_MIRCC_H
#define NDN_CONSUMER_MIRCC_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ndn-consumer.hpp"
#include <unordered_map>
#include <ndn-cxx/lp/tags.hpp>


namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief Ndn application for sending out Interest packets at a "constant" rate (Poisson process)
 */
class ConsumerMIRCC : public Consumer {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   * Sets up randomizer function and packet sequence number
   */
  ConsumerMIRCC();
  virtual ~ConsumerMIRCC();

protected:
  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  virtual void
  ScheduleNextPacket();


  void
  StopApplication() override;
  //相比原有的consumer，MIRCC多个子流同时发送，存在一个eventid表需要关闭。

  void
  StartApplication() override;
  //相比原有的consumer，MIRCC第一步为先探索路径，而非直接发送

  //void
  //SendPacket(uint64_t pathId) override;

  void
  OnData(shared_ptr<const Data> data) override;

  void
  OnTimeout(uint32_t sequenceNumber);

protected:
  //这两个目前没有用处
  double m_frequency; // Frequency of interest packets (in hertz)
  bool m_firstTime;
  //
  Ptr<RandomVariableStream> m_random;
  std::string m_randomType;

private:
  //路径id到发送事件id的一一映射关系
  std:: unordered_map <uint64_t,EventId> pathId2EventId_p,pathId2EventId_s;

  //路径id到路径随机数的一一映射关系（使用对应路径随机数使得路由器按该路径转发）
  std:: unordered_map <uint64_t,uint64_t> pathId2RandomId;

  //路径id到该路径Rp的一一映射关系
  std:: unordered_map <uint64_t,uint64_t> pathId2Rp;

  //路径id到该路径Rs的一一映射关系
  std:: unordered_map <uint64_t,uint64_t> pathId2Rs;

  //
  std:: unordered_map <uint64_t,uint64_t> seq2RandomId;
  // 序列号到 classId 的映射
  std::unordered_map<uint64_t, uint64_t> seq2ClassId;
  // 序列号到 pathId 的映射
  std::unordered_map<uint64_t, uint64_t> seq2PathId;


  const uint64_t INTEREST_SIZE = 200,DATA_SIZE = 8696,C = 1000 * 1000;
  
  uint64_t m_start_ms,m_end_ms,appid,logtms,greedy_ms,greedy_rate,m_greedy_tag = 0,logcnt = 0,pktcnt_p = 0,pktcnt_s = 0,inscnt_p = 0,inscnt_s = 0,schcnt_p = 0,schcnt_s = 0;
  //利用随机数探索路径
  void PathExplorate();

  //使用pathId发送
  void SchedulePacketForPathId(uint64_t pathId,uint64_t classId);

  //计算所有Rp的速率之和
  uint64_t SumRp();


  //计算所有Rp的速率最大值
  uint64_t MaxRp();

  //记录seq号和随机数，用于绑定pathid和randid。
  void recordSeq();

  void tryBindRandId(uint64_t seq, uint64_t pathId);

  void recordSeq(uint64_t seq,uint64_t randId);
  
  void SendPacket(uint64_t pathId,uint64_t classId);

  void try_refresh_p(uint64_t pathId,uint64_t Rp);

  void try_refresh_s(uint64_t pathId,uint64_t Rs);

  void mircclog();

  void StartGreedy();
  
  EventId explore_event,log_event,greedy_event;
};


//注意，目前版本的consumer假设所有节点的FIB不产生变化，否则会产生大量的路径判断未知问题！！！

} // namespace ndn
} // namespace ns3

#endif
