pragma experimental solidity;

type T(P, Q, R);
type U;
type V;

class Self: C {}
class Self: D {}

function run() {
    let x: T(U, X, Z: C);
    let y: T(V, Y, Z: D);
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-47): Inferred type: tfun((?ba:type, ?bb:type, ?bc:type), T(?ba:type, ?bb:type, ?bc:type))
// Info 4164: (37-46): Inferred type: (?x:type, ?y:type, ?z:type)
// Info 4164: (38-39): Inferred type: ?x:type
// Info 4164: (41-42): Inferred type: ?y:type
// Info 4164: (44-45): Inferred type: ?z:type
// Info 4164: (48-55): Inferred type: U
// Info 4164: (56-63): Inferred type: V
// Info 4164: (65-81): Inferred type: C
// Info 4164: (71-75): Inferred type: ?be:(type, C)
// Info 4164: (82-98): Inferred type: D
// Info 4164: (88-92): Inferred type: ?bg:(type, D)
// Info 4164: (100-170): Inferred type: () -> ()
// Info 4164: (112-114): Inferred type: ()
// Info 4164: (125-141): Inferred type: T(U, ?bm:type, ?bo:(type, C))
// Info 4164: (128-141): Inferred type: T(U, ?bm:type, ?bo:(type, C))
// Info 4164: (128-129): Inferred type: tfun((U, ?bm:type, ?bo:(type, C)), T(U, ?bm:type, ?bo:(type, C)))
// Info 4164: (130-131): Inferred type: U
// Info 4164: (133-134): Inferred type: ?bm:type
// Info 4164: (136-140): Inferred type: ?bo:(type, C)
// Info 4164: (136-137): Inferred type: ?bo:(type, C)
// Info 4164: (139-140): Inferred type: ?bo:(type, C)
// Info 4164: (151-167): Inferred type: T(V, ?bt:type, ?bv:(type, D))
// Info 4164: (154-167): Inferred type: T(V, ?bt:type, ?bv:(type, D))
// Info 4164: (154-155): Inferred type: tfun((V, ?bt:type, ?bv:(type, D)), T(V, ?bt:type, ?bv:(type, D)))
// Info 4164: (156-157): Inferred type: V
// Info 4164: (159-160): Inferred type: ?bt:type
// Info 4164: (162-166): Inferred type: ?bv:(type, D)
// Info 4164: (162-163): Inferred type: ?bv:(type, D)
// Info 4164: (165-166): Inferred type: ?bv:(type, D)
