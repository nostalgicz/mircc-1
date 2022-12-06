#include "mircc-strategy.hpp"

NFD_LOG_INIT(MIRCCStrategy);

namespace nfd {
    namespace fw {

        MIRCCStrategy::MIRCCStrategy(Forwarder &forwarder, const ndn::Name &name)
        : Strategy(forwarder), ProcessNackTraits<MIRCCStrategy>(this) 
        {
            this->setInstanceName(makeInstanceName(name, getStrategyName()));
            time::nanoseconds ttime(1);
            getScheduler().schedule(ttime, bind(&MIRCCStrategy::mirccinit, this));
        }

        const Name&
        MIRCCStrategy::getStrategyName()
        {
            static Name strategyName("/localhost/nfd/strategy/mircc/%FD%01");
            return strategyName;
        }

        void
        MIRCCStrategy::mirccinit()
        {
            const FaceTable &faceTable = getFaceTable();
            for (auto& tmp : faceTable)
            {
                
                if (tmp.getId() < 257)
                    continue;

                m_linkC[inface(tmp.getId())] =  1 * 1000 * 1000;

                m_Rp_base[inface(tmp.getId())] = m_linkC[inface(tmp.getId())];
                m_Rp_ex[inface(tmp.getId())] = 0;
                m_Rs_base[inface(tmp.getId())] = 0;
                m_Rs_ex[inface(tmp.getId())] = 0;

                m_Yp[inface(tmp.getId())] = 0;
                m_Yp_old[inface(tmp.getId())] = 0;
                m_Ys[inface(tmp.getId())] = 0;
                m_Ys_old[inface(tmp.getId())] = 0;

                m_d[inface(tmp.getId())] = 0.03;

                sendQueueInterest(tmp.getId());

                calcRt(tmp.getId());

            }
        }

        void
        MIRCCStrategy::sendQueueInterest(FaceId mface)
        {
            
            
            time::nanoseconds ttime(std::max((uint64_t)1,(uint64_t)1000000000 * DATA_SIZE / m_linkC[inface(mface)]));
            getScheduler().schedule(ttime, bind(&MIRCCStrategy::sendQueueInterest, this, mface));

            
            if (m_Queue[inface(mface)].qp.size() > 0 and m_Queue[inface(mface)].qs.size() == 0)
            {
                shared_ptr<Interest> interest = m_Queue[inface(mface)].qp.front();
                sendInterest(m_Queue[inface(mface)].pp.front(),FaceEndpoint(*getFaceTable().get(mface),0),*interest);
                m_Queue[inface(mface)].qp.pop_front();
                m_Queue[inface(mface)].tp.pop_front();
                m_Queue[inface(mface)].pp.pop_front();

            }else if (m_Queue[inface(mface)].qs.size() > 0 and m_Queue[inface(mface)].qp.size() == 0)
            {
                shared_ptr<Interest> interest = m_Queue[inface(mface)].qs.front();
                sendInterest(m_Queue[inface(mface)].ps.front(),FaceEndpoint(*getFaceTable().get(mface),0),*interest);
                m_Queue[inface(mface)].qs.pop_front();
                m_Queue[inface(mface)].ts.pop_front();
                m_Queue[inface(mface)].ps.pop_front();
            }else if (m_Queue[inface(mface)].qs.size() > 0 and m_Queue[inface(mface)].qp.size() > 0)
            {
                if (m_Queue[inface(mface)].tp.front() < m_Queue[inface(mface)].ts.front())
                {
                    shared_ptr<Interest> interest =m_Queue[inface(mface)].qp.front();
                    sendInterest(m_Queue[inface(mface)].pp.front(),FaceEndpoint(*getFaceTable().get(mface),0),*interest);
                    m_Queue[inface(mface)].qp.pop_front();
                    m_Queue[inface(mface)].tp.pop_front();
                    m_Queue[inface(mface)].pp.pop_front();
                }else
                {
                    shared_ptr<Interest> interest = m_Queue[inface(mface)].qs.front();
                    sendInterest(m_Queue[inface(mface)].ps.front(),FaceEndpoint(*getFaceTable().get(mface),0),*interest);
                    m_Queue[inface(mface)].qs.pop_front();
                    m_Queue[inface(mface)].ts.pop_front();
                    m_Queue[inface(mface)].ps.pop_front();
                }
            }else
            {
            }
            
        }


