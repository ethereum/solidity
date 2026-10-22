bytes32 constant k = keccak256("abcdefgh");
contract C layout at k{ }
// ----
// TypeError 1763: (65-66): The base slot of the storage layout must evaluate to an integer (the type is 'bytes32' instead).
