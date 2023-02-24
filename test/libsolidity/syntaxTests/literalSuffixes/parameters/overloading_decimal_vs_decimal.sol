function uSuffix(uint8, uint) pure suffix returns (int8) {}
function uSuffix(uint16, uint) pure suffix returns (int16) {}

function iSuffix(int8, uint) pure suffix returns (int24) {}
function iSuffix(int16, uint) pure suffix returns (int32) {}

function iuSuffix(uint8, uint) pure suffix returns (int40) {}
function iuSuffix(int8, uint) pure suffix returns (int48) {}

contract C {
    int16 a = 1.024 uSuffix;
    int32 b = 1.024 iSuffix;

    int16 c = -1.024 uSuffix;
    int32 d = -1.024 iSuffix;

    int40 e = 2.55 iuSuffix;
    int40 f = -2.55 iuSuffix;
}
