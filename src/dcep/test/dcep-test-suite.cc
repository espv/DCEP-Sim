
#include "ns3/test.h"
#include "ns3/Operator.h"

namespace ns3 {

/**
 * \ingroup tests
 *
 */
        struct AggregationWindowOperatorTest : public TestCase
        {
            AggregationWindowOperatorTest () : TestCase ("AggregationWindowOperatorTest")
            {
            }
            virtual void DoRun ();
            /**
             * Handler test function
             * \param addr the IPv4 address of the neighbor
             */
            void Handler (Ipv4Address addr);
            /// Check timeout function 1
            void CheckTimeout1 ();
            /// Check timeout function 2
            void CheckTimeout2 ();
            /// Check timeout function 3
            void CheckTimeout3 ();
        };

        void
        AggregationWindowOperatorTest::Handler (Ipv4Address addr)
        {
        }

        void
        AggregationWindowOperatorTest::CheckTimeout1 ()
        {
        }
        void
        AggregationWindowOperatorTest::CheckTimeout2 ()
        {
        }
        void
        AggregationWindowOperatorTest::CheckTimeout3 ()
        {
        }

        void
        AggregationWindowOperatorTest::DoRun ()
        {
            auto group_op = CreateObject<GroupOperator>();
            group_op->stream = CreateObject<Stream>((std::vector<int>){1});

            auto window_op = CreateObject<AggregationWindowOperator>();


            //Simulator::Schedule (Seconds (2), &NeighborTest::CheckTimeout1, this);
            //Simulator::Schedule (Seconds (15), &NeighborTest::CheckTimeout2, this);
            //Simulator::Schedule (Seconds (30), &NeighborTest::CheckTimeout3, this);
            Simulator::Run ();
            Simulator::Destroy ();
        }


/**
 * \ingroup aodv-test
 * \ingroup tests
 *
 * \brief AODV test suite
 */
        class DcepTestSuite : public TestSuite
        {
        public:
            DcepTestSuite () : TestSuite ("dcep-test-suite", UNIT)
            {
                AddTestCase (new AggregationWindowOperatorTest, TestCase::QUICK);
            }
        } g_dcepTestSuite; ///< the test suite

}  // namespace ns3
