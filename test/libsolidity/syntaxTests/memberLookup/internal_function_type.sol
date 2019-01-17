contract C {
    function () internal returns (uint) x;
    constructor() public {
        C.x = g;
    }
    function g() public pure returns (uint) {}
}
