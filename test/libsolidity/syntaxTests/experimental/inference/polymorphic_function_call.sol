pragma experimental solidity;

type T;
type U(A);

forall (X, Y)
function f(x, y: X, z: U(Y)) {}

function run(a: T, b: U(T), c: U(U(T))) {
    f(a, a, b);
    f(b, b, c);
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-38): Inferred type: T
// Info 4164: (39-49): Inferred type: tfun('t:type, U('t:type))
// Info 4164: (45-48): Inferred type: 't:type
// Info 4164: (46-47): Inferred type: 't:type
// Info 4164: (58-64): Inferred type: ('u:type, 'v:type)
// Info 4164: (59-60): Inferred type: 'u:type
// Info 4164: (62-63): Inferred type: 'v:type
// Info 4164: (65-96): Inferred type: ('y:type, 'u:type, U('v:type)) -> ()
// Info 4164: (75-93): Inferred type: ('y:type, 'u:type, U('v:type))
// Info 4164: (76-77): Inferred type: 'y:type
// Info 4164: (79-83): Inferred type: 'u:type
// Info 4164: (82-83): Inferred type: 'u:type
// Info 4164: (85-92): Inferred type: U('v:type)
// Info 4164: (88-92): Inferred type: U('v:type)
// Info 4164: (88-89): Inferred type: tfun('v:type, U('v:type))
// Info 4164: (90-91): Inferred type: 'v:type
// Info 4164: (98-173): Inferred type: (T, U(T), U(U(T))) -> ()
// Info 4164: (110-137): Inferred type: (T, U(T), U(U(T)))
// Info 4164: (111-115): Inferred type: T
// Info 4164: (114-115): Inferred type: T
// Info 4164: (117-124): Inferred type: U(T)
// Info 4164: (120-124): Inferred type: U(T)
// Info 4164: (120-121): Inferred type: tfun(T, U(T))
// Info 4164: (122-123): Inferred type: T
// Info 4164: (126-136): Inferred type: U(U(T))
// Info 4164: (129-136): Inferred type: U(U(T))
// Info 4164: (129-130): Inferred type: tfun(U(T), U(U(T)))
// Info 4164: (131-135): Inferred type: U(T)
// Info 4164: (131-132): Inferred type: tfun(T, U(T))
// Info 4164: (133-134): Inferred type: T
// Info 4164: (144-154): Inferred type: ()
// Info 4164: (144-145): Inferred type: (T, T, U(T)) -> ()
// Info 4164: (146-147): Inferred type: T
// Info 4164: (149-150): Inferred type: T
// Info 4164: (152-153): Inferred type: U(T)
// Info 4164: (160-170): Inferred type: ()
// Info 4164: (160-161): Inferred type: (U(T), U(T), U(U(T))) -> ()
// Info 4164: (162-163): Inferred type: U(T)
// Info 4164: (165-166): Inferred type: U(T)
// Info 4164: (168-169): Inferred type: U(U(T))
