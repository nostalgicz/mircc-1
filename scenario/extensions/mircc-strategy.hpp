
#include "face/face.hpp"
#include "fw/strategy.hpp"
#include "fw/algorithm.hpp"
#include "fw/process-nack-traits.hpp"
#include <unordered_map>
#include <limits>
#include <deque>
#include "ndn-cxx/lp/tags.hpp"
#include "common/global.hpp"
#include "ns3/simulator.h"


namespace nfd {
    namespace fw {
        class MIRCCQueue
        {
        public:
            std:: deque<shared_ptr<Interest>> qp,qs;
            std:: deque<time::steady_clock::TimePoint> tp,ts;

            std:: deque<shared_ptr<pit::Entry>> pp,ps;

        };

        class MIRCCStrategy : public Strategy, public ProcessNackTraits<MIRCCStrategy> 
        {
        public:
            explicit MIRCCStrategy(Forwarder &forwarder, const ndn::Name &name);

            ~MIRCCStrategy() override = default;

            void
            afterReceiveInterest(const FaceEndpoint &ingress, const Interest &interest,const shared_ptr<pit::Entry> &pitEntry) override;

            void
            afterReceiveNack(const FaceEndpoint &ingress, const lp::Nack &nack,const shared_ptr<pit::Entry> &pitEntry) override;

            void
            beforeSatisfyInterest(const shared_ptr<pit::Entry> &pitEntry,const FaceEndpoint &ingress, const Data &data) override;

            void
            afterReceiveData(const shared_ptr<pit::Entry> &pitEntry,const FaceEndpoint &ingress, const Data &data) override;

            static const Name &
            getStrategyName();

        protected:
            friend ProcessNackTraits<MIRCCStrategy>;

        private:
            static const uint64_t MAX_FACE = 12;
            const uint64_t DATA_SIZE = 8696;
            const uint64_t MAX_QUEUE = 10;

            const double m_B = 0.1;
            const double m_a = 0.8;
            const double m_miu = 0.99;
            const double m_rtt_a = 0.7;


            uint64_t m_Rp_base[MAX_FACE];
            uint64_t m_Rp_ex[MAX_FACE];
            uint64_t m_Rs_base[MAX_FACE];
            uint64_t m_Rs_ex[MAX_FACE];
            uint64_t m_Yp[MAX_FACE];
            uint64_t m_Yp_old[MAX_FACE];
            uint64_t m_Ys[MAX_FACE];
            uint64_t m_Ys_old[MAX_FACE];
            uint64_t m_linkC[MAX_FACE];

            double m_d[MAX_FACE];
            

            MIRCCQueue m_Queue[MAX_FACE];
            
            
            void
            sendQueueInterest(FaceId mface);

            
            void
            pushQueueInterest(FaceId mface, const shared_ptr<Interest>& interest,const  shared_ptr<pit::Entry>& pitEntry);

            
            uint64_t 
            getQt(FaceId mface);

            
            void
            calcRt(FaceId mface);

            
            void 
            mirccinit();

            
            uint64_t
            inface(FaceId mface)
            {
                return mface - 257;
            }
        };
    }
}




