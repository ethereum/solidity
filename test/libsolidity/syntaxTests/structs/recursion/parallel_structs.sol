pragma experimental ABIEncoderV2;

contract TestContract
{
    struct SubStruct {
        uint256 id;
    }
    struct TestStruct {
        SubStruct subStruct1;
        SubStruct subStruct2;
    }
    function addTestStruct(TestStruct) public pure {}
}
// ----
// Warning: Experimental features are turned on. Do not use experimental features on live deployments.
