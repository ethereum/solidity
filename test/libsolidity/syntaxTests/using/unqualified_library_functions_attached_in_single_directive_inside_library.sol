library L {
    function externalFunction(uint) external pure {}
    function publicFunction(uint) public pure {}
    function internalFunction(uint) internal pure {}
    function privateFunction(uint) private pure {}

    using {externalFunction, publicFunction, internalFunction, privateFunction} for uint;

    function f() public pure {
        uint x;
        x.externalFunction();
        x.publicFunction();
        x.internalFunction();
        x.privateFunction();
    }
}
// ----
// TypeError 6700: (365-385): Libraries cannot call their own functions externally.
// TypeError 6700: (395-413): Libraries cannot call their own functions externally.
