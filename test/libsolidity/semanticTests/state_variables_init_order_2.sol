contract A {
    uint public x = 0;
    uint y = f();
    function f() public returns (uint256) {
        ++x;
        return 42;
    }
}
contract B is A {
    uint public z;
    constructor() public {
        z = x;
    }
}
// ====
// compileViaYul: also
// ----
// z() -> 1