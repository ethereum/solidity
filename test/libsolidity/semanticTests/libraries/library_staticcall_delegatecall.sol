library Lib {
    function x() public view returns (uint) {
        return 1;
    }
}
contract Test {
    uint t;
    function f() public returns (uint) {
        t = 2;
        return this.g();
    }
    function g() public view returns (uint) {
        return Lib.x();
    }
}
// ====
// compileViaYul: also
// ----
// library: Lib
// f() -> 1
