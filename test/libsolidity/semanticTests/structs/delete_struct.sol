contract test {
    struct topStruct {
        nestedStruct nstr;
        uint topValue;
        mapping (uint => uint) topMapping;
    }
    uint toDelete;
    topStruct str;
    struct nestedStruct {
        uint nestedValue;
        mapping (uint => bool) nestedMapping;
    }
    constructor() public {
        toDelete = 5;
        str.topValue = 1;
        str.topMapping[0] = 1;
        str.topMapping[1] = 2;

        str.nstr.nestedValue = 2;
        str.nstr.nestedMapping[0] = true;
        str.nstr.nestedMapping[1] = false;
        delete str;
        delete toDelete;
    }
    function getToDelete() public returns (uint res){
        res = toDelete;
    }
    function getTopValue() public returns(uint topValue){
        topValue = str.topValue;
    }
    function getNestedValue() public returns(uint nestedValue){
        nestedValue = str.nstr.nestedValue;
    }
    function getTopMapping(uint index) public returns(uint ret) {
        ret = str.topMapping[index];
    }
    function getNestedMapping(uint index) public returns(bool ret) {
        return str.nstr.nestedMapping[index];
    }
}
// ----
// getToDelete() -> 0
// getTopValue() -> 0
// getNestedValue() -> 0
// #mapping values should be the same#
// getTopMapping(uint256): 0 -> 1
// getTopMapping(uint256): 1 -> 2
// getNestedMapping(uint256): 0 -> true
// getNestedMapping(uint256): 1 -> false
