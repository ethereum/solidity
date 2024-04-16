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
// Info 4164: (38-41): Inferred type: 's:type
// Info 4164: (39-40): Inferred type: 's:type
// Info 4164: (42-61): Inferred type: 's:type -> ()
// Info 4164: (52-58): Inferred type: 's:type
// Info 4164: (53-57): Inferred type: 's:type
// Info 4164: (56-57): Inferred type: 's:type
// Info 4164: (70-76): Inferred type: ('v:type, 'w:type)
// Info 4164: (71-72): Inferred type: 'v:type
// Info 4164: (74-75): Inferred type: 'w:type
// Info 4164: (77-102): Inferred type: ('v:type, 'w:type) -> ()
// Info 4164: (87-99): Inferred type: ('v:type, 'w:type)
// Info 4164: (88-92): Inferred type: 'v:type
// Info 4164: (91-92): Inferred type: 'v:type
// Info 4164: (94-98): Inferred type: 'w:type
// Info 4164: (97-98): Inferred type: 'w:type
