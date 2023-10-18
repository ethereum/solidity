pragma experimental solidity;

type bool = __builtin("bool");

type T(A);
type int;
type str;

class Self: C {
    function foo(a: Self, b: Self) -> Self;
}

class Self: P1 {}
class Self: P2 {}
class Self: P3 {}
class Self: P4 {}

instantiation int: P1 {}
instantiation int: P2 {}
instantiation int: P3 {}

instantiation str: P1 {}
instantiation str: P2 {}
instantiation str: P4 {}

instantiation T(A: P1): + {
    // FIXME: Type comparison with type class function fails because we get
    // two different variables for A.
    function add(x: T(A), y: T(A)) -> T(A) {}
}

instantiation T(A: P2): == {
    function eq(x: T(A), y: T(A)) -> bool {}
}

instantiation T(A: (P1, P2)): C {
    function foo(x: T(A), y: T(A)) -> T(A) {}
}

function fun(a: T(int: P3), b: T(str: P4)) {
    a + a;
    b + b;

    a == a;
    b == b;

    C.foo(a, a);
    C.foo(b, b);
}
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 7428: (651-732): Instantiation function 'foo' does not match the declaration in the type class ((T("o:(type, P1, P2)), T("o:(type, P1, P2))) -> T("o:(type, P1, P2)) != (T('v:(type, P1, P2)), T('v:(type, P1, P2))) -> T('v:(type, P1, P2))).
// TypeError 7428: (383-572): Instantiation function 'add' does not match the declaration in the type class ((T("p:(type, P1)), T("p:(type, P1))) -> T("p:(type, P1)) != (T('bh:(type, P1)), T('bh:(type, P1))) -> T('bh:(type, P1))).
// TypeError 7428: (574-649): Instantiation function 'eq' does not match the declaration in the type class ((T("q:(type, P2)), T("q:(type, P2))) -> bool != (T('br:(type, P2)), T('br:(type, P2))) -> bool).
