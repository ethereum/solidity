var chai = require('chai');
var assert = chai.assert;
var web3 = require('../index');
var FakeHttpProvider = require('./helpers/FakeHttpProvider');
var bn = require('bignumber.js');

describe('lib/web3/batch', function () {
    describe('execute', function () {
        it('should execute batch request', function (done) {
            
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();

            var result = '0x126';
            var result2 = '0x127';
            provider.injectBatchResults([result, result2]);

            var counter = 0;
            var callback = function (err, r) {
                counter++;
                assert.deepEqual(new bn(result), r);
            };

            var callback2 = function (err, r) {
                assert.equal(counter, 1);
                assert.deepEqual(new bn(result2), r);
                done();
            };

            provider.injectValidation(function (payload) {
                var first = payload[0];
                var second = payload[1];

                assert.equal(first.method, 'eth_getBalance');
                assert.deepEqual(first.params, ['0x0000000000000000000000000000000000000000', 'latest']);
                assert.equal(second.method, 'eth_getBalance');
                assert.deepEqual(second.params, ['0x0000000000000000000000000000000000000005', 'latest']);
            });

            var batch = web3.createBatch(); 
            batch.add(web3.eth.getBalance.request('0x0000000000000000000000000000000000000000', 'latest', callback));
            batch.add(web3.eth.getBalance.request('0x0000000000000000000000000000000000000005', 'latest', callback2));
            batch.execute();
        });
        
        it('should execute batch request', function (done) {
            
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();

            var abi = [{
                "name": "balance(address)",
                "type": "function",
                "inputs": [{
                    "name": "who",
                    "type": "address"
                }],
                "constant": true,
                "outputs": [{
                    "name": "value",
                    "type": "uint256"
                }]
            }];

            
            var address = '0x1000000000000000000000000000000000000001';
            var result = '0x126';
            var result2 = '0x0000000000000000000000000000000000000000000000000000000000000123';

            var counter = 0;
            var callback = function (err, r) {
                counter++;
                assert.deepEqual(new bn(result), r);
            };

            var callback2 = function (err, r) {
                assert.equal(counter, 1);
                assert.deepEqual(new bn(result2), r);
                done();
            };

            provider.injectValidation(function (payload) {
                var first = payload[0];
                var second = payload[1];

                assert.equal(first.method, 'eth_getBalance');
                assert.deepEqual(first.params, ['0x0000000000000000000000000000000000000000', 'latest']);
                assert.equal(second.method, 'eth_call');
                assert.deepEqual(second.params, [{
                    'to': '0x1000000000000000000000000000000000000001',
                    'data': '0xe3d670d70000000000000000000000001000000000000000000000000000000000000001'
                }]);
            });

            var batch = web3.createBatch(); 
            batch.add(web3.eth.getBalance.request('0x0000000000000000000000000000000000000000', 'latest', callback));
            batch.add(web3.eth.contract(abi).at(address).balance.request(address, callback2));
            provider.injectBatchResults([result, result2]);
            batch.execute();
        });
    });
});

