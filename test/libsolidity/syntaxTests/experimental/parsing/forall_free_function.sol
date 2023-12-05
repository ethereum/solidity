pragma experimental solidity;

forall (A)
function f(a: A) {}

forall (A, B)
function g(a: A, b: B) {}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (42-61): Inferred type: 's:type -> ()
// Info 4164: (52-58): Inferred type: 's:type
// Info 4164: (53-57): Inferred type: 's:type
// Info 4164: (77-102): Inferred type: ('t:type, 'u:type) -> ()
// Info 4164: (87-99): Inferred type: ('t:type, 'u:type)
// Info 4164: (88-92): Inferred type: 't:type
// Info 4164: (94-98): Inferred type: 'u:type
