contract C {
    function shl_zero(uint a) public returns(uint c) {
        assembly {
            c: = shl(0, a)
        }
    }

    function shr_zero(uint a) public returns(uint c) {
        assembly {
            c: = shr(0, a)
        }
    }

    function sar_zero(uint a) public returns(uint c) {
        assembly {
            c: = sar(0, a)
        }
    }

    function shl_large(uint a) public returns(uint c) {
        assembly {
            c: = shl(0x110, a)
        }
    }

    function shr_large(uint a) public returns(uint c) {
        assembly {
            c: = shr(0x110, a)
        }
    }

    function sar_large(uint a) public returns(uint c) {
        assembly {
            c: = sar(0x110, a)
        }
    }

    function shl_combined(uint a) public returns(uint c) {
        assembly {
            c: = shl(4, shl(12, a))
        }
    }

    function shr_combined(uint a) public returns(uint c) {
        assembly {
            c: = shr(4, shr(12, a))
        }
    }

    function sar_combined(uint a) public returns(uint c) {
        assembly {
            c: = sar(4, sar(12, a))
        }
    }

    function shl_combined_large(uint a) public returns(uint c) {
        assembly {
            c: = shl(0xd0, shl(0x40, a))
        }
    }

    function shl_combined_overflow(uint a) public returns(uint c) {
        assembly {
            c: = shl(0x01, shl(not(0x00), a))
        }
    }

    function shr_combined_large(uint a) public returns(uint c) {
        assembly {
            c: = shr(0xd0, shr(0x40, a))
        }
    }

    function shr_combined_overflow(uint a) public returns(uint c) {
        assembly {
            c: = shr(0x01, shr(not(0x00), a))
        }
    }

    function sar_combined_large(uint a) public returns(uint c) {
        assembly {
            c: = sar(0xd0, sar(0x40, a))
        }
    }
}

// ====
// compileViaYul: also
// ----
shl_zero(uint256): "0"
shl_zero(uint256): "65535"
shl_zero(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shr_zero(uint256): "0"
shr_zero(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
sar_zero(uint256): "0"
sar_zero(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shl_large(uint256): "0"
shl_large(uint256): "65535"
shl_large(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shr_large(uint256): "0"
shr_large(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
sar_large(uint256): "0"
sar_large(uint256): "57896044618658097711785492504343953926634992332820282019728792003956564819967"
sar_large(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shl_combined(uint256): "0"
shl_combined(uint256): "65535"
shl_combined(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shr_combined(uint256): "0"
shr_combined(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
sar_combined(uint256): "0"
sar_combined(uint256): "57896044618658097711785492504343953926634992332820282019728792003956564819967"
sar_combined(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shl_combined_large(uint256): "0"
shl_combined_large(uint256): "65535"
shl_combined_large(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shl_combined_overflow(uint256): "2"
shr_combined_large(uint256): "0"
shr_combined_large(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
shr_combined_overflow(uint256): "2"
sar_combined_large(uint256): "0"
sar_combined_large(uint256): "57896044618658097711785492504343953926634992332820282019728792003956564819967"
sar_combined_large(uint256): "115792089237316195423570985008687907853269984665640564039457584007913129639935"
