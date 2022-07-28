type Int is int;

library L {
    using {add as +} for Int;

    function add(Int, Int) internal pure returns (Int) {}
    function unsub(Int) internal pure returns (Int) {}
}
