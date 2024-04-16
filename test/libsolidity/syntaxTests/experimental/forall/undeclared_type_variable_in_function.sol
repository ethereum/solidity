pragma experimental solidity;

type T(X);

function f(p: P, q: T(Q)) {
    let r: (R, S);
    let s: S;
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 5934: (57-58): Undeclared type variable.
// TypeError 5934: (65-66): Undeclared type variable.
// TypeError 5934: (83-84): Undeclared type variable.
// TypeError 5934: (86-87): Undeclared type variable.
// TypeError 5934: (101-102): Undeclared type variable.
