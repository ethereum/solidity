interface I {}
contract J {}
contract C {
    mapping(I => bool) i;
    mapping(J => bool) j;
    function f(I x, J y, address z) public view returns (bool, bool, bool) {
        return (i[y], j[x], i[z]);
    }
}
// ----
// TypeError: (189-190): Type contract J is not implicitly convertible to expected type contract I.
// TypeError: (195-196): Type contract I is not implicitly convertible to expected type contract J.
// TypeError: (201-202): Type address is not implicitly convertible to expected type contract I.
