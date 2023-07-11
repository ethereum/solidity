using {L.externalFunction, L.publicFunction, L.internalFunction} for uint;

library L {
    function externalFunction(uint) external pure {}
    function publicFunction(uint) public pure {}
    function internalFunction(uint) internal pure {}

    function f() public pure {
        uint x;
        x.externalFunction();
        x.publicFunction();
        x.internalFunction();
    }
}
// ----
// TypeError 6700: (299-319): Libraries cannot call their own functions externally.
// TypeError 6700: (329-347): Libraries cannot call their own functions externally.
