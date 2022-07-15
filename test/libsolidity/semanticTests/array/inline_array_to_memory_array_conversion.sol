pragma abicoder v2;

contract C {

    struct CustomStruct{
        uint16 index;
        string text;
    }

    function statically_sized_uint_array() public returns (uint8[3] memory) {
        uint8[3] memory a = [1, 2, 3];
        return a;
    }

    function dynamically_sized_uint_array() public returns (uint8[] memory) {
        uint8[] memory a = [1, 2, 3];
        return a;
    }

    function statically_sized_int_array() public returns (int8[3] memory) {
        int8[3] memory a = [-1, -2, -3];
        return a;
    }

    function dynamically_sized_int_array() public returns (int8[] memory) {
        int8[] memory a = [-1, -2, -3];
        return a;
    }

    function statically_sized_string_array() public returns (string[2] memory) {
        string[2] memory a = ["foo", "bar"];
        return a;
    }

    function dynamically_sized_string_array() public returns (string[] memory) {
        string[] memory a = ["foo", "bar"];
        return a;
    }

    function statically_sized_struct_array() public returns (CustomStruct[2] memory) {
        CustomStruct[2] memory s = [CustomStruct(1, "foo"), CustomStruct(2, "bar")];
        return s;
    }

    function dynamically_sized_struct_array() public returns (CustomStruct[] memory) {
        CustomStruct[] memory s = [CustomStruct(1, "foo"), CustomStruct(2, "bar")];
        return s;
    }
}

// ====
// EVMVersion: >=constantinople
// compileToEwasm: also
// compileViaYul: also
// ----
// statically_sized_uint_array() -> 1, 2, 3
// dynamically_sized_uint_array() -> 0x20, 3, 1, 2, 3
// statically_sized_int_array() -> -1, -2, -3
// dynamically_sized_int_array() -> 0x20, 3, -1, -2, -3
// statically_sized_string_array() -> 0x20, 0x40, 0x80, 3, "foo", 3, "bar"
// dynamically_sized_string_array() -> 0x20, 2, 0x40, 0x80, 3, 46332796673528066027243215619882264990369332300865266851730502456685210107904, 3, 44498830125527143464827115118378702402016761369235290884359940707316142178304
// statically_sized_struct_array() -> 0x20, 0x40, 0xc0, 1, 0x40, 3, 46332796673528066027243215619882264990369332300865266851730502456685210107904, 2, 0x40, 3, 44498830125527143464827115118378702402016761369235290884359940707316142178304
// dynamically_sized_struct_array() -> 0x20, 2, 0x40, 0xc0, 1, 0x40, 3, 46332796673528066027243215619882264990369332300865266851730502456685210107904, 2, 0x40, 3, 44498830125527143464827115118378702402016761369235290884359940707316142178304
