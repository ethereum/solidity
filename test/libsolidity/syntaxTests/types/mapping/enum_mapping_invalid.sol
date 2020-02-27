enum E { A, B, C }
contract C {
    mapping(E => bool) e;
    function f(uint256 a, uint8 b) public view returns (bool, bool) {
        return (e[a], e[b]);
    }
}
// ----
// TypeError: (146-147): Type uint256 is not implicitly convertible to expected type enum E.
// TypeError: (152-153): Type uint8 is not implicitly convertible to expected type enum E.
