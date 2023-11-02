pragma experimental solidity;

forall ()
function f(x: ()) {}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (38-40): Inferred type: ()
// Info 4164: (41-61): Inferred type: () -> ()
// Info 4164: (51-58): Inferred type: ()
// Info 4164: (52-57): Inferred type: ()
// Info 4164: (55-57): Inferred type: ()
