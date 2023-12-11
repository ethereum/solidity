pragma experimental solidity;

type T(A);

class Self: C {}

instantiation T(A: D): C {}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 9789: (80-81): Undeclared identifier. Expected type class name.
