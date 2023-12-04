pragma experimental solidity;

class Self: Class1 {}
class Self: Class2 {}

forall (A: (Class1, Class2), B: Class1)
function f(a: A: Class1, b: B: Class1) {}

forall A: Class1
function g(a: A) {}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-52): Inferred type: Class1
// Info 4164: (53-74): Inferred type: Class2
// Info 4164: (116-157): Inferred type: ('ba:(type, Class1, Class2), 'be:(type, Class1)) -> ()
// Info 4164: (126-154): Inferred type: ('ba:(type, Class1, Class2), 'be:(type, Class1))
// Info 4164: (127-139): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (141-153): Inferred type: 'be:(type, Class1)
// Info 4164: (176-195): Inferred type: 'bg:(type, Class1) -> ()
// Info 4164: (186-192): Inferred type: 'bg:(type, Class1)
// Info 4164: (187-191): Inferred type: 'bg:(type, Class1)
