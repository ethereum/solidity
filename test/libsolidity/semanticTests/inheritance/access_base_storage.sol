contract Base {
    uint256 dataBase;

    function getViaBase() public returns (uint256 i) {
        return dataBase;
    }
}


contract Derived is Base {
    uint256 dataDerived;

    function setData(uint256 base, uint256 derived) public returns (bool r) {
        dataBase = base;
        dataDerived = derived;
        return true;
    }

    function getViaDerived() public returns (uint256 base, uint256 derived) {
        base = dataBase;
        derived = dataDerived;
    }
}
// ----
// setData(uint256,uint256): 1, 2 -> true
// getViaBase() -> 1
// getViaDerived() -> 1, 2
