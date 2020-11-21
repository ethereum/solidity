contract C {
    uint256 public initial;

    constructor() {
        initial = double(2);
    }

    function double(uint256 _arg) public returns (uint256 _ret) {
        _ret = _arg * 2;
    }

    function runtime(uint256 _arg) public returns (uint256) {
        return double(_arg);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// runtime(uint256): 3 -> 6
// initial() -> 4
