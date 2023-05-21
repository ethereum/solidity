contract C {
    struct S {
        uint256 x;
    }

    function f() public pure returns(bytes32) {
        return type(S).typehash;
    }
}