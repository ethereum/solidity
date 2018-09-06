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
    function addTestStruct(TestStruct memory) public pure {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
