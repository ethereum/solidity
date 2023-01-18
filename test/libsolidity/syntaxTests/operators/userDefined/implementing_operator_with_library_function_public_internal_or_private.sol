type Int is int128;

library L {
    function publicOperator(Int, Int) public pure returns (Int) {}
    function internalOperator(Int, Int) internal pure returns (Int) {}
}

using {L.publicOperator as +} for Int;
using {L.internalOperator as +} for Int;
