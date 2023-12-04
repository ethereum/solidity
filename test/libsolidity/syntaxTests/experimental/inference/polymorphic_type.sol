pragma experimental solidity;

type T(P, Q, R);
type U;
type V;

class Self: C {}
class Self: D {}

forall (X, Y, Z)
function run() {
    let x: T(U, X, Z: C);
    let y: T(V, Y, Z: D);
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-47): Inferred type: tfun(('x:type, 'y:type, 'z:type), T('x:type, 'y:type, 'z:type))
// Info 4164: (48-55): Inferred type: U
// Info 4164: (56-63): Inferred type: V
// Info 4164: (65-81): Inferred type: C
// Info 4164: (82-98): Inferred type: D
// Info 4164: (117-187): Inferred type: () -> ()
// Info 4164: (129-131): Inferred type: ()
// Info 4164: (142-158): Inferred type: T(U, ?bc:type, ?bo:(type, C, D))
// Info 4164: (168-184): Inferred type: T(V, ?bd:type, ?bo:(type, C, D))
