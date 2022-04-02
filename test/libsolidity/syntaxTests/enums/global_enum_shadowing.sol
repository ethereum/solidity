enum E { A }
contract C {
    enum E { A }
    function f() public pure {
        E e = E.A;
        e;
    }
}
// ----
// Warning 2519: (30-42='enum E { A }'): This declaration shadows an existing declaration.
