enum E { A }
contract C {
    enum E { A }
    function f() public pure {
        E e = E.A;
        e;
    }
}
// ----
// Warning: (30-42): This declaration shadows an existing declaration.
