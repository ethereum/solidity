// This used to work in pre-0.6.0.
library Lib {
    function min(uint, uint) public returns (uint);
}
contract Test {
    function f() public {
        uint t = Lib.min(12, 7);
    }
}
// ----
// TypeError: (53-100): Library functions must be implemented if declared.
