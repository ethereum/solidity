contract C {
    uint public i;
    constructor(uint newI) public {
        i = newI;
    }
}
contract D {
    C c;
    constructor(uint v) public {
        c = new C(v);
    }
    function f() public returns (uint r) {
        return c.i();
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): 2 ->
// f() -> 2
