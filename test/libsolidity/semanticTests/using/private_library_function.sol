library L {
    using {L.privateFunction} for uint;
    function privateFunction(uint x) private pure returns (uint) { return x + 1; }
    function f() public pure returns (uint) {
        uint x = 1;
        return x.privateFunction();
    }
}
// ----
// f() -> 2
