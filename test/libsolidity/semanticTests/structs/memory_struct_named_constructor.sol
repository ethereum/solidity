pragma experimental ABIEncoderV2;

contract C {
    struct S {
        uint256 a;
        bool x;
    }

    function s() public returns(S memory)
    {
        return S({x: true, a: 8});
    }
}

// ====
// compileViaYul: also
// ----
// s() -> 8, true
