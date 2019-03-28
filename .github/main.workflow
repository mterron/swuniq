workflow "Build swuniq" {
  on = "push"
  resolves = [
    "Bulid with clang",
    "Build with gcc",
    "Build static",
  ]
}

action "Build with gcc" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make gcc binutils && cd $GITHUB_WORKSPACE && make"]
}

action "Bulid with clang" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make clang binutils && cd $GITHUB_WORKSPACE && CC=clang make"]
}

action "Build static" {
  uses = "docker://alpine:latest"
  runs = ["sh", "-c", "apk add make clang musl-dev binutils && cd $GITHUB_WORKSPACE && CC=clang make static"]
}
