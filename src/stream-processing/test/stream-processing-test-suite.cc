
#include "ns3/test.h"
#include "ns3/Operator.h"

namespace ns3 {

/**
 * \ingroup aodv-test
 * \ingroup tests
 *
 * \brief AODV test suite
 */
        class StreamProcessingTestSuite : public TestSuite
        {
        public:
            StreamProcessingTestSuite () : TestSuite ("stream-processing", UNIT)
            {
                //AddTestCase (new AggregationWindowOperatorTest, TestCase::QUICK);
            }
        } g_streamProcessingTestSuite; ///< the test suite

}  // namespace ns3
