pragma experimental solidity;

type T;
type U;

function f(x) {}

function run(a: T, b: U) {
    // NOTE: The type of f is polymorphic but the inferred type of g is not - this would be
    // let-polymorphism, which we decided not to support.
    let g = f;
    g(a);
    g(b);
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 8456: (272-276): Cannot unify T and U.
