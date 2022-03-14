function id(uint16 x) pure returns(uint16) {
    return x;
}
contract C {
    using {id} for uint8;
}
