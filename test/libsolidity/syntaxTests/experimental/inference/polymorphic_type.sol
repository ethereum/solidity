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
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-47): Inferred type: tfun(('p:type, 'q:type, 'r:type), T('p:type, 'q:type, 'r:type))
// Info 4164: (37-46): Inferred type: ('m:type, 'n:type, 'o:type)
// Info 4164: (38-39): Inferred type: 'm:type
// Info 4164: (41-42): Inferred type: 'n:type
// Info 4164: (44-45): Inferred type: 'o:type
// Info 4164: (48-55): Inferred type: U
// Info 4164: (56-63): Inferred type: V
// Info 4164: (65-81): Inferred type: C
// Info 4164: (71-75): Inferred type: "b:(type, C)
// Info 4164: (82-98): Inferred type: D
// Info 4164: (88-92): Inferred type: "c:(type, D)
// Info 4164: (100-170): Inferred type: () -> ()
// Info 4164: (112-114): Inferred type: ()
// Info 4164: (125-141): Inferred type: T(U, 'z:type, 'bb:(type, C))
// Info 4164: (128-141): Inferred type: T(U, 'z:type, 'bb:(type, C))
// Info 4164: (128-129): Inferred type: tfun((U, 'z:type, 'bb:(type, C)), T(U, 'z:type, 'bb:(type, C)))
// Info 4164: (130-131): Inferred type: U
// Info 4164: (133-134): Inferred type: 'z:type
// Info 4164: (136-140): Inferred type: 'bb:(type, C)
// Info 4164: (136-137): Inferred type: 'bb:(type, C)
// Info 4164: (139-140): Inferred type: 'bb:(type, C)
// Info 4164: (151-167): Inferred type: T(V, 'bg:type, 'bi:(type, D))
// Info 4164: (154-167): Inferred type: T(V, 'bg:type, 'bi:(type, D))
// Info 4164: (154-155): Inferred type: tfun((V, 'bg:type, 'bi:(type, D)), T(V, 'bg:type, 'bi:(type, D)))
// Info 4164: (156-157): Inferred type: V
// Info 4164: (159-160): Inferred type: 'bg:type
// Info 4164: (162-166): Inferred type: 'bi:(type, D)
// Info 4164: (162-163): Inferred type: 'bi:(type, D)
// Info 4164: (165-166): Inferred type: 'bi:(type, D)
