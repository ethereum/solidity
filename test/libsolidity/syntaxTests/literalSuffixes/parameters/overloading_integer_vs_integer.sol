function uSuffix(uint8) pure suffix returns (uint) {}
function uSuffix(uint16) pure suffix returns (int) {}

function iSuffix(int8) pure suffix returns (uint) {}
function iSuffix(int16) pure suffix returns (int) {}

function iuSuffix(uint8) pure suffix returns (int) {}
function iuSuffix(int8) pure suffix returns (uint) {}

contract C {
    int a = 1024 uSuffix;
    int b = 1024 iSuffix;

    int c = -1024 uSuffix;
    int d = -1024 iSuffix;

    int e = 255 iuSuffix;
    int f = -255 iuSuffix;
}
