type T is bool;
library L{
    T constant c = T.wrap(true);
    uint constant z = 42;
}
T constant g = T.wrap(true);

contract C {
    T constant b = T.wrap(true);
    uint constant X = 42;

    function f() external pure {
        T x = T.wrap(true);
        assert(T.unwrap(x)); // should hold

        assert(L.z == 42); // should hold
        assert(T.unwrap(L.c)); // should hold

        assert(T.unwrap(g)); // should hold

        assert(C.X == 42); // should hold
        assert(T.unwrap(b)); // should hold
        assert(T.unwrap(C.b)); // should hold
    }
}
// ----
// Info 1391: CHC: 7 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
