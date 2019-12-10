enum E { A }
contract C {
    function f() public pure {
        E e = E.A;
        e;
    }
}