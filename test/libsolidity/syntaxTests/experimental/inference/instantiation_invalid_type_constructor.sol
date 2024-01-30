pragma experimental solidity;

class Self: C {}

function f() {}

instantiation f: C {}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 9831: (80-81): Expected type declaration.
