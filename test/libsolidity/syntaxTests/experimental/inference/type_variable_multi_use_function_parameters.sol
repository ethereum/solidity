pragma experimental solidity;

type T;
type U;

forall X
function f(x: X, y: X) {}

function test(t: T, u: U) {
    f(t, u);
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 8456: (116-123): Cannot unify T and U.
