# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

**Resolve IR files via Pd's search path**

`vas_pdmaxobject_read` concatenated the canvas directory with the filename,
so an IR was only ever found next to the patch loading it. Use
`open_via_path`: absolute path, then the patch directory, then the global
search path a host can extend with `libpd_add_to_search_path`.

Also bounds the path (fullpath is 512 bytes, `MAXPDSTRING` is 1000) and
reports a missing file instead of passing a bad path down.
