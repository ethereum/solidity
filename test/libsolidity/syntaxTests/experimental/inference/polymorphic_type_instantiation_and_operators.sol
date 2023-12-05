pragma experimental solidity;

type bool = __builtin("bool");

type T(A);
type int;
type str;

class Self: C {
    function foo(a: Self, b: Self) -> Self;
}

class Self: P1 {}
class Self: P2 {}
class Self: P3 {}
class Self: P4 {}

instantiation int: P1 {}
instantiation int: P2 {}
instantiation int: P3 {}

instantiation str: P1 {}
instantiation str: P2 {}
instantiation str: P4 {}

instantiation T(A: P1): + {
    function add(x: T(A), y: T(A)) -> T(A) {}
}

instantiation T(A: P2): == {
    function eq(x: T(A), y: T(A)) -> bool {}
}

instantiation T(A: (P1, P2)): C {
    function foo(x: T(A), y: T(A)) -> T(A) {}
}

function fun(a: T(int: P3), b: T(str: P4)) {
    a + a;
    b + b;

    a == a;
    b == b;

    C.foo(a, a);
    C.foo(b, b);
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-61): Inferred type: bool
// Info 4164: (63-73): Inferred type: tfun('y:type, T('y:type))
// Info 4164: (74-83): Inferred type: int
// Info 4164: (84-93): Inferred type: str
// Info 4164: (95-156): Inferred type: C
// Info 4164: (115-154): Inferred type: ('k:(type, C), 'k:(type, C)) -> 'k:(type, C)
// Info 4164: (127-145): Inferred type: ('k:(type, C), 'k:(type, C))
// Info 4164: (128-135): Inferred type: 'k:(type, C)
// Info 4164: (137-144): Inferred type: 'k:(type, C)
// Info 4164: (158-175): Inferred type: P1
// Info 4164: (176-193): Inferred type: P2
// Info 4164: (194-211): Inferred type: P3
// Info 4164: (212-229): Inferred type: P4
// Info 4164: (415-456): Inferred type: (T('bf:(type, P1)), T('bf:(type, P1))) -> T('bf:(type, P1))
// Info 4164: (427-445): Inferred type: (T('bf:(type, P1)), T('bf:(type, P1)))
// Info 4164: (428-435): Inferred type: T('bf:(type, P1))
// Info 4164: (437-444): Inferred type: T('bf:(type, P1))
// Info 4164: (493-533): Inferred type: (T('bn:(type, P2)), T('bn:(type, P2))) -> bool
// Info 4164: (504-522): Inferred type: (T('bn:(type, P2)), T('bn:(type, P2)))
// Info 4164: (505-512): Inferred type: T('bn:(type, P2))
// Info 4164: (514-521): Inferred type: T('bn:(type, P2))
// Info 4164: (575-616): Inferred type: (T('bw:(type, P1, P2)), T('bw:(type, P1, P2))) -> T('bw:(type, P1, P2))
// Info 4164: (587-605): Inferred type: (T('bw:(type, P1, P2)), T('bw:(type, P1, P2)))
// Info 4164: (588-595): Inferred type: T('bw:(type, P1, P2))
// Info 4164: (597-604): Inferred type: T('bw:(type, P1, P2))
// Info 4164: (620-748): Inferred type: (T(int), T(str)) -> ()
// Info 4164: (632-662): Inferred type: (T(int), T(str))
// Info 4164: (633-646): Inferred type: T(int)
// Info 4164: (648-661): Inferred type: T(str)
// Info 4164: (669-674): Inferred type: T(int)
// Info 4164: (669-670): Inferred type: T(int)
// Info 4164: (673-674): Inferred type: T(int)
// Info 4164: (680-685): Inferred type: T(str)
// Info 4164: (680-681): Inferred type: T(str)
// Info 4164: (684-685): Inferred type: T(str)
// Info 4164: (692-698): Inferred type: bool
// Info 4164: (692-693): Inferred type: T(int)
// Info 4164: (697-698): Inferred type: T(int)
// Info 4164: (704-710): Inferred type: bool
// Info 4164: (704-705): Inferred type: T(str)
// Info 4164: (709-710): Inferred type: T(str)
// Info 4164: (717-728): Inferred type: T(int)
// Info 4164: (717-722): Inferred type: (T(int), T(int)) -> T(int)
// Info 4164: (717-718): Inferred type: C
// Info 4164: (723-724): Inferred type: T(int)
// Info 4164: (726-727): Inferred type: T(int)
// Info 4164: (734-745): Inferred type: T(str)
// Info 4164: (734-739): Inferred type: (T(str), T(str)) -> T(str)
// Info 4164: (734-735): Inferred type: C
// Info 4164: (740-741): Inferred type: T(str)
// Info 4164: (743-744): Inferred type: T(str)
