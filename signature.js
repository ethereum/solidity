var chai = require('chai');
var assert = chai.assert;
var utils = require('../lib/utils/utils');
var FakeHttpProvider = require('./helpers/FakeHttpProvider');
var signature = require('../lib/web3/signature');
var web3 = require('../index');

var tests = [{
    method: 'functionSignatureFromAscii',
    call: 'web3_sha3',
    request: 'multiply',
    formattedRequest: utils.fromAscii('multiply'),
    result: '0x255d31552d29a21e93334e96055c6dca7cd329f5420ae74ec166d0c47f9f9843',
    formattedResult: '0x255d3155'
},{
    method: 'eventSignatureFromAscii',
    call: 'web3_sha3',
    request: 'multiply',
    formattedRequest:  utils.fromAscii('multiply'),
    result: '0x255d31552d29a21e93334e96055c6dca7cd329f5420ae74ec166d0c47f9f9843',
    formattedResult: '0x255d31552d29a21e93334e96055c6dca7cd329f5420ae74ec166d0c47f9f9843'
}];

describe('lib/web3/signature', function () {
    tests.forEach(function (test, index) {
        describe(test.method, function () {
            it('should properly format and return signature of solidity functioni ' + index, function () {

                // given
                var provider = new FakeHttpProvider();
                web3.setProvider(provider);
                provider.injectResult(test.result);
                provider.injectValidation(function (payload) {
                    assert.equal(payload.method, test.call);
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.params[0], test.formattedRequest);
                });

                // when
                var result = signature[test.method].call(null, test.request);

                // then
                assert.equal(result, test.formattedResult);
            });
        });
    });
});

