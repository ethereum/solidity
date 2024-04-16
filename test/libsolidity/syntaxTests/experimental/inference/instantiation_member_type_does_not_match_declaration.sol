pragma experimental solidity;

type T;
type U;

class Self: C {
    function f(self: Self);
}

instantiation T: C {
    function f(self: U) {}
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 7428: (95-144): Instantiation function 'f' does not match the declaration in the type class (U -> () != T -> ()).
