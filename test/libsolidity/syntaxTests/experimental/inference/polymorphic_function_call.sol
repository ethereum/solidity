pragma experimental solidity;

type T;
type U(A);

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
// Info 4164: (39-49): Inferred type: tfun(?u:type, U(?u:type))
// Info 4164: (45-48): Inferred type: ?t:type
// Info 4164: (46-47): Inferred type: ?t:type
// Info 4164: (51-82): Inferred type: ('x:type, 'y:type, U('ba:type)) -> ()
// Info 4164: (61-79): Inferred type: ('x:type, 'y:type, U('ba:type))
// Info 4164: (62-63): Inferred type: 'x:type
// Info 4164: (65-69): Inferred type: 'y:type
// Info 4164: (68-69): Inferred type: 'y:type
// Info 4164: (71-78): Inferred type: U('ba:type)
// Info 4164: (74-78): Inferred type: U('ba:type)
// Info 4164: (74-75): Inferred type: tfun('ba:type, U('ba:type))
// Info 4164: (76-77): Inferred type: 'ba:type
// Info 4164: (84-159): Inferred type: (T, U(T), U(U(T))) -> ()
// Info 4164: (96-123): Inferred type: (T, U(T), U(U(T)))
// Info 4164: (97-101): Inferred type: T
// Info 4164: (100-101): Inferred type: T
// Info 4164: (103-110): Inferred type: U(T)
// Info 4164: (106-110): Inferred type: U(T)
// Info 4164: (106-107): Inferred type: tfun(T, U(T))
// Info 4164: (108-109): Inferred type: T
// Info 4164: (112-122): Inferred type: U(U(T))
// Info 4164: (115-122): Inferred type: U(U(T))
// Info 4164: (115-116): Inferred type: tfun(U(T), U(U(T)))
// Info 4164: (117-121): Inferred type: U(T)
// Info 4164: (117-118): Inferred type: tfun(T, U(T))
// Info 4164: (119-120): Inferred type: T
// Info 4164: (130-140): Inferred type: ()
// Info 4164: (130-131): Inferred type: (T, T, U(T)) -> ()
// Info 4164: (132-133): Inferred type: T
// Info 4164: (135-136): Inferred type: T
// Info 4164: (138-139): Inferred type: U(T)
// Info 4164: (146-156): Inferred type: ()
// Info 4164: (146-147): Inferred type: (U(T), U(T), U(U(T))) -> ()
// Info 4164: (148-149): Inferred type: U(T)
// Info 4164: (151-152): Inferred type: U(T)
// Info 4164: (154-155): Inferred type: U(U(T))
