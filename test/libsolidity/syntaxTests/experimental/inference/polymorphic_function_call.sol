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
// Info 4164: (65-96): Inferred type: ('bg:type, 'u:type, U('v:type)) -> ()
// Info 4164: (75-93): Inferred type: ('bg:type, 'u:type, U('v:type))
// Info 4164: (76-77): Inferred type: 'bg:type
// Info 4164: (79-83): Inferred type: 'u:type
// Info 4164: (85-92): Inferred type: U('v:type)
// Info 4164: (98-173): Inferred type: (T, U(T), U(U(T))) -> ()
// Info 4164: (110-137): Inferred type: (T, U(T), U(U(T)))
// Info 4164: (111-115): Inferred type: T
// Info 4164: (117-124): Inferred type: U(T)
// Info 4164: (126-136): Inferred type: U(U(T))
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
