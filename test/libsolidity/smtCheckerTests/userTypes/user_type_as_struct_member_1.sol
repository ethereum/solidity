contract C {
    type T is address;

    struct S {
        T a;
        uint x;
    }

    mapping (T => S) m;

    constructor() {
        m[T.wrap(address(0))] = S(T.wrap(address(0)), 0);
    }

    function set(address _a) external {
        m[T.wrap(_a)] = S(T.wrap(_a), 0);
    }

    function inv(T t) external view {
        assert( // should hold
            T.unwrap(m[t].a) == T.unwrap(t) ||
            T.unwrap(m[t].a) == address(0)
        );
    }
}
// ----
