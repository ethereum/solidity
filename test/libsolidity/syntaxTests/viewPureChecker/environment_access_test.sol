contract C {
    function f() payable public {
        block.coinbase;
        block.timestamp;
        block.difficulty;
        block.number;
        block.gaslimit;
        blockhash(7);
        gasleft();
        msg.value;
        msg.sender;
        tx.origin;
        tx.gasprice;
        this;
        address(1).balance;
    }
}
// ---
