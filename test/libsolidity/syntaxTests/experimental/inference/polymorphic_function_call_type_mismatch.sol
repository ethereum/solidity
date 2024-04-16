pragma experimental solidity;

type T(A);
type U;
type V;

function f(x: T(U)) {}

function run(a: T(V)) {
    f(a);
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 8456: (111-115): Cannot unify U and V.
