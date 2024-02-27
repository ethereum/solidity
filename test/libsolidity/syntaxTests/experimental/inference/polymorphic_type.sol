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
// Info 4164: (37-46): Inferred type: ('x:type, 'y:type, 'z:type)
// Info 4164: (38-39): Inferred type: 'x:type
// Info 4164: (41-42): Inferred type: 'y:type
// Info 4164: (44-45): Inferred type: 'z:type
// Info 4164: (48-55): Inferred type: U
// Info 4164: (56-63): Inferred type: V
// Info 4164: (65-81): Inferred type: C
// Info 4164: (71-75): Inferred type: 'k:(type, C)
// Info 4164: (82-98): Inferred type: D
// Info 4164: (88-92): Inferred type: 'l:(type, D)
// Info 4164: (107-116): Inferred type: (?bc:type, ?bd:type, ?bq:(type, C, D))
// Info 4164: (108-109): Inferred type: ?bc:type
// Info 4164: (111-112): Inferred type: ?bd:type
// Info 4164: (114-115): Inferred type: ?bq:(type, C, D)
// Info 4164: (117-187): Inferred type: () -> ()
// Info 4164: (129-131): Inferred type: ()
// Info 4164: (142-158): Inferred type: T(U, ?bc:type, ?bq:(type, C, D))
// Info 4164: (145-158): Inferred type: T(U, ?bc:type, ?bq:(type, C, D))
// Info 4164: (145-146): Inferred type: tfun((U, ?bc:type, ?bq:(type, C, D)), T(U, ?bc:type, ?bq:(type, C, D)))
// Info 4164: (147-148): Inferred type: U
// Info 4164: (150-151): Inferred type: ?bc:type
// Info 4164: (153-157): Inferred type: ?bq:(type, C, D)
// Info 4164: (153-154): Inferred type: ?bq:(type, C, D)
// Info 4164: (156-157): Inferred type: ?bq:(type, C, D)
// Info 4164: (168-184): Inferred type: T(V, ?bd:type, ?bq:(type, C, D))
// Info 4164: (171-184): Inferred type: T(V, ?bd:type, ?bq:(type, C, D))
// Info 4164: (171-172): Inferred type: tfun((V, ?bd:type, ?bq:(type, C, D)), T(V, ?bd:type, ?bq:(type, C, D)))
// Info 4164: (173-174): Inferred type: V
// Info 4164: (176-177): Inferred type: ?bd:type
// Info 4164: (179-183): Inferred type: ?bq:(type, C, D)
// Info 4164: (179-180): Inferred type: ?bq:(type, C, D)
// Info 4164: (182-183): Inferred type: ?bq:(type, C, D)
