// This tests skipping the extcodesize check.

contract T {
    constructor() { this.f(); }
    function f() external {}
}
contract U {
    constructor() { this.f(); }
    function f() external returns (uint) {}
}

contract C {
    function f(uint c) external returns (uint) {
        if (c == 0) new T();
        else if (c == 1) new U();
        return 1 + c;
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f(uint256): 0 -> FAILURE
// f(uint256): 1 -> FAILURE
// f(uint256): 2 -> 3
