contract X {
    function f() public returns (uint x) {
        x = g();
    }
    function g() public returns (uint x) {
        x = 2;
    }
}
contract C is X {
    function f1() public returns (uint x) {
        // direct call
        x = g();
    }
    function f2() public returns (uint x) {
        // call via base
        x = f();
    }
    function f3() public returns (uint x) {
        // explicit call via base
        //x = super.g();
    }
    function g() public returns (uint x) {
        x = 3;
    }
}
// ===
// compileViaYul: true
// ----
// f() -> 3
// f1() -> 3
// f2() -> 3
// g() -> 3
