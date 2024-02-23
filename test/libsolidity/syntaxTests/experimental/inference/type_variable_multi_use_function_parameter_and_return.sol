pragma experimental solidity;

type T;
type U;

forall X
function f(x: X) -> X {}

function test(t: T, u: U) {
    t = f(u);
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 8456: (115-123): Cannot unify T and U.
