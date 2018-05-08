pragma experimental "v0.5.0";

contract C {
    function () pure returns (uint) x;
    uint constant y = x();
}
// ----
// TypeError: (105-108): Initial value for constant variable has to be compile-time constant.
