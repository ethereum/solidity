library L {
    function transfer(address a) internal {}
    function send(address a) internal {}
}

contract C {
    using L for address;

    function useTransfer(address a) public {
        a.transfer();
    }

    function useSend(address a) public {
        a.send();
    }
}
// ----
// useTransfer(address): 0x111122223333444455556666777788889999aAaa ->
// useSend(address): 0x111122223333444455556666777788889999aAaa ->
