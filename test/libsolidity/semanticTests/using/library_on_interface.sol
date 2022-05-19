using L for I;
interface I { function f() external pure returns (uint); }
library L {
    function execute(I i) internal pure returns (uint) {
        return i.f();
    }
}
contract C is I {
    function x() public view returns (uint) {
        I i = this;
        return i.execute();
    }
    function f() public pure returns (uint) { return 7; }
}
// ----
// x() -> 7
