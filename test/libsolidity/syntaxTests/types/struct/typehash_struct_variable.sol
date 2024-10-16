contract C {
    struct S {
        uint256 x;
    }

    function f1(S calldata s) public pure returns(bytes32 h) {
        h = type(s).typehash; // should fail
    }

    function f3(S calldata s) public pure returns(bytes32 h) {
        h = type(S).typehash;
    }
}
// ----
// TypeError 4259: (134-135): Invalid type for argument in the function call. An enum type, contract type, struct type or an integer type is required, but struct C.S calldata provided.