        void
        MIRCCStrategy::calcRt(FaceId mface)
        {
            
            time::nanoseconds ttime(100000000);
            getScheduler().schedule(ttime, std::bind(&MIRCCStrategy::calcRt, this, mface));
            
            long long ia,ib,Rp,Rs;
            double db;
            Rp = m_Rp_base[inface(mface)] + m_Rp_ex[inface(mface)];
            Rs = m_Rs_base[inface(mface)] + m_Rs_ex[inface(mface)];

            

            
            
            ia = std::max(m_linkC[inface(mface)],m_Yp[inface(mface)]);

            ib = std::max(Rp,1ll);
            double Np = std::max((double)ia / ib,1.0);

            

            double Bp;
            if (m_Yp[inface(mface)] >= m_Yp_old[inface(mface)])
            {
                ia = m_Yp[inface(mface)] - m_Yp_old[inface(mface)];

                ib = std::max(m_Yp[inface(mface)],(uint64_t)1);

                Bp = std::max(m_B,(double)ia / ib);
            }else
            {
                Bp = m_B;
            }

            

            ia = m_linkC[inface(mface)] - Bp * getQt(mface) / std::max((double)m_d[inface(mface)],(double)0.001);
            
            db = std::max(Np,(double)1);
            long long Rp_base_n = std::min(std::max(ia / db,0.0),m_linkC[inface(mface)] / Np);
            
            m_Rp_base[inface(mface)] = m_a * m_Rp_base[inface(mface)] + (1 - m_a) * Rp_base_n;

            

            ia = m_Yp[inface(mface)];

            db = std::max(Np,(double)1);

            long long Rp_ex_n = std::min(std::max((uint64_t)0,(uint64_t)(Rp - ia / db)),(uint64_t)(m_linkC[inface(mface)] - Rp_base_n));

            m_Rp_ex[inface(mface)] = m_a * m_Rp_ex[inface(mface)] + (1 - m_a) * Rp_ex_n;
            
            
            m_Yp_old[inface(mface)] = m_Yp[inface(mface)];

            
            uint64_t Cs = std::max(m_linkC[inface(mface)] - m_Yp[inface(mface)] ,(uint64_t)0);


            


            
            
            double Ns = std::max((double)std::max(Cs,m_Ys[inface(mface)]) / std::max(Rp + Rs,1ll) ,1.0);

            

            ia = m_Ys[inface(mface)] - m_Ys_old[inface(mface)];
            ib = std::max(m_Ys[inface(mface)],(uint64_t)1);
            double Bs = std::max(m_B,(double)ia / ib);


            ia = Cs - Bs * getQt(mface) / std::max(m_d[inface(mface)],(double)0.001);
            
            db = Ns;

            uint64_t Rs_base_n = std::min((uint64_t)std::max(0.0,ia / db),uint64_t(Cs / Ns));
            m_Rs_base[inface(mface)] = m_a * m_Rs_base[inface(mface)] + (1 - m_a) * Rs_base_n;

            


            ia = m_Ys[inface(mface)];
            db = Ns;


            
            uint64_t Rs_ex_n = std::min((uint64_t)std::max(0.0,Rs - ia / db),uint64_t(Cs - Rs_base_n));

            m_Rs_ex[inface(mface)] = m_a * m_Rs_ex[inface(mface)] + (1 - m_a) * Rs_ex_n;

            

            m_Ys_old[inface(mface)] = m_Ys[inface(mface)];

            m_Yp[inface(mface)] = 0;
            m_Ys[inface(mface)] = 0;
        }

        uint64_t 
        MIRCCStrategy::getQt(FaceId mface)
        {
            
            
            
            uint64_t time = 10;
            return (m_Queue[inface(mface)].qp.size() + m_Queue[inface(mface)].qs.size()) * DATA_SIZE * time;
        }

        void
        MIRCCStrategy::pushQueueInterest(FaceId mface,const shared_ptr<Interest>& interest,const shared_ptr<pit::Entry>& pitEntry)
        {
            
            uint64_t p = *(interest->getTag<lp::PTag>());
            
            if (p == 1)
            {
                
                if (m_Queue[inface(mface)].qp.size() + m_Queue[inface(mface)].qs.size() >= MAX_QUEUE)
                {
                    
                    if (m_Queue[inface(mface)].qs.size() > 0)
                    {
                        
                        m_Queue[inface(mface)].qs.pop_back();
                        m_Queue[inface(mface)].ts.pop_back();
                        m_Queue[inface(mface)].ps.pop_back();
                        m_Queue[inface(mface)].qp.push_back(interest);
                        m_Queue[inface(mface)].tp.push_back(time::steady_clock::now());
                        m_Queue[inface(mface)].pp.push_back(pitEntry);
                    }else
                    {
                        

                        
                        this->rejectPendingInterest(pitEntry);
                    }
                }else
                {
                    
                    m_Queue[inface(mface)].qp.push_back(interest);
                    m_Queue[inface(mface)].tp.push_back(time::steady_clock::now());
                    m_Queue[inface(mface)].pp.push_back(pitEntry);
                }
            }else
            {
              
              if (m_Queue[inface(mface)].qp.size() + m_Queue[inface(mface)].qs.size() >= MAX_QUEUE)
              {
                  
                  this->rejectPendingInterest(pitEntry);
                  
              }else
              {
                  
                  m_Queue[inface(mface)].qs.push_back(interest);
                  m_Queue[inface(mface)].ts.push_back(time::steady_clock::now());
                  m_Queue[inface(mface)].ps.push_back(pitEntry);
              }
            }
            
        }

