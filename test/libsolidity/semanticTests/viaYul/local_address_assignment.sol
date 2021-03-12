contract C {
    function f(address a) public pure returns (address x) {
        address b = a;
        x = b;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(address): 0x1234 -> 0x1234
