library L {
    function externalFunction(uint) external pure {}
    function publicFunction(uint) public pure {}
    function internalFunction(uint) internal pure {}
}

contract C {
    using L for uint;

    function f() public pure {
        uint x;
        x.externalFunction();
        x.publicFunction();
        x.internalFunction();
    }
}
// ----
