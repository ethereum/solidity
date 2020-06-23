contract C {
    uint256 public result_in_constructor;
    function(uint256) returns (uint256) internal x;

    constructor() {
        x = double;
        result_in_constructor = use(2);
    }

    function double(uint256 _arg) public returns (uint256 _ret) {
        _ret = _arg * 2;
    }

    function use(uint256 _arg) public returns (uint256) {
        return x(_arg);
    }
}

// ----
// use(uint256): 3 -> 6
// result_in_constructor() -> 4
