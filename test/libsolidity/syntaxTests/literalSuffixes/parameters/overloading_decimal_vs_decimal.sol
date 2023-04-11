function uSuffix(uint8, uint) pure suffix returns (uint) {}
function uSuffix(uint16, uint) pure suffix returns (int) {}

function iSuffix(int8, uint) pure suffix returns (uint) {}
function iSuffix(int16, uint) pure suffix returns (int) {}

function iuSuffix(uint8, uint) pure suffix returns (int) {}
function iuSuffix(int8, uint) pure suffix returns (uint) {}

contract C {
    int a = 1.024 uSuffix;
    int b = 1.024 iSuffix;

    int c = -1.024 uSuffix;
    int d = -1.024 iSuffix;

    int e = 2.55 iuSuffix;
    int f = -2.55 iuSuffix;
}
