pragma abicoder v2;

type MyAddress is address;
contract C {
    function id(MyAddress a) external returns (MyAddress b) {
        b = a;
    }

    function unwrap_assembly(MyAddress a) external returns (address b) {
        assembly { b := a }
    }

    function wrap_assembly(address a) external returns (MyAddress b) {
        assembly { b := a }
    }

    function unwrap(MyAddress a) external returns (address b) {
        b = MyAddress.unwrap(a);
    }
    function wrap(address a) external returns (MyAddress b) {
        b = MyAddress.wrap(a);
    }

}
// ----
// id(address): 5 -> 5
// id(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// id(address): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
// unwrap(address): 5 -> 5
// unwrap(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// unwrap(address): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
// wrap(address): 5 -> 5
// wrap(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// wrap(address): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
// unwrap_assembly(address): 5 -> 5
// unwrap_assembly(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// unwrap_assembly(address): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
// wrap_assembly(address): 5 -> 5
// wrap_assembly(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// wrap_assembly(address): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
