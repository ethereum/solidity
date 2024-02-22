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
// gas irOptimized: 113776
// gas irOptimized code: 52800
// gas legacy: 117835
// gas legacy code: 100600
// gas legacyOptimized: 113676
// gas legacyOptimized code: 53600
// f() -> 27
