pragma experimental solidity;

type T;
type U(A);

function f(x, y: X, z: U(Y)) {}

function run(a: T, b: U(T), c: U(U(T))) {
    let fRef = f;

    // FIXME: The second call does not unify. Apparently the call via a function pointer does not
    // get fresh type variables like a direct call.
    fRef(a, a, b);
    fRef(b, b, c);
}
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 8456: (318-331): Cannot unify T and U(T).
// TypeError 8456: (318-331): Cannot unify T and U(T).
// TypeError 8456: (318-331): Cannot unify T and U(T).
