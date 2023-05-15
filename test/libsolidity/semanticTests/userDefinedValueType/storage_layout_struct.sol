type MyInt64 is int64;
struct HalfSlot {
    MyInt64 a;
    MyInt64 b;
}

struct RegularHalfSlot {
    int64 a;
    int64 b;
}

type MyAddress is address;
type MyInt96 is int96;
struct FullSlot {
    MyInt96 a;
    MyAddress b;
}
struct RegularFullSlot {
    int96 a;
    address b;
}

contract C {
    HalfSlot public a;
    RegularHalfSlot public ra;

    HalfSlot public b;
    RegularHalfSlot public rb;

    HalfSlot public c;
    RegularHalfSlot public rc;

    FullSlot public d;
    RegularFullSlot public rd;

    function storage_a() pure external returns(uint slot, uint offset) {
        assembly {
            slot := a.slot
            offset := a.offset
        }
    }

    function storage_ra() pure external returns(uint slot, uint offset) {
        assembly {
            slot := ra.slot
            offset := ra.offset
        }
    }

    function storage_b() pure external returns(uint slot, uint offset) {
        assembly {
            slot := b.slot
            offset := b.offset
        }
    }

    function storage_rb() pure external returns(uint slot, uint offset) {
        assembly {
            slot := rb.slot
            offset := rb.offset
        }
    }

   function storage_c() pure external returns(uint slot, uint offset) {
        assembly {
            slot := c.slot
            offset := c.offset
        }
    }

   function storage_rc() pure external returns(uint slot, uint offset) {
        assembly {
            slot := rc.slot
            offset := rc.offset
        }
    }

   function storage_d() pure external returns(uint slot, uint offset) {
        assembly {
            slot := d.slot
            offset := d.offset
        }
    }

   function storage_rd() pure external returns(uint slot, uint offset) {
        assembly {
            slot := rd.slot
            offset := rd.offset
        }
    }


   function set_a(MyInt64 _a, MyInt64 _b) external {
       a.a = _a;
       a.b = _b;
   }

   function set_ra(int64 _a, int64 _b) external {
       ra.a = _a;
       ra.b = _b;
   }

   function set_b(MyInt64 _a, MyInt64 _b) external {
       b.a = _a;
       b.b = _b;
   }

   function set_rb(int64 _a, int64 _b) external {
       rb.a = _a;
       rb.b = _b;
   }

   function set_c(MyInt64 _a, MyInt64 _b) external {
       c.a = _a;
       c.b = _b;
   }

   function set_rc(int64 _a, int64 _b) external {
       rc.a = _a;
       rc.b = _b;
   }

   function set_d(MyInt96 _a, MyAddress _b) external {
       d.a = _a;
       d.b = _b;
   }

   function set_rd(int96 _a, address _b) external {
       rd.a = _a;
       rd.b = _b;
   }

   function read_slot(uint slot) view external returns (uint value) {
       assembly {
           value := sload(slot)
       }
   }

   function read_contents_asm() external returns (bytes32 rxa, bytes32 rya, bytes32 rxb, bytes32 ryb) {
       b.a = MyInt64.wrap(-2);
       b.b = MyInt64.wrap(-3);
       HalfSlot memory x = b;
       MyInt64 y = b.a;
       MyInt64 z = b.b;
       assembly {
           rxa := mload(x)
           rya := y
           rxb := mload(add(x, 0x20))
           ryb := z
       }
   }
}
// ----
// storage_a() -> 0, 0
// set_a(int64,int64): 100, 200 ->
// read_slot(uint256): 0 -> 0xc80000000000000064
// storage_ra() -> 1, 0
// set_ra(int64,int64): 100, 200 ->
// read_slot(uint256): 1 -> 0xc80000000000000064
// storage_b() -> 2, 0
// set_b(int64,int64): 0, 200 ->
// read_slot(uint256): 2 -> 3689348814741910323200
// storage_rb() -> 3, 0
// set_rb(int64,int64): 0, 200 ->
// read_slot(uint256): 3 -> 3689348814741910323200
// storage_c() -> 4, 0
// set_c(int64,int64): 100, 0 ->
// read_slot(uint256): 4 -> 0x64
// storage_rc() -> 5, 0
// set_rc(int64,int64): 100, 0 ->
// read_slot(uint256): 5 -> 0x64
// storage_d() -> 6, 0
// set_d(int96,address): 39614081257132168796771975167, 1461501637330902918203684832716283019655932542975 ->
// read_slot(uint256): 6 -> -39614081257132168796771975169
// storage_rd() -> 7, 0
// set_rd(int96,address): 39614081257132168796771975167, 1461501637330902918203684832716283019655932542975 ->
// read_slot(uint256): 7 -> -39614081257132168796771975169
// read_contents_asm() -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd
