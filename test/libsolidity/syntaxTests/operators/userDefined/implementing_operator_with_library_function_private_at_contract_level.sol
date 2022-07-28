type Int is int128;

library L {
    using {L.privateOperator as +} for Int;
    function privateOperator(Int, Int) private pure returns (Int) {}
}
