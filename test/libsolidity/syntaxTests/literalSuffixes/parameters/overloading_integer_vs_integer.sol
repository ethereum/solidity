function uSuffix(uint8) pure suffix returns (int8) {}
function uSuffix(uint16) pure suffix returns (int16) {}

function iSuffix(int8) pure suffix returns (int24) {}
function iSuffix(int16) pure suffix returns (int32) {}

function iuSuffix(uint8) pure suffix returns (int40) {}
function iuSuffix(int8) pure suffix returns (int48) {}

contract C {
    int16 a = 1024 uSuffix;
    int32 b = 1024 iSuffix;

    int16 c = -1024 uSuffix;
    int32 d = -1024 iSuffix;

    int40 e = 255 iuSuffix;
    int40 f = -255 iuSuffix;
}
