load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

release = "1.10.0"
http_archive(
  name = "googletest",
  urls = ["https://github.com/google/googletest/archive/release-" + release + ".tar.gz"],
  strip_prefix = "googletest-release-" + release,
)

http_file(
  name = "cpplint_build",
  urls = ["https://raw.githubusercontent.com/nicmcd/pkgbuild/master/cpplint.BUILD"],
)

release = "1.5.4"
http_archive(
  name = "cpplint",
  urls = ["https://github.com/cpplint/cpplint/archive/" + release + ".tar.gz"],
  strip_prefix = "cpplint-" + release,
  build_file = "@cpplint_build//file:downloaded",
)

http_file(
  name = "clang_format",
  urls = ["https://raw.githubusercontent.com/nicmcd/pkgbuild/master/clang-format"],
)

http_file(
  name = "zlib_build",
  urls = ["https://raw.githubusercontent.com/nicmcd/pkgbuild/master/zlib.BUILD"],
)

version = "1.2.11"
http_archive(
  name = "zlib",
  urls = ["https://www.zlib.net/zlib-" + version + ".tar.gz"],
  strip_prefix = "zlib-" + version,
  build_file = "@zlib_build//file:downloaded",
)

http_file(
    name = "nlohmann_json_build",
    urls = ["https://raw.githubusercontent.com/nicmcd/pkgbuild/master/nlohmannjson.BUILD"],
)

release = "3.9.1"
http_archive(
    name = "nlohmann_json",
    urls = ["https://github.com/nlohmann/json/archive/v" + release + ".tar.gz"],
    strip_prefix = "json-" + release,
    build_file = "@nlohmann_json_build//file:downloaded",
)

hash = "ef2951d"
http_archive(
  name = "libprim",
  urls = ["https://github.com/nicmcd/libprim/tarball/" + hash],
  type = "tar.gz",
  strip_prefix = "nicmcd-libprim-" + hash,
)

hash = "2fa6f76"
http_archive(
  name = "libstrop",
  urls = ["https://github.com/nicmcd/libstrop/tarball/" + hash],
  type = "tar.gz",
  strip_prefix = "nicmcd-libstrop-" + hash,
)

hash = "5573f57"
http_archive(
  name = "libfio",
  urls = ["https://github.com/nicmcd/libfio/tarball/" + hash],
  type = "tar.gz",
  strip_prefix = "nicmcd-libfio-" + hash,
)
