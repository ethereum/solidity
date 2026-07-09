contract A { uint public x; constructor() { x = 1; } }
contract B is A { constructor() { x = x * 10; } }
contract C1 is B layout at 2**256 - 2**40 { constructor() { x = x + 5; } }
contract C2 is B { constructor() { x = x + 5; } }
contract F {
    function withSpecifier() public returns (uint) {
        return new C1().x();
    }
    function withoutSpecifier() public returns (uint) {
        return new C2().x();
    }
}
// ====
// targetContract: F
// ----
// withSpecifier() -> 15
// gas legacy: 77592
// gas legacy code: 30000
// withoutSpecifier() -> 15
// gas legacy: 77502
// gas legacy code: 23600
