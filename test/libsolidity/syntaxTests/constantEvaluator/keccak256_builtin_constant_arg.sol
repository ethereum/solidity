bytes32 constant input = "abcdefgh";
bytes32 constant k = keccak256(input);
contract C layout at k{ }
// ----
// TypeError 7556: (68-73): Invalid type for argument in function call. Invalid implicit conversion from bytes32 to bytes memory requested. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
