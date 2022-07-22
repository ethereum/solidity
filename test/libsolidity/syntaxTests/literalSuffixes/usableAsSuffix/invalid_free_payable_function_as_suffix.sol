function suffix(uint x) payable returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 9559: (0-49): Free functions cannot be payable.
// TypeError 4438: (77-88): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
