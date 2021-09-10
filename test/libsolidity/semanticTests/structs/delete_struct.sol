contract test {
    struct topStruct {
        nestedStruct nstr;
        uint topValue;
    }
    uint toDelete;
    topStruct str;
    struct nestedStruct {
        uint nestedValue;
    }
    constructor() {
        toDelete = 5;
        str.topValue = 1;

        str.nstr.nestedValue = 2;
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
}
// ====
// compileViaYul: also
// ----
// getToDelete() -> 0
// getTopValue() -> 0
// getNestedValue() -> 0 #mapping values should be the same#
