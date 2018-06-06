contract C {
    mapping(uint8 => uint8) table1;
    mapping(uint32 => uint8) table2;

    function f() public {
        true ? table1 : table2;
    }
}
// ----
// TypeError: (121-143): True expression's type mapping(uint8 => uint8) doesn't match false expression's type mapping(uint32 => uint8).
