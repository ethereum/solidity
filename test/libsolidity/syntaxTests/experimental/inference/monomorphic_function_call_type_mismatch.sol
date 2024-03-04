pragma experimental solidity;

type T;
type U;

function f(x: T, y: U) {}

function run(a: U, b: T) {
    f(a, b);
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 8456: (106-113): Cannot unify T and U.
// TypeError 8456: (106-113): Cannot unify U and T.
