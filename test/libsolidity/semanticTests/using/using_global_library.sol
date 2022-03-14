==== Source: A ====
type T is uint;
using L for T global;
library L {
    function inc(T x) internal pure returns (T) {
        return T.wrap(T.unwrap(x) + 1);
    }
    function dec(T x) external pure returns (T) {
        return T.wrap(T.unwrap(x) - 1);
    }
}

==== Source: B ====
contract C {
    function f() public pure returns (T r1, T r2) {
        r1 = r1.inc().inc();
        r2 = r1.dec();
    }
}

import {T} from "A";

// ====
// compileViaYul: also
// ----
// library: "A":L
// f() -> 2, 1
