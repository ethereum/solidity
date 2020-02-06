enum E { A, B, C }
contract C {
    mapping(E => bool) e;
    function f(E v) public view returns (bool, bool) {
        return (e[v], e[E.A]);
    }
}
