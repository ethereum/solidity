==== Source: A ====
import {T as U} from "A";
import "A" as X;

type T is uint;
function f(T x) pure returns (T) { return T.wrap(T.unwrap(x) + 1); }
function g(T x) pure returns (uint) { return T.unwrap(x) + 10; }

using { f } for X.X.U global;
using { g } for T global;

function cr() pure returns (T) {}

==== Source: B ====
import { cr } from "A";

contract C {
    function f() public returns (uint) {
        return cr().f().g();
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 11
