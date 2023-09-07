event E1();
event E2(uint);
event E3(uint, string indexed, bytes, bool);
event E4(int, int, int) anonymous;

function f() {
    emit E1();
    emit E2(1);
    emit E3(1, "abc", "abc", true);
    emit E4(1, 2, 3);
}
