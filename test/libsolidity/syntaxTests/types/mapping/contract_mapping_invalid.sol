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
// TypeError 7407: (189-190='y'): Type contract J is not implicitly convertible to expected type contract I.
// TypeError 7407: (195-196='x'): Type contract I is not implicitly convertible to expected type contract J.
// TypeError 7407: (201-202='z'): Type address is not implicitly convertible to expected type contract I.
