pragma abicoder               v2;

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
