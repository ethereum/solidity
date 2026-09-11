struct S {
    uint256 v;
}

library L {
    function doubleInternal(S memory s) pure internal returns(uint256) {
        return 2 * s.v;
    }

    function doubleExternal(S memory s) pure external returns(uint256) {
        return 2 * s.v;
    }

    function doublePublic(S memory s) pure public returns(uint256) {
        return 2 * s.v;
    }
}

using L for S;

contract C
{
    uint256 constant s = S(1).v;

    function test() pure private {
        S(1);
        S(1).v;
        S(1).doubleInternal;
        S(1).doubleExternal;
        S(1).doublePublic;
    }
}
// ----
// Warning 6133: (457-461): Statement has no effect.
// Warning 6133: (471-477): Statement has no effect.
// Warning 6133: (487-506): Statement has no effect.
// Warning 6133: (516-535): Statement has no effect.
// Warning 6133: (545-562): Statement has no effect.
