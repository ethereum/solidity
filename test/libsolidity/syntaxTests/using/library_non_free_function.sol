library L {
    function id(uint x) internal pure returns(uint) {
        return x;
    }
}

contract C {
    using {L.id} for uint;
    function f(uint x) external pure {
        x.id();
    }
}
// ----
