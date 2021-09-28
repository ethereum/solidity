type MyInt8 is int8;
type MyAddress is address;
type MyInt96 is int96;

contract C {
    MyInt8 a;
    MyInt8 b;
    MyInt8 c;
    MyAddress d;

    MyAddress e;

    MyAddress f;
    MyInt96 g;

    function storage_a() pure external returns(uint slot, uint offset) {
        assembly {
            slot := a.slot
            offset := a.offset
        }
    }

    function storage_b() pure external returns(uint slot, uint offset) {
        assembly {
            slot := b.slot
            offset := b.offset
        }
    }

    function storage_c() pure external returns(uint slot, uint offset) {
        assembly {
            slot := d.slot
            offset := c.offset
        }
    }
    function storage_d() pure external returns(uint slot, uint offset) {
        assembly {
            slot := d.slot
            offset := d.offset
        }
    }

    function storage_e() pure external returns(uint slot, uint offset) {
        assembly {
            slot := e.slot
            offset := e.offset
        }
    }

    function storage_f() pure external returns(uint slot, uint offset) {
        assembly {
            slot := f.slot
            offset := f.offset
        }
    }

    function storage_g() pure external returns(uint slot, uint offset) {
        assembly {
            slot := g.slot
            offset := g.offset
        }
    }

}
// ====
// compileViaYul: also
// ----
// storage_a() -> 0, 0
// storage_b() -> 0, 1
// storage_c() -> 0, 2
// storage_d() -> 0, 3
// storage_e() -> 1, 0
// storage_f() -> 2, 0
// storage_g() -> 2, 0x14
