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
// Info 4164: (69-72): Inferred type: 'y:type
// Info 4164: (70-71): Inferred type: 'y:type
// Info 4164: (74-83): Inferred type: int
// Info 4164: (84-93): Inferred type: str
// Info 4164: (95-156): Inferred type: C
// Info 4164: (101-105): Inferred type: 'k:(type, C)
// Info 4164: (115-154): Inferred type: ('k:(type, C), 'k:(type, C)) -> 'k:(type, C)
// Info 4164: (127-145): Inferred type: ('k:(type, C), 'k:(type, C))
// Info 4164: (128-135): Inferred type: 'k:(type, C)
// Info 4164: (131-135): Inferred type: 'k:(type, C)
// Info 4164: (137-144): Inferred type: 'k:(type, C)
// Info 4164: (140-144): Inferred type: 'k:(type, C)
// Info 4164: (149-153): Inferred type: 'k:(type, C)
// Info 4164: (158-175): Inferred type: P1
// Info 4164: (164-168): Inferred type: 'l:(type, P1)
// Info 4164: (176-193): Inferred type: P2
// Info 4164: (182-186): Inferred type: 'm:(type, P2)
// Info 4164: (194-211): Inferred type: P3
// Info 4164: (200-204): Inferred type: 'n:(type, P3)
// Info 4164: (212-229): Inferred type: P4
// Info 4164: (218-222): Inferred type: 'o:(type, P4)
// Info 4164: (231-255): Inferred type: void
// Info 4164: (256-280): Inferred type: void
// Info 4164: (281-305): Inferred type: void
// Info 4164: (307-331): Inferred type: void
// Info 4164: (332-356): Inferred type: void
// Info 4164: (357-381): Inferred type: void
// Info 4164: (383-458): Inferred type: void
// Info 4164: (398-405): Inferred type: 'bu:(type, P1)
// Info 4164: (399-404): Inferred type: 'bu:(type, P1)
// Info 4164: (402-404): Inferred type: 'bu:(type, P1)
// Info 4164: (415-456): Inferred type: (T('bu:(type, P1)), T('bu:(type, P1))) -> T('bu:(type, P1))
// Info 4164: (427-445): Inferred type: (T('bu:(type, P1)), T('bu:(type, P1)))
// Info 4164: (428-435): Inferred type: T('bu:(type, P1))
// Info 4164: (431-435): Inferred type: T('bu:(type, P1))
// Info 4164: (431-432): Inferred type: tfun('bu:(type, P1), T('bu:(type, P1)))
// Info 4164: (433-434): Inferred type: 'bu:(type, P1)
// Info 4164: (437-444): Inferred type: T('bu:(type, P1))
// Info 4164: (440-444): Inferred type: T('bu:(type, P1))
// Info 4164: (440-441): Inferred type: tfun('bu:(type, P1), T('bu:(type, P1)))
// Info 4164: (442-443): Inferred type: 'bu:(type, P1)
// Info 4164: (449-453): Inferred type: T('bu:(type, P1))
// Info 4164: (449-450): Inferred type: tfun('bu:(type, P1), T('bu:(type, P1)))
// Info 4164: (451-452): Inferred type: 'bu:(type, P1)
// Info 4164: (460-535): Inferred type: void
// Info 4164: (475-482): Inferred type: 'ce:(type, P2)
// Info 4164: (476-481): Inferred type: 'ce:(type, P2)
// Info 4164: (479-481): Inferred type: 'ce:(type, P2)
// Info 4164: (493-533): Inferred type: (T('ce:(type, P2)), T('ce:(type, P2))) -> bool
// Info 4164: (504-522): Inferred type: (T('ce:(type, P2)), T('ce:(type, P2)))
// Info 4164: (505-512): Inferred type: T('ce:(type, P2))
// Info 4164: (508-512): Inferred type: T('ce:(type, P2))
// Info 4164: (508-509): Inferred type: tfun('ce:(type, P2), T('ce:(type, P2)))
// Info 4164: (510-511): Inferred type: 'ce:(type, P2)
// Info 4164: (514-521): Inferred type: T('ce:(type, P2))
// Info 4164: (517-521): Inferred type: T('ce:(type, P2))
// Info 4164: (517-518): Inferred type: tfun('ce:(type, P2), T('ce:(type, P2)))
// Info 4164: (519-520): Inferred type: 'ce:(type, P2)
// Info 4164: (526-530): Inferred type: bool
// Info 4164: (537-618): Inferred type: void
// Info 4164: (552-565): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (553-564): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (556-564): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (557-559): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (561-563): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (575-616): Inferred type: (T('bi:(type, P1, P2)), T('bi:(type, P1, P2))) -> T('bi:(type, P1, P2))
// Info 4164: (587-605): Inferred type: (T('bi:(type, P1, P2)), T('bi:(type, P1, P2)))
// Info 4164: (588-595): Inferred type: T('bi:(type, P1, P2))
// Info 4164: (591-595): Inferred type: T('bi:(type, P1, P2))
// Info 4164: (591-592): Inferred type: tfun('bi:(type, P1, P2), T('bi:(type, P1, P2)))
// Info 4164: (593-594): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (597-604): Inferred type: T('bi:(type, P1, P2))
// Info 4164: (600-604): Inferred type: T('bi:(type, P1, P2))
// Info 4164: (600-601): Inferred type: tfun('bi:(type, P1, P2), T('bi:(type, P1, P2)))
// Info 4164: (602-603): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (609-613): Inferred type: T('bi:(type, P1, P2))
// Info 4164: (609-610): Inferred type: tfun('bi:(type, P1, P2), T('bi:(type, P1, P2)))
// Info 4164: (611-612): Inferred type: 'bi:(type, P1, P2)
// Info 4164: (620-748): Inferred type: (T(int), T(str)) -> ()
// Info 4164: (632-662): Inferred type: (T(int), T(str))
// Info 4164: (633-646): Inferred type: T(int)
// Info 4164: (636-646): Inferred type: T(int)
// Info 4164: (636-637): Inferred type: tfun(int, T(int))
// Info 4164: (638-645): Inferred type: int
// Info 4164: (638-641): Inferred type: int
// Info 4164: (643-645): Inferred type: int
// Info 4164: (648-661): Inferred type: T(str)
// Info 4164: (651-661): Inferred type: T(str)
// Info 4164: (651-652): Inferred type: tfun(str, T(str))
// Info 4164: (653-660): Inferred type: str
// Info 4164: (653-656): Inferred type: str
// Info 4164: (658-660): Inferred type: str
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
