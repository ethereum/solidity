type Int is int128;

library L {
    function privateOperator(Int, Int) private pure returns (Int) {}
}

using {L.privateOperator as +} for Int;
// ----
// TypeError 6772: (112-129): Function "L.privateOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
