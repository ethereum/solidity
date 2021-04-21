contract C {
    function f() public pure returns (address r) {
        bytes21 x = 0x1122334455667788990011223344556677889900ff;
        bytes20 y;
        assembly {
            y := x
        }
        address z = address(y);
        assembly {
            r := z
        }
        require(z == 0x1122334455667788990011223344556677889900);
    }

    function g() public pure returns (address payable r) {
        bytes21 x = 0x1122334455667788990011223344556677889900ff;
        bytes20 y;
        assembly {
            y := x
        }
        address payable z = payable(address(y));
        assembly {
            r := z
        }
        require(z == 0x1122334455667788990011223344556677889900);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0x1122334455667788990011223344556677889900
// g() -> 0x1122334455667788990011223344556677889900
