bytes32 constant LITERAL = "1234abcd";
bytes32 constant HEX_LITERAL = hex"12345678";
bytes32 constant HEX_NUMBER = 0x0000000000000000000000000000000000000000000000000000000000000042;
uint constant CONST = uint(LITERAL);

contract A layout at uint(LITERAL) {}
contract B layout at uint(HEX_LITERAL) {
    int[uint(HEX_NUMBER)] array;
}
contract C {
    uint[CONST] array;
}
// ----
// Warning 7325: (352-363): Type uint256[22252025335055652125439449056026276909764973351109828411300806246416752050176] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
