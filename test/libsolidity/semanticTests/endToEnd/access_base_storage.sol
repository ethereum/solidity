contract Base {
    uint dataBase;

    function getViaBase() public returns(uint i) {
        return dataBase;
    }
}
contract Derived is Base {
    uint dataDerived;

    function setData(uint base, uint derived) public returns(bool r) {
        dataBase = base;
        dataDerived = derived;
        return true;
    }

    function getViaDerived() public returns(uint base, uint derived) {
        base = dataBase;
        derived = dataDerived;
    }
}

// ====
// compileViaYul: also
// ----
// setData(uint256,uint256): 1, 2 -> true
// setData(uint256,uint256):"1, 2" -> "1"
// getViaBase() -> 1
// getViaBase():"" -> "1"
// getViaDerived() -> 1, 2
// getViaDerived():"" -> "1, 2"
