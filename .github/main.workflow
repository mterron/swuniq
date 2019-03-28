workflow "Build swuniq" {
  on = "push"
  resolves = [
    "Build with gcc",
    "Build static",
    "Build with clang",
  ]
}

action "Build with gcc" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make gcc musl-dev && cd $GITHUB_WORKSPACE && make"]
}

action "Build with clang" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make gcc clang musl-dev && cd $GITHUB_WORKSPACE && CC=clang make"]
}

action "Build static" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make clang musl-dev && cd $GITHUB_WORKSPACE && CC=clang make static || exit 0"]
}
