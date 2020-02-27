interface I {}
contract J {}
contract C {
    mapping(I => bool) i;
    mapping(J => bool) j;
    function f(I x, J y) public view returns (bool, bool) {
        return (i[x], j[y]);
    }
}
