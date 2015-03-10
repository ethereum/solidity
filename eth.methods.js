var assert = require('assert');
var web3 = require('../index.js');
var u = require('./test.utils.js');

describe('web3', function() {
    describe('eth', function() {
        u.methodExists(web3.eth, 'getBalance');
        u.methodExists(web3.eth, 'getStorageAt');
        u.methodExists(web3.eth, 'getStorage');
        u.methodExists(web3.eth, 'getTransactionCount');
        u.methodExists(web3.eth, 'getData');
        u.methodExists(web3.eth, 'sendTransaction');
        u.methodExists(web3.eth, 'call');
        u.methodExists(web3.eth, 'getBlock');
        u.methodExists(web3.eth, 'getTransaction');
        u.methodExists(web3.eth, 'getUncle');
        u.methodExists(web3.eth, 'getCompilers');
        u.methodExists(web3.eth.compile, 'lll');
        u.methodExists(web3.eth.compile, 'solidity');
        u.methodExists(web3.eth.compile, 'serpent');
        u.methodExists(web3.eth, 'getBlockTransactionCount');
        u.methodExists(web3.eth, 'getBlockUncleCount');
        u.methodExists(web3.eth, 'filter');
        u.methodExists(web3.eth, 'contract');

        u.propertyExists(web3.eth, 'coinbase');
        u.propertyExists(web3.eth, 'mining');
        u.propertyExists(web3.eth, 'gasPrice');
        u.propertyExists(web3.eth, 'accounts');
        u.propertyExists(web3.eth, 'defaultBlock');
        u.propertyExists(web3.eth, 'blockNumber');
    });

    // Fail at the moment
    // describe('eth', function(){
    //     it('should be a positive balance', function() {
    //         // when
    //         var testAddress = '0x50f4ed0e83f9da907017bcfb444e3e25407f59bb';
    //         var balance = web3.eth.balanceAt(testAddress);
    //         // then
    //         assert(balance > 0, 'Balance is ' + balance);
    //     });

    //     it('should return a block', function() {
    //         // when
    //         var block = web3.eth.block(0);
            
    //         // then
    //         assert.notEqual(block, null);
    //         assert.equal(block.number, 0);
    //         assert(web3.toDecimal(block.difficulty) > 0);
    //     });
    // });
});





