contract C {
    function f(address a) public pure returns (address x) {
        address b = a;
        x = b;
    }
}
// ====
// compileViaYul: true
// ----
// f(address): 0x1234 -> 0x1234
