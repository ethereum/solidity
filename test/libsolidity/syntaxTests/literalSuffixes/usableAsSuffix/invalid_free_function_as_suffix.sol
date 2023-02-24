function suffix(uint x) pure returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 4438: (79-85): The literal suffix must be either a subdenomination or a file-level suffix function.