        void
        MIRCCStrategy::afterReceiveInterest(const FaceEndpoint &ingress, const Interest &interest, const shared_ptr<pit::Entry> &pitEntry)
        {
            

            if (hasPendingOutRecords(*pitEntry))
            {
                
                return;
            }

            
            const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
            fib::NextHopList outFaces = fibEntry.getNextHops();
            for (auto it = outFaces.begin();it != outFaces.end();it++)
            {
                if (it->getFace().getId() ==  ingress.face.getId())
                {
                    outFaces.erase(it);
                    break;
                }
            }

            if (outFaces.size() != 0)
            {
                
                uint64_t hashGuide = *interest.getTag<lp::PathidTag>();
                uint64_t Facenum = hashGuide % outFaces.size();
                hashGuide = hashGuide / outFaces.size();
                interest.setTag(make_shared<lp::PathidTag>(hashGuide));

                Face& outFace = outFaces[Facenum].getFace();
                
                uint64_t p = *interest.getTag<lp::PTag>();
                uint64_t time = 10;
                if (p == 1)
                {
                    m_Yp[inface(outFace.getId())] += DATA_SIZE * time;
                }else
                {
                    m_Ys[inface(outFace.getId())] += DATA_SIZE * time;
                }

                
                this->pushQueueInterest(outFace.getId(), const_cast<Interest&>(interest).shared_from_this(), pitEntry);
                return;
            }

            
            lp::NackHeader nackHeader;
            nackHeader.setReason(lp::NackReason::NO_ROUTE);
            this->sendNack(pitEntry, ingress, nackHeader);
            this->rejectPendingInterest(pitEntry);
            return;
        }

        void
        MIRCCStrategy::afterReceiveNack(const FaceEndpoint &ingress, const lp::Nack &nack, const shared_ptr<pit::Entry> &pitEntry)
        {
            
            
        }

        void
        MIRCCStrategy::beforeSatisfyInterest(const shared_ptr<pit::Entry> &pitEntry, const FaceEndpoint &ingress, const Data &data)
        {
            
            auto now = time::steady_clock::now();
            
            for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
                if (inRecord.getExpiry() > now) 
                {
                    if (inRecord.getFace().getId() == ingress.face.getId() && inRecord.getFace().getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) 
                    {
                        continue;
                    }
                    
                    
                    auto dura = now - inRecord.getLastRenewed();
                    
                    m_d[inface(ingress.face.getId())] = m_rtt_a * m_d[inface(ingress.face.getId())] + (1.0 - m_rtt_a) * ((double)dura.count() / 1000000000);
                }
            }
        }

        void
        MIRCCStrategy::afterReceiveData(const shared_ptr<pit::Entry> &pitEntry, const FaceEndpoint &ingress, const Data &data)
        {
            
            

            if (ingress.face.getId() >= 257)
            {
                
                auto oldPathTag = data.getTag<lp::PathidTag>();
                uint64_t hashVal;
                if (oldPathTag != nullptr)
                {
                    hashVal = *oldPathTag;
                }else
                {
                    hashVal = 0;
                }
                hashVal = (hashVal * 10  + (ingress.face.getId() - 257));
                
                data.setTag(make_shared<lp::PathidTag>(hashVal));

                auto FaceRateTag = data.getTag<lp::PTag>();
                if (FaceRateTag != nullptr)
                {
                    m_linkC[inface(ingress.face.getId())] = m_miu * (*FaceRateTag);
                }else
                {
                    
                    m_linkC[inface(ingress.face.getId())] = m_miu * 8 * 100 * 1000 * 1000;
                }

                
                auto oldRpTag = data.getTag<lp::RpTag>();
                if (oldRpTag != nullptr)
                {
                    uint64_t dataRp = *oldRpTag;
                    if (dataRp > m_Rp_base[inface(ingress.face.getId())] + m_Rp_ex[inface(ingress.face.getId())])
                    {
                        data.setTag(make_shared<lp::RpTag>(m_Rp_base[inface(ingress.face.getId())] + m_Rp_ex[inface(ingress.face.getId())]));
                    }     
                }else
                {
                    data.setTag(make_shared<lp::RpTag>(m_Rp_base[inface(ingress.face.getId())] + m_Rp_ex[inface(ingress.face.getId())]));
                }

                
                auto oldRsTag = data.getTag<lp::RsTag>();
                if (oldRsTag != nullptr)
                {
                    uint64_t dataRs =*oldRsTag;
                    if (dataRs > m_Rs_base[inface(ingress.face.getId())] + m_Rs_ex[inface(ingress.face.getId())])
                    {
                        data.setTag(make_shared<lp::RsTag>(m_Rs_base[inface(ingress.face.getId())] + m_Rs_ex[inface(ingress.face.getId())]));
                    }
                }else
                {
                    data.setTag(make_shared<lp::RsTag>(m_Rs_base[inface(ingress.face.getId())] + m_Rs_ex[inface(ingress.face.getId())]));
                }

            }

            beforeSatisfyInterest(pitEntry, ingress, data); 
            sendDataToAll(pitEntry, ingress, data);
        }
    }
}