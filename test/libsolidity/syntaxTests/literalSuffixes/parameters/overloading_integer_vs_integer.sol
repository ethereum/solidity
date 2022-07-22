function uSuffix(uint8) pure returns (int8) {}
function uSuffix(uint16) pure returns (int16) {}

function iSuffix(int8) pure returns (int24) {}
function iSuffix(int16) pure returns (int32) {}

function iuSuffix(uint8) pure returns (int40) {}
function iuSuffix(int8) pure returns (int48) {}

contract C {
    int16 a = 1024 uSuffix;
    int32 b = 1024 iSuffix;

    int16 c = -1024 uSuffix;
    int32 d = -1024 iSuffix;

    int40 e = 255 iuSuffix;
    int40 f = -255 iuSuffix;
}
