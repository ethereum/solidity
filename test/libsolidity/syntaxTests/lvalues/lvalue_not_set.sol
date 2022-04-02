contract C {
    function foo(uint x) public
    {
        // Used to cause an ICE
        uint p = new uint[] = x;
    }
}
// ----
// TypeError 4247: (100-110='new uint[]'): Expression has to be an lvalue.
// TypeError 7407: (113-114='x'): Type uint256 is not implicitly convertible to expected type function (uint256) pure returns (uint256[] memory).
// TypeError 9574: (91-114='uint p = new uint[] = x'): Type function (uint256) pure returns (uint256[] memory) is not implicitly convertible to expected type uint256.
