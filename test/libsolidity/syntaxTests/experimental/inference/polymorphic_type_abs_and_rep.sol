pragma experimental solidity;

type uint;
type string;

type T(A);
type U(B) = T(B);

function fun() {
    let w: U(uint);
    let v: T(uint);
    U.rep(w);
    U.abs(v);

    let s: U(string);
    let t: T(string);
    U.rep(s);
    U.abs(t);
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-41): Inferred type: uint
// Info 4164: (42-54): Inferred type: string
// Info 4164: (56-66): Inferred type: tfun(?v:type, T(?v:type))
// Info 4164: (62-65): Inferred type: ?u:type
// Info 4164: (63-64): Inferred type: ?u:type
// Info 4164: (67-84): Inferred type: tfun(?x:type, U(?x:type))
// Info 4164: (73-76): Inferred type: ?w:type
// Info 4164: (74-75): Inferred type: ?w:type
// Info 4164: (79-83): Inferred type: T(?w:type)
// Info 4164: (79-80): Inferred type: tfun(?w:type, T(?w:type))
// Info 4164: (81-82): Inferred type: ?w:type
// Info 4164: (86-245): Inferred type: () -> ()
// Info 4164: (98-100): Inferred type: ()
// Info 4164: (111-121): Inferred type: U(uint)
// Info 4164: (114-121): Inferred type: U(uint)
// Info 4164: (114-115): Inferred type: tfun(uint, U(uint))
// Info 4164: (116-120): Inferred type: uint
// Info 4164: (131-141): Inferred type: T(uint)
// Info 4164: (134-141): Inferred type: T(uint)
// Info 4164: (134-135): Inferred type: tfun(uint, T(uint))
// Info 4164: (136-140): Inferred type: uint
// Info 4164: (147-155): Inferred type: T(?bi:type)
// Info 4164: (147-152): Inferred type: U(uint) -> T(?bi:type)
// Info 4164: (147-148): Inferred type: U(?bg:type)
// Info 4164: (153-154): Inferred type: U(uint)
// Info 4164: (161-169): Inferred type: U(?bm:type)
// Info 4164: (161-166): Inferred type: T(uint) -> U(?bm:type)
// Info 4164: (161-162): Inferred type: U(?bk:type)
// Info 4164: (167-168): Inferred type: T(uint)
// Info 4164: (180-192): Inferred type: U(string)
// Info 4164: (183-192): Inferred type: U(string)
// Info 4164: (183-184): Inferred type: tfun(string, U(string))
// Info 4164: (185-191): Inferred type: string
// Info 4164: (202-214): Inferred type: T(string)
// Info 4164: (205-214): Inferred type: T(string)
// Info 4164: (205-206): Inferred type: tfun(string, T(string))
// Info 4164: (207-213): Inferred type: string
// Info 4164: (220-228): Inferred type: T(?bu:type)
// Info 4164: (220-225): Inferred type: U(string) -> T(?bu:type)
// Info 4164: (220-221): Inferred type: U(?bs:type)
// Info 4164: (226-227): Inferred type: U(string)
// Info 4164: (234-242): Inferred type: U(?by:type)
// Info 4164: (234-239): Inferred type: T(string) -> U(?by:type)
// Info 4164: (234-235): Inferred type: U(?bw:type)
// Info 4164: (240-241): Inferred type: T(string)
