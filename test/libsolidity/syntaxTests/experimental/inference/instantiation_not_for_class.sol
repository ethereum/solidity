pragma experimental solidity;

type T;
type U;

instantiation T: U {}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 3570: (65-66): Expected a type class.
