contract C {
    uint public x = msg.value - 10;
    constructor() payable {}
}

contract D {
    function f() public {
        unchecked {
            new C();
        }
    }
    function g() public payable returns (uint) {
        return (new C{value: 11}()).x();
    }
}
// ====
// compileViaYul: also
// ----
// f() -> FAILURE, hex"4e487b71", 0x11
// g(), 100 wei -> 1
// gas legacy: 101918
