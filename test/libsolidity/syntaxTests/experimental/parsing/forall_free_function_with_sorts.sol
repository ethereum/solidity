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
// Info 4164: (37-41): Inferred type: 'k:(type, Class1)
// Info 4164: (53-74): Inferred type: Class2
// Info 4164: (59-63): Inferred type: 'l:(type, Class2)
// Info 4164: (83-115): Inferred type: ('ba:(type, Class1, Class2), 'bg:(type, Class1))
// Info 4164: (84-103): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (87-103): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (88-94): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (96-102): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (105-114): Inferred type: 'bg:(type, Class1)
// Info 4164: (108-114): Inferred type: 'bg:(type, Class1)
// Info 4164: (116-157): Inferred type: ('ba:(type, Class1, Class2), 'bg:(type, Class1)) -> ()
// Info 4164: (126-154): Inferred type: ('ba:(type, Class1, Class2), 'bg:(type, Class1))
// Info 4164: (127-139): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (130-139): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (130-131): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (133-139): Inferred type: 'ba:(type, Class1, Class2)
// Info 4164: (141-153): Inferred type: 'bg:(type, Class1)
// Info 4164: (144-153): Inferred type: 'bg:(type, Class1)
// Info 4164: (144-145): Inferred type: 'bg:(type, Class1)
// Info 4164: (147-153): Inferred type: 'bg:(type, Class1)
// Info 4164: (166-175): Inferred type: 'bi:(type, Class1)
// Info 4164: (166-175): Inferred type: 'bi:(type, Class1)
// Info 4164: (169-175): Inferred type: 'bi:(type, Class1)
// Info 4164: (176-195): Inferred type: 'bi:(type, Class1) -> ()
// Info 4164: (186-192): Inferred type: 'bi:(type, Class1)
// Info 4164: (187-191): Inferred type: 'bi:(type, Class1)
// Info 4164: (190-191): Inferred type: 'bi:(type, Class1)
