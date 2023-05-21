struct S {
    uint256 x;
}

bytes32 constant TYPE_HASH_FILE_LEVEL = type(S).typehash;

contract A {
    function f() public pure {
        // TYPE_HASH_FILE_LEVEL == keccak256("S(uint256 x)")
        assert(TYPE_HASH_FILE_LEVEL == 0x2a7af8c10b1d48ad8e0a6aad976d8385e84377b5bd03b59e2c445dc430ac2ca2);
    }
}

contract C {
    struct S {
        uint256 y;
    }

    function f() public pure {
        // type(S).typehash == keccak256("S(uint256 y)") and not the shadowed file-level struct S
        assert(type(S).typehash == 0xea24952476a382c98f2d2e42112ff8a673d8ed19d22ffc89ee9fe2f415bf6c35);
        assert(type(S).typehash != TYPE_HASH_FILE_LEVEL);
    }
}
// ----
// Warning 2519: (327-362): This declaration shadows an existing declaration.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
