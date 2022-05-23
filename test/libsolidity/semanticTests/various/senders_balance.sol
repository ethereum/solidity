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
// gas irOptimized: 170627
// gas legacy: 222977
// gas legacyOptimized: 169779
// f() -> 27
