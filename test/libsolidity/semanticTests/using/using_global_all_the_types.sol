==== Source: A ====
enum E {A, B}
struct S { uint x; }
type T is uint;
using L for E global;
using L for S global;
using L for T global;
library L {
    function f(E e) internal pure returns (uint) {
        return uint(e);
    }
    function f(S memory s) internal pure returns (uint) {
        return s.x;
    }
    function f(T t) internal pure returns (uint) {
        return T.unwrap(t);
    }
}

==== Source: B ====
contract C {
    function f() public pure returns (uint a, uint b, uint c) {
        E e = E.B;
        a = e.f();
        S memory s;
        s.x = 7;
        b = s.f();
        T t = T.wrap(9);
        c = t.f();
    }
}

import {E, S, T} from "A";

// ----
// f() -> 1, 7, 9
