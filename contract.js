var chai = require('chai');
var assert = chai.assert;
var web3 = require('../index');
var FakeHttpProvider = require('./helpers/FakeHttpProvider');
var FakeHttpProvider2 = require('./helpers/FakeHttpProvider2');
var utils = require('../lib/utils/utils');
var BigNumber = require('bignumber.js');

var desc = [{
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
}, {
    "name": "send(address,uint256)",
    "type": "function",
    "inputs": [{
        "name": "to",
        "type": "address"
    }, {
        "name": "value",
        "type": "uint256"
    }],
    "outputs": []
}, {
    "name": "testArr(int[])",
    "type": "function",
    "inputs": [{
        "name": "value",
        "type": "int[]"
    }],
    "constant": true,
    "outputs": [{
        "name": "d",
        "type": "int"
    }]
}, {
    "name":"Changed",
    "type":"event",
    "inputs": [
        {"name":"from","type":"address","indexed":true},
        {"name":"amount","type":"uint256","indexed":true},
        {"name":"t1","type":"uint256","indexed":false},
        {"name":"t2","type":"uint256","indexed":false}
    ],
}];

var address = '0x1234567890123456789012345678901234567890';

describe('web3.eth.contract', function () {
    describe('event', function () {
        it('should create event filter', function (done) {
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset(); // reset different polls
            var sha3 = '0x5131231231231231231231';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('Changed(address,uint256,uint256,uint256)'));
                } else if (step === 1) {
                    step = 2;
                    provider.injectResult(3);
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'eth_newFilter');
                    assert.deepEqual(payload.params[0], {
                        topics: [
                            sha3,
                            '0x0000000000000000000000001234567890123456789012345678901234567890',
                            null
                        ],
                        address: '0x1234567890123456789012345678901234567890'
                    });
                } else if (step === 2) {
                    step = 3;
                    provider.injectResult([{
                        address: address,
                        topics: [
                            sha3,
                            '0x0000000000000000000000001234567890123456789012345678901234567890',
                            '0x0000000000000000000000000000000000000000000000000000000000000001'
                        ],
                        number: 2,
                        data: '0x0000000000000000000000000000000000000000000000000000000000000001' +
                                '0000000000000000000000000000000000000000000000000000000000000008' 
                    }]);
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'eth_getFilterLogs');
                } else if (step === 3 && utils.isArray(payload)) {
                    provider.injectBatchResults([[{
                        address: address,
                        topics: [
                            sha3,
                            '0x0000000000000000000000001234567890123456789012345678901234567890',
                            '0x0000000000000000000000000000000000000000000000000000000000000001'
                        ],
                        number: 2,
                        data: '0x0000000000000000000000000000000000000000000000000000000000000001' +
                                '0000000000000000000000000000000000000000000000000000000000000008' 
                    }]]);
                    var r = payload.filter(function (p) {
                        return p.jsonrpc === '2.0' && p.method === 'eth_getFilterChanges' && p.params[0] === 3;
                    });
                    assert.equal(r.length > 0, true);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            var res = 0;
            contract.Changed({from: address}).watch(function(err, result) {
                assert.equal(result.args.from, address); 
                assert.equal(result.args.amount, 1);
                assert.equal(result.args.t1, 1);
                assert.equal(result.args.t2, 8);
                res++;
                if (res === 2) {
                    done();
                }
            });
        });

        it('should call constant function', function () {
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('balance(address)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_call');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + '0000000000000000000000001234567890123456789012345678901234567890',
                        to: address
                    }, 'latest']);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.balance(address);
        });

        it('should sendTransaction to contract function', function () {
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('send(address,uint256)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_sendTransaction');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + 
                            '0000000000000000000000001234567890123456789012345678901234567890' + 
                            '0000000000000000000000000000000000000000000000000000000000000011' ,
                        to: address
                    }]);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.send(address, 17);
        });

        it('should make a call with optional params', function () {
           
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('balance(address)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_call');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + '0000000000000000000000001234567890123456789012345678901234567890',
                        to: address,
                        from: address,
                        gas: '0xc350'
                    }, 'latest']);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.balance(address, {from: address, gas: 50000});

        });

        it('should explicitly make a call with optional params', function () {
           
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('balance(address)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_call');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + '0000000000000000000000001234567890123456789012345678901234567890',
                        to: address,
                        from: address,
                        gas: '0xc350'
                    }, 'latest']);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.balance.call(address, {from: address, gas: 50000});

        });

        it('should sendTransaction with optional params', function () {
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('send(address,uint256)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_sendTransaction');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + 
                            '0000000000000000000000001234567890123456789012345678901234567890' + 
                            '0000000000000000000000000000000000000000000000000000000000000011' ,
                        to: address,
                        from: address,
                        gas: '0xc350',
                        gasPrice: '0xbb8',
                        value: '0x2710'
                    }]);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.send(address, 17, {from: address, gas: 50000, gasPrice: 3000, value: 10000});
        });

        it('should explicitly sendTransaction with optional params', function () {
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('send(address,uint256)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_sendTransaction');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + 
                            '0000000000000000000000001234567890123456789012345678901234567890' + 
                            '0000000000000000000000000000000000000000000000000000000000000011' ,
                        to: address,
                        from: address,
                        gas: '0xc350',
                        gasPrice: '0xbb8',
                        value: '0x2710'
                    }]);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.send.sendTransaction(address, 17, {from: address, gas: 50000, gasPrice: 3000, value: 10000});
        });

        it('should explicitly sendTransaction with optional params and call callback without error', function (done) {
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResult(sha3);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 0) {
                    step = 1;
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, 'web3_sha3');
                    assert.equal(payload.params[0], web3.fromAscii('send(address,uint256)'));
                } else if (step === 1) {
                    assert.equal(payload.method, 'eth_sendTransaction');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + 
                            '0000000000000000000000001234567890123456789012345678901234567890' + 
                            '0000000000000000000000000000000000000000000000000000000000000011' ,
                        to: address,
                        from: address,
                        gas: '0xc350',
                        gasPrice: '0xbb8',
                        value: '0x2710'
                    }]);
                }
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.send.sendTransaction(address, 17, {from: address, gas: 50000, gasPrice: 3000, value: 10000}, function (err) {
                assert.equal(err, null);
                done();
            });
        });

        it('should call testArr method and properly parse result', function () {
            var provider = new FakeHttpProvider2();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResultList([{
                result: sha3
            }, {
                result: '0x0000000000000000000000000000000000000000000000000000000000000005'
            }]);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 1) { // getting sha3 is first
                    assert.equal(payload.method, 'eth_call');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + 
                            '0000000000000000000000000000000000000000000000000000000000000020' + 
                            '0000000000000000000000000000000000000000000000000000000000000001' + 
                            '0000000000000000000000000000000000000000000000000000000000000003',
                        to: address
                    },
                        'latest'
                    ]);
                }
                step++;
            });

            var contract = web3.eth.contract(desc).at(address);
            var result = contract.testArr([3]);

            assert.deepEqual(new BigNumber(5), result);
        });
        
        it('should call testArr method, properly parse result and return the result async', function (done) {
            var provider = new FakeHttpProvider2();
            web3.setProvider(provider);
            web3.reset();
            var sha3 = '0x5131231231231231231231';
            var address = '0x1234567890123456789012345678901234567890';
            provider.injectResultList([{
                result: sha3
            }, {
                result: '0x0000000000000000000000000000000000000000000000000000000000000005'
            }]);
            var step = 0;
            provider.injectValidation(function (payload) {
                if (step === 1) { // getting sha3 is first
                    assert.equal(payload.method, 'eth_call');
                    assert.deepEqual(payload.params, [{
                        data: sha3.slice(0, 10) + 
                            '0000000000000000000000000000000000000000000000000000000000000020' + 
                            '0000000000000000000000000000000000000000000000000000000000000001' + 
                            '0000000000000000000000000000000000000000000000000000000000000003',
                        to: address
                    },
                        'latest'
                    ]);
                }
                step++;
            });

            var contract = web3.eth.contract(desc).at(address);

            contract.testArr([3], function (err, result) {
                assert.deepEqual(new BigNumber(5), result);
                done();
            });

        });
    });
});

