function suffix(uint) pure suffix returns (uint) {}

library L {
    function add(uint, uint) internal pure returns (uint) {}
}

contract C {
    using L for *;

    uint x = (1000 suffix).add(1);
}
