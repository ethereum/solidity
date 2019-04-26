## CircleCI integration

### Docker images

The docker images are build locally on the developer machine:

```!sh
cd .circleci/docker/

docker build -t ethereum/solc-buildpack-deps:ubuntu1904 -f Dockerfile.ubuntu1904 .
docker push solidity/solc-buildpack-deps:ubuntu1904

docker build -t ethereum/solc-buildpack-deps:archlinux -f Dockerfile.archlinux .
docker push solidity/solc-buildpack-deps:archlinux
```

which you can find on Dockerhub after the push at:

    https://hub.docker.com/r/ethereum/solidity-buildpack-deps

where the image tag reflects the target OS to build Solidity and run its test on.
