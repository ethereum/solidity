contract C {
    struct Y {
        Y[] x;
    }
    mapping(uint256 => Y) public m;
}
// ----
// TypeError: (53-83): Internal or recursive type is not allowed for public state variables.
