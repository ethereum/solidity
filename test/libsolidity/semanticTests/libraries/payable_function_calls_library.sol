library L {
    function f() public returns (uint) { return 7; }
}
contract C {
    function f() public payable returns (uint) {
        return L.f();
    }
}
// ====
// compileToEwasm: false
// ----
// library: L
// f(): 27 -> 7
