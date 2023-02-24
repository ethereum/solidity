function suffix(uint x) payable returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 9559: (0-49): Free functions cannot be payable.
// TypeError 4438: (77-88): The literal suffix must be either a subdenomination or a file-level suffix function.
