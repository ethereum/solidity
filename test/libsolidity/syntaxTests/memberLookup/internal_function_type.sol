contract C {
    function () internal returns (uint) x;
    constructor() {
        C.x = g;
    }
    function g() public pure returns (uint) {}
}
// ----
