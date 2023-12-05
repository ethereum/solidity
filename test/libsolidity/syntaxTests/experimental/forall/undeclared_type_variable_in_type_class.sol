pragma experimental solidity;

type T(X);

class Self: C {
    function f(self: Self);
}

instantiation T(Y): C {
    function f(self: T(Z)) {}
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 5934: (137-138): Undeclared type variable.
