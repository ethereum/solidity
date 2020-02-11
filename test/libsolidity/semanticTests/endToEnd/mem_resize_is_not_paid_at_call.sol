contract C {
    function f() public returns(uint[200] memory) {}
}
contract D {
    function f(C c) public returns(uint) {
        c.f();
        return 7;
    }
}

// ----

contract C {
    function f() public returns(uint[200] memory) {}
}
contract D {
    function f(C c) public returns(uint) {
        c.f();
        return 7;
    }
}

// ----
// f(address): cAddr -> 7
// f(address):"0" -> "7"
