contract C {
    function f() view external returns (uint ret) {
        assembly {
            ret := shl(gas(), 5)
            ret := shr(ret, 2)
            ret := sar(ret, 2)
        }
    }
    function g() external returns (address ret) {
        assembly {
            ret := create2(0, 0, 0, 0)
        }
    }
    function h() view external returns (bytes32 ret) {
        assembly {
            ret := extcodehash(address())
        }
    }
}
// ====
// EVMVersion: =byzantium
// ----
// TypeError 6612: (103-106): The "shl" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// DeclarationError 8678: (96-116): Variable count does not match number of values (1 vs. 0)
// TypeError 7458: (136-139): The "shr" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// DeclarationError 8678: (129-147): Variable count does not match number of values (1 vs. 0)
// TypeError 2054: (167-170): The "sar" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// DeclarationError 8678: (160-178): Variable count does not match number of values (1 vs. 0)
// TypeError 6166: (283-290): The "create2" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// DeclarationError 8678: (276-302): Variable count does not match number of values (1 vs. 0)
// TypeError 7110: (412-423): The "extcodehash" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// DeclarationError 8678: (405-434): Variable count does not match number of values (1 vs. 0)
