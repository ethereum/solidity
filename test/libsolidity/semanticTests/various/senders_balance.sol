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

// ----
// constructor(), 27 wei ->
// gas irOptimized: 169371
// gas legacy: 218447
// gas legacyOptimized: 167286
// f() -> 27
