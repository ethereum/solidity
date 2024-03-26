contract C {
    struct S {
        uint256 x;
    }

    function f() public view returns(bytes32) { // can be pure
        return type(S).typehash;
    }
}
// ----
// Warning 2018: (58-155): Function state mutability can be restricted to pure
