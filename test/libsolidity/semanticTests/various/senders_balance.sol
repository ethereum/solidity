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
// gas irOptimized: 114025
// gas irOptimized code: 53400
// gas legacy: 117834
// gas legacy code: 100600
// gas legacyOptimized: 113676
// gas legacyOptimized code: 53600
// gas ssaCFGOptimized: 113686
// gas ssaCFGOptimized code: 49000
// f() -> 27
