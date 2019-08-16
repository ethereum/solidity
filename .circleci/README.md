## CircleCI integration

### Docker images

The docker images are build locally on the developer machine:

```!sh
cd .circleci/docker/

docker build -t ethereum/solidity-buildpack-deps:ubuntu1904 -f Dockerfile.ubuntu1904 .
docker push ethereum/solidity-buildpack-deps:ubuntu1904
```

which you can find on Dockerhub after the push at:

    https://hub.docker.com/r/ethereum/solidity-buildpack-deps

where the image tag reflects the target OS to build Solidity and run its test on.

### Testing docker images locally

```!sh
cd solidity
# Mounts your local solidity directory in docker container for testing
docker run -v `pwd`:/src/solidity -ti ethereum/solidity-buildpack-deps:ubuntu1904 /bin/bash
cd /src/solidity
<commands_to_test_build_with_new_docker_image>
```