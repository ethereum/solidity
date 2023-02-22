contract Base {
    uint256 data;

    function setData(uint256 i) public {
        data = i;
    }

    function getViaBase() public returns (uint256 i) {
        return data;
    }
}


contract A is Base {
    function setViaA(uint256 i) public {
        setData(i);
    }
}


contract B is Base {
    function getViaB() public returns (uint256 i) {
        return getViaBase();
    }
}


contract Derived is Base, B, A {}
// ----
// getViaB() -> 0
// setViaA(uint256): 23 ->
// getViaB() -> 23
