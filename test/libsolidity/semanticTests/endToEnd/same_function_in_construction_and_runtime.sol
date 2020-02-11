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
// runtime(uint256): 3 -> 6
// initial() -> 4
