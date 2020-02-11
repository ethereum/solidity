contract C {
    uint public initial;
    constructor() public {
        initial = double(2);
    }

    function double(uint _arg) public returns(uint _ret) {
        _ret = _arg * 2;
    }

    function runtime(uint _arg) public returns(uint) {
        return double(_arg);
    }
}

// ----
// runtime(uint256): encodeArgs(3)) -> 6
// runtime(uint256):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3]" -> "6"
// initial() -> 4
// initial():"" -> "4"
