contract C {
    function f() public view returns (uint256) {
        return msg.sender.balance;
    }
}


contract D {
    C c = new C();

    constructor() payable {}

    function f() public view returns (uint256) {
        return c.f();
    }
}

// ====
// compileViaYul: also
// ----
// constructor(), 27 wei ->
// gas ir: 351785
// gas irOptimized: 190266
// gas legacy: 261914
// gas legacyOptimized: 216889
// f() -> 27
