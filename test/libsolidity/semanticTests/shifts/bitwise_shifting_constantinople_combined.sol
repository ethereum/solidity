contract C {
    function shl_zero(uint256 a) public returns (uint256 c) {
        assembly {
            c := shl(0, a)
        }
    }

    function shr_zero(uint256 a) public returns (uint256 c) {
        assembly {
            c := shr(0, a)
        }
    }

    function sar_zero(uint256 a) public returns (uint256 c) {
        assembly {
            c := sar(0, a)
        }
    }

    function shl_large(uint256 a) public returns (uint256 c) {
        assembly {
            c := shl(0x110, a)
        }
    }

    function shr_large(uint256 a) public returns (uint256 c) {
        assembly {
            c := shr(0x110, a)
        }
    }

    function sar_large(uint256 a) public returns (uint256 c) {
        assembly {
            c := sar(0x110, a)
        }
    }

    function shl_combined(uint256 a) public returns (uint256 c) {
        assembly {
            c := shl(4, shl(12, a))
        }
    }

    function shr_combined(uint256 a) public returns (uint256 c) {
        assembly {
            c := shr(4, shr(12, a))
        }
    }

    function sar_combined(uint256 a) public returns (uint256 c) {
        assembly {
            c := sar(4, sar(12, a))
        }
    }

    function shl_combined_large(uint256 a) public returns (uint256 c) {
        assembly {
            c := shl(0xd0, shl(0x40, a))
        }
    }

    function shl_combined_overflow(uint256 a) public returns (uint256 c) {
        assembly {
            c := shl(0x01, shl(not(0x00), a))
        }
    }

    function shr_combined_large(uint256 a) public returns (uint256 c) {
        assembly {
            c := shr(0xd0, shr(0x40, a))
        }
    }

    function shr_combined_overflow(uint256 a) public returns (uint256 c) {
        assembly {
            c := shr(0x01, shr(not(0x00), a))
        }
    }

    function sar_combined_large(uint256 a) public returns (uint256 c) {
        assembly {
            c := sar(0xd0, sar(0x40, a))
        }
    }
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: also
// ----
// shl_zero(uint256): 0x00 -> 0x00
// shl_zero(uint256): 0xffff -> 0xffff
// shl_zero(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// shr_zero(uint256): 0x00 -> 0x00
// shr_zero(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// sar_zero(uint256): 0x00 -> 0x00
// sar_zero(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// shl_large(uint256): 0x00 -> 0x00
// shl_large(uint256): 0xffff -> 0x00
// shl_large(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// shr_large(uint256): 0x00 -> 0x00
// shr_large(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// sar_large(uint256): 0x00 -> 0x00
// sar_large(uint256): 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// sar_large(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// shl_combined(uint256): 0x00 -> 0x00
// shl_combined(uint256): 0xffff -> 0xffff0000
// shl_combined(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0000
// shr_combined(uint256): 0x00 -> 0x00
// shr_combined(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// sar_combined(uint256): 0x00 -> 0x00
// sar_combined(uint256): 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// sar_combined(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// shl_combined_large(uint256): 0x00 -> 0x00
// shl_combined_large(uint256): 0xffff -> 0x00
// shl_combined_large(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// shl_combined_overflow(uint256): 0x02 -> 0x00
// shr_combined_large(uint256): 0x00 -> 0x00
// shr_combined_large(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// shr_combined_overflow(uint256): 0x02 -> 0x00
// sar_combined_large(uint256): 0x00 -> 0x00
// sar_combined_large(uint256): 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// sar_combined_large(uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
