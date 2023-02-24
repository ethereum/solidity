function suffix(uint x) returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 4438: (69-80): The literal suffix must be either a subdenomination or a file-level suffix function.
