contract C {
    struct Y {
        Y[] x;
    }
    mapping(uint256 => Y) public m;
}
// ----
// TypeError 6744: (53-83='mapping(uint256 => Y) public m'): Internal or recursive type is not allowed for public state variables.
