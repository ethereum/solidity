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
// ----
// f() -> FAILURE
// g(), 100 wei -> 1
