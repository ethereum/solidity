pragma experimental solidity;

forall A
function f(b: B) {
    let c: C;
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 5934: (54-55): Undeclared type variable.
// TypeError 5934: (70-71): Undeclared type variable.
