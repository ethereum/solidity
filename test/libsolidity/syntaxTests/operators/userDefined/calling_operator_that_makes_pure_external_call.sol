type Int is int16;

using {add as +} for Int;

function add(Int, Int) pure returns (Int) {
    return b.f();
}

contract B {
    function f() external pure returns (Int) {}
}

contract C {
    function test() public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// DeclarationError 7576: (102-103): Undeclared identifier.
